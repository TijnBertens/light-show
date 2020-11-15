#version 420

uniform mat4 ModelM;
uniform mat4 ViewM;
uniform mat4 ProjectionM;

uniform vec3 cameraPosition;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;

layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 biTangent;

out vec3 worldPos;
out vec3 worldNorm;
out vec2 uvCoord;

out vec3 worldTangent;
out vec3 worldBiTangent;

out mat3 TBN;

void main()
{
    gl_Position = ProjectionM * ViewM * ModelM * vec4(vPos, 1.0);
    uvCoord = vTex;

    //TODO: it is very slow to compute this as a 4x4 rather than 3x3 and to do it in a vertex shader
    mat4 normalMatrix = transpose(inverse(ModelM));

    worldPos = (ModelM * vec4(vPos, 1.0)).xyz;
    worldNorm = normalize((normalMatrix * vec4(vNorm, 0.0)).xyz);
    worldTangent = normalize((normalMatrix * vec4(tangent, 0.0)).xyz);
    worldBiTangent = normalize((normalMatrix * vec4(biTangent, 0.0)).xyz);

    TBN = mat3(
        worldTangent,
        worldBiTangent,
        worldNorm
    );
}