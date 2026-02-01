#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform vec2 offsets[9];
uniform int edge_kernel[9];
uniform float blur_kernel[9];

uniform bool chaos;
uniform bool confuse;
uniform bool shake;

void main() {
  vec3 color = vec3(0.0);
  vec3 samples[9];

  // Sample texture at kernel offsets
  if (chaos || shake) {
    for (int i = 0; i < 9; i++) {
      samples[i] = vec3(texture(scene, TexCoords + offsets[i]));
    }
  }

  // Apply effects
  if (chaos) {
    // Edge detection kernel
    for (int i = 0; i < 9; i++) {
      color += samples[i] * edge_kernel[i];
    }
    color = abs(color);
  } else if (confuse) {
    // Invert colors
    color = vec3(1.0 - texture(scene, TexCoords));
  } else if (shake) {
    // Blur kernel
    for (int i = 0; i < 9; i++) {
      color += samples[i] * blur_kernel[i];
    }
  } else {
    // No effect
    color = vec3(texture(scene, TexCoords));
  }

  FragColor = vec4(color, 1.0);
}
