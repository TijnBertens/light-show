#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cmath>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Camera {
private:
    constexpr static const float MIN_PITCH = -.49f * M_PI;
    constexpr static const float MAX_PITCH = +.49f * M_PI;
    constexpr static const float MAX_ZOOM = 32.f;
    constexpr static const float MIN_ZOOM = .1f;

    /** Aspect ratio of the viewport. */
    float aspect_ratio;

    /** Vertical field of view in radians. */
    float fov;

    float near_clipping_dist;
    float far_clipping_dist;

    /** Target location around which this camera orbits. */
    glm::vec3 target;

    /** Yaw, pitch and roll respectively represented in a vector (in radians). */
    glm::vec3 angles;

    /** Used to calculate distance from the camera to the target. (Further away is higher). */
    float zoom_level;
public:
    Camera(float p_aspect_ratio, float p_fov, float p_near_clipping_dist, float p_far_clipping_dist);

    /** Add a given value to the zoom level. */
    void add_zoom(float zoom_increment);

    /** Constructs and returns the view matrix for this camera. */
    glm::mat4 get_view_matrix();

    /** Constructs and returns the projection matrix for this camera. */
    glm::mat4 get_proj_matrix() const;

    /** Reverse-engineers the camera position from the target, angles and zoom level. */
    glm::vec3 get_camera_position();

    /** Sets the aspect ratio to a specific value. */
    void set_aspect(float p_aspect_ratio);

    /** Translate using mouse offsets. */
    void translate(double offset_xpos, double offset_ypos);

    /** Rotate using mouse offsets. */
    void rotate(double offset_xpos, double offset_ypos);
};


#endif //CAMERA_HPP
