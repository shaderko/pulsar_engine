#version 460 core

// World position of the chunk
layout (location = 0) in vec3 position;

// Camera view and projection
uniform mat4 view;
uniform mat4 projection;

out vec4 fragPosition;

void main()
{
    fragPosition = projection * view * vec4(position, 1.0);
    gl_Position = fragPosition;
}
