#version 420

in vec3 worldPos;
in vec3 worldNorm;
in vec2 uvCoord;

in vec3 worldTangent;
in vec3 worldBiTangent;

in mat3 TBN;

uniform vec3 cameraPosition;

const float PI = 3.14159265359;

uniform vec3 albedoConstant;
uniform float roughnessConstant;
uniform float metallicConstant;

uniform int useAlbedoTexture;
uniform int useRoughnessTexture;
uniform int useMetallicTexture;
uniform int useNormalTexture;

uniform sampler2D albedoTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D metallicTexture;
uniform sampler2D normalTexture;

/*
 * Taken from https://learnopengl.com/PBR/Lighting
 */

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3  albedo;
    float roughness;
    float metallic;
    float ao = 1;

    if(useAlbedoTexture != 0) {
        albedo = texture(albedoTexture, uvCoord).rgb;
    } else {
        albedo = albedoConstant;
    }

    if(useRoughnessTexture != 0) {
        roughness = texture(roughnessTexture, uvCoord).r;
    } else {
        roughness = roughnessConstant;
    }

    if(useMetallicTexture != 0) {
        metallic = texture(metallicTexture, uvCoord).r;
    } else {
        metallic = metallicConstant;
    }

    vec3 N;
    vec3 V;

    vec3 lightDir;

    if(useNormalTexture != 0) {
        N = normalize(TBN * (texture(normalTexture, uvCoord).rgb*2.0 - 1.0));
        V = normalize((cameraPosition - worldPos));
        lightDir = normalize( vec3(-1, -1, -1));
    } else {
        N = normalize(worldNorm);
        V = normalize(cameraPosition - worldPos);
        lightDir = normalize(vec3(-1, -1, -1));
    }

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    vec3 lightPosition = vec3(4, 5, 4);
    vec3 lightColor = vec3(1, 1, 1) * 1;

    // calculate per-light radiance
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);
    //float distance    = length(lightPosition - worldPos);
    //float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = lightColor;// * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = clamp(ambient + Lo, 0, 1);

    //color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    gl_FragColor = vec4(color, 1.0);
}