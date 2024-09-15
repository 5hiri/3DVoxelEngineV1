#version 330 core
layout (location = 0) in vec3 aPos;    // Position attribute
layout (location = 1) in vec3 aColor;  // Color attribute

out vec3 ourColor; // Output to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Set the vertex position
    ourColor = aColor;             // Pass the color to the fragment shader
}