#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIds;
layout(location = 4) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 150;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    // Calculate bone transformation
    mat4 boneTransform = mat4(0.0);

    for(int i = 0; i < 4; i++)
    {
        if(aBoneIds[i] == -1)
            continue;
        if(aBoneIds[i] >= MAX_BONES)
        {
            boneTransform = mat4(1.0);
            break;
        }
        boneTransform += finalBonesMatrices[aBoneIds[i]] * aWeights[i];
    }

    // If no bones influence this vertex, use identity
    if(boneTransform == mat4(0.0))
        boneTransform = mat4(1.0);

    vec4 totalPosition = boneTransform * vec4(aPos, 1.0);
    FragPos = vec3(model * totalPosition);

    mat3 normalMatrix = mat3(transpose(inverse(boneTransform)));
    Normal = mat3(transpose(inverse(model))) * normalMatrix * aNormal;

    TexCoords = aTexCoord;
    gl_Position = projection * view * model * totalPosition;
}
