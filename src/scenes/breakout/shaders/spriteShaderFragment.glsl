#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float spriteAlpha;
uniform bool useTexture;

void main()
{
  if (useTexture) {
    color = vec4(spriteColor, spriteAlpha) * texture(image, TexCoords);
  } else {
    color = vec4(spriteColor, spriteAlpha);
  }
}
