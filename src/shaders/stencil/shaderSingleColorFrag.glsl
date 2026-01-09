#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform bool useTex;
uniform sampler2D texture_diffuse1;

void main()
{
  if (useTex) {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if (texColor.a < 0.1) discard;
  }

  FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}
