#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float spriteAlpha;

void main()
{
  color = vec4(spriteColor, spriteAlpha) * texture(image, TexCoords);
}
