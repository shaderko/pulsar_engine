#version 450 core

#define CHUNK_SIZE 64

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 instancePosition;
layout(location = 3) in float instanceTextureIndex;

out vec3 GridPos;
out vec3 FragPos;
out vec2 TexCoords;
flat out float textureIndex;

uniform mat4 view;
uniform mat4 projection;
uniform sampler2DArray heightMaps;

void main() {
    vec3 displacedPos = ((aPos + instancePosition) * CHUNK_SIZE);

    gl_Position = projection * view * vec4(displacedPos, 1.0);

    GridPos = instancePosition * CHUNK_SIZE;
    FragPos = displacedPos;
    TexCoords = aTexCoords;
    textureIndex = instanceTextureIndex;
}