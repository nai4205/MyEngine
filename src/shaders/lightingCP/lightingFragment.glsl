#version 330 core
struct Material {
  bool useTex;
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shininess;
  vec3 vAmbient;
  vec3 vDiffuse;
  vec3 vSpecular;
};

struct Light {
  vec3 position;
  vec3 direction;

  float cutOff;
  float outerCutOff;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
  vec3 lightDir = normalize(light.position - FragPos);
  // vec3 lightDir = normalize(-light.direction);
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  if (theta > light.cutOff) {
    // Normalize vectors
    vec3 norm = normalize(Normal);

    // Attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient lighting
    vec3 ambient;
    if (material.useTex) {
      ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    } else {
      ambient = material.vAmbient;
    }

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse;
    if (material.useTex) {
      diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    } else {
      diffuse = material.vDiffuse * diff;
    }

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular;
    if (material.useTex) {
      specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
    } else {
      specular = material.vSpecular * spec;
    }

    // // Emission lighting
    // vec3 emission = texture(material.emission, TexCoords).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    diffuse *= intensity;
    specular *= intensity;

    // Combine results
    // vec3 result = ambient + diffuse + specular + emission;
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
  } else {
    if (material.useTex) {
      FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);
    } else {
      FragColor = vec4(light.ambient * material.vDiffuse, 1.0);
    }
  }
}
