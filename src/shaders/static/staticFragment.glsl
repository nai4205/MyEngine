#version 330 core

const float PI = 3.14159265;

struct Material {
  bool useTex;
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D emission;
  float shininess;
  vec3 vAmbient;
  vec3 vDiffuse;
  vec3 vSpecular;
};

struct SpotLight {
  bool isActive;
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;

  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct DirLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
  vec3 position;

  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
#define NR_POINT_LIGHTS 10

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
  // properties
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // directional lighting
  vec3 result = CalcDirLight(dirLight, norm, viewDir);

  // point lights
  for (int i = 0; i < NR_POINT_LIGHTS; i++)
  {
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  // spot light
  if (spotLight.isActive)
  {
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
  }

  if (material.useTex) {
    vec4 texColor = texture(material.texture_diffuse1, TexCoords);
    if (texColor.a < 0.1) discard;
    FragColor = vec4(result * texColor.rgb, texColor.a);
  } else {
    FragColor = vec4(result, 1.0);
  }
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading (Blinn-Phong with energy conservation)
  float kEnergyConservation = (8.0 + material.shininess) / (8.0 * PI);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = kEnergyConservation * pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  // combine results
  vec3 ambient;
  if (material.useTex) {
    ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    ambient = light.ambient * material.vAmbient;
  }

  vec3 diffuse;
  if (material.useTex) {
    diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    diffuse = light.diffuse * diff * material.vDiffuse;
  }
  vec3 specular;
  if (material.useTex) {
    specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb;
  } else {
    specular = light.specular * spec * material.vSpecular;
  }
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading (Blinn-Phong with energy conservation)
  float kEnergyConservation = (8.0 + material.shininess) / (8.0 * PI);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = kEnergyConservation * pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  // attenuation
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
        light.quadratic * (distance * distance));
  // combine results
  vec3 ambient;
  if (material.useTex) {
    ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    ambient = light.ambient * material.vAmbient;
  }
  vec3 diffuse;
  if (material.useTex) {
    diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    diffuse = light.diffuse * diff * material.vDiffuse;
  }
  vec3 specular;
  if (material.useTex) {
    specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb;
  } else {
    specular = light.specular * spec * material.vSpecular;
  }
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading (Blinn-Phong with energy conservation)
  float kEnergyConservation = (8.0 + material.shininess) / (8.0 * PI);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = kEnergyConservation * pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  // attenuation
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  // spotlight intensity
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  // combine results
  vec3 ambient;
  if (material.useTex) {
    ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    ambient = light.ambient * material.vAmbient;
  }
  vec3 diffuse;
  if (material.useTex) {
    diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
  } else {
    diffuse = light.diffuse * diff * material.vDiffuse;
  }
  vec3 specular;
  if (material.useTex) {
    specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb;
  } else {
    specular = light.specular * spec * material.vSpecular;
  }
  ambient *= attenuation * intensity;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;
  return (ambient + diffuse + specular);
}
