#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effect; // 0 = normal, 1 = invert, 2 = grayscale, 3 = sharpen, 4 = blur, 5 = edge detection

const float offset = 1.0 / 300.0;

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    if (effect == 0) {
        // Normal - no post-processing
        FragColor = vec4(color, 1.0);
    }
    else if (effect == 1) {
        // Invert colors
        FragColor = vec4(vec3(1.0 - color), 1.0);
    }
    else if (effect == 2) {
        // Grayscale
        float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
        FragColor = vec4(average, average, average, 1.0);
    }
    else if (effect == 3) {
        // Sharpen kernel
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), vec2( 0.0,    offset), vec2( offset,  offset),
            vec2(-offset,  0.0),    vec2( 0.0,    0.0),    vec2( offset,  0.0),
            vec2(-offset, -offset), vec2( 0.0,   -offset), vec2( offset, -offset)
        );

        float kernel[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++) {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    else if (effect == 4) {
        // Blur kernel
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), vec2( 0.0,    offset), vec2( offset,  offset),
            vec2(-offset,  0.0),    vec2( 0.0,    0.0),    vec2( offset,  0.0),
            vec2(-offset, -offset), vec2( 0.0,   -offset), vec2( offset, -offset)
        );

        float kernel[9] = float[](
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++) {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    else if (effect == 5) {
        // Edge detection kernel
        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), vec2( 0.0,    offset), vec2( offset,  offset),
            vec2(-offset,  0.0),    vec2( 0.0,    0.0),    vec2( offset,  0.0),
            vec2(-offset, -offset), vec2( 0.0,   -offset), vec2( offset, -offset)
        );

        float kernel[9] = float[](
            1,  1,  1,
            1, -8,  1,
            1,  1,  1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++) {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    else {
        // Default to normal
        FragColor = vec4(color, 1.0);
    }
}
