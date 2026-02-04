#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float spriteAlpha;
uniform bool useTexture;
uniform bool isCircle;

void main()
{
  if (isCircle) {
    // Discard fragments outside circle (UV center is 0.5, 0.5)
    vec2 center = TexCoords - vec2(0.5);
    if (dot(center, center) > 0.25) { // 0.25 = 0.5^2
      discard;
    }
  }

  if (useTexture) {
    color = vec4(spriteColor, spriteAlpha) * texture(image, TexCoords);
  } else {
    color = vec4(spriteColor, spriteAlpha);
  }
}
