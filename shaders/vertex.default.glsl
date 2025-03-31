#version 330 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec2 vertexTexture;

out vec3 color;
out vec2 texturePosition;

uniform float scale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vertexPos, 1.0);
    color = vertexColor;
    texturePosition = vertexTexture;
}