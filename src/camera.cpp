#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(
        float p_aspect_ratio, float p_fov, float p_near_clipping_dist, float p_far_clipping_dist
) :
        aspect_ratio(p_aspect_ratio), fov(p_fov), near_clipping_dist(p_near_clipping_dist),
        far_clipping_dist(p_far_clipping_dist)
{
    target = glm::vec3(0.f);
    angles = glm::vec3(0.f);
    zoom_level = 5.f;
}

void Camera::add_zoom(float zoom_increment)
{
    zoom_level = fmaxf(MIN_ZOOM, fminf(MAX_ZOOM, zoom_level - zoom_increment));
}

glm::mat4 Camera::get_view_matrix()
{
    const glm::vec3 UP = glm::vec3(0.f, 1.f, 0.f);
    return glm::lookAt(get_camera_position(), target, UP);
}

glm::mat4 Camera::get_proj_matrix() const
{
    return glm::perspective(fov, aspect_ratio, near_clipping_dist, far_clipping_dist);
}

glm::vec3 Camera::get_camera_position()
{
    // distance to focus point is computed as (1.2^zoomConstant)
    float dist = powf(1.2f, zoom_level);

    // pitch, roll and yaw rotation to find world position
    // calculate the rotation matrix
    glm::mat4 rotation = glm::eulerAngleYXZ(angles[1], angles[0], angles[2]);

    // apply rotation matrix
    glm::vec4 above = glm::vec4(0.f, 0.f, dist, 0.f);
    glm::vec4 position = rotation * above;

    glm::vec3 position_3 = glm::vec3(position.x, position.y, position.z);

    return position_3 + target;
}

void Camera::set_aspect(float p_aspect_ratio)
{
    aspect_ratio = p_aspect_ratio;
}

void Camera::translate(double offset_xpos, double offset_ypos)
{
    float PANNING_SENSITIVITY = .005f * zoom_level;

    // new offset based on sensitivity
    glm::vec4 offset = glm::vec4(
            (float) -offset_xpos * PANNING_SENSITIVITY, 0.f,
            (float) -offset_ypos * PANNING_SENSITIVITY, 0.f
    );

    // rotate the offset with the camera yaw
    glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), angles[1], glm::vec3(0.f, 1.f, 0.f));
    glm::vec4 rotated_offset = rotation * offset;

    // add offset to current lookat
    glm::vec3 rotated_offset_3 = glm::vec3(
            rotated_offset.x / rotated_offset.w,
            rotated_offset.y / rotated_offset.w,
            rotated_offset.z / rotated_offset.w);

    target += rotated_offset_3;
}

void Camera::rotate(double offset_xpos, double offset_ypos)
{
    const float ROTATION_SENSITIVITY = 0.03f;

    angles[0] += (float) -offset_ypos * ROTATION_SENSITIVITY;  // pitch
    angles[0] = fmaxf(MIN_PITCH, fminf(MAX_PITCH, angles[0])); // enforce max and min pitch
    angles[1] += (float) -offset_xpos * ROTATION_SENSITIVITY;  // yaw
}
