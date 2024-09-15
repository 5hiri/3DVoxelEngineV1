#version 330 core
in vec3 ourColor;       // Input from vertex shader

out vec4 FragColor;     // Output color

void main()
{
    FragColor = vec4(ourColor, 1.0); // Set the fragment color
}