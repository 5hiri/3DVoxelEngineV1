#include "Cube.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

// Initialize static members
unsigned int Cube::VAO = 0;
unsigned int Cube::VBO = 0;
unsigned int Cube::modelLoc = 0;

// Define the cube's vertex glBufferData
float Cube::vertices[] = {
    // Positions          // Colors
    // Front face (z = 0.5f)
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, // Bottom-left (Red)
     0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f, // Bottom-right (Green)
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Top-right (Blue)
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Top-right (Blue)
    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f, // Top-left (Yellow)
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, // Bottom-left (Red)
    // Back face (z = -0.5f)
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Bottom-left (Magenta)
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 1.0f, // Top-right (Cyan)
     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // Bottom-right (Green)
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 1.0f, // Top-right (Cyan)
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Bottom-left (Magenta)
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // Top-left (White)
    // Left face
    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f, // Top-right (Yellow)
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // Top-left (White)
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Bottom-left (Magenta)
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Bottom-left (Magenta)
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, // Bottom-right (Red)
    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f, // Top-right (Yellow)
    // Right face
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Top-left (Blue)
     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // Bottom-right (Green)
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Bottom-left (Blue)
     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // Bottom-right (Green)
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Top-left (Blue)
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 1.0f, // Top-right (Cyan)
    // Bottom face
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Top-right (Magenta)
     0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // Top-left (Green)
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Bottom-left (Blue)
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Bottom-left (Blue)
    -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, // Bottom-right (Red)
    -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f, // Top-right (Magenta)
    // Top face
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // Top-left (White)
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Bottom-right (Blue)
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 1.0f, // Top-right (Cyan)
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, // Bottom-right (Blue)
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // Top-left (White)
    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f  // Bottom-left (Yellow)
};

// Constructor implementations
Cube::Cube()
 : position(0.0f), rotationAxis(0.0f, 1.0f, 0.0f), rotationAngle(0.0f), scale(1.0f) {
 updateModelMatrix();
}

Cube::Cube(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale)
 : position(position), rotationAxis(rotationAxis), rotationAngle(rotationAngle), scale(scale) {
 updateModelMatrix();
}

Cube::~Cube() {
 // no need to delete VAO and VBO here since they are shared among all cubes
}

void Cube::initBuffers() {
 if (VAO == 0) {
  // Generate and bind VAO and VBO
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Unbind VAO
  glBindVertexArray(0);
 }
}

// Update model matrix
void Cube::updateModelMatrix() {
 modelMatrix = glm::mat4(1.0f); // Identify Matrix
 modelMatrix = glm::translate(modelMatrix, position);
 modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), rotationAxis);
 modelMatrix = glm::scale(modelMatrix, scale);
}

// Draw the cube
void Cube::draw(unsigned int shaderProgram) {
 // use the shader program
 glUseProgram(shaderProgram);

 // Set the model matrix uniform
 glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

 // Bind the VAO
 glBindVertexArray(VAO);

 // Draw the cube
 glDrawArrays(GL_TRIANGLES, 0, 36);

 // Unbind the VAO
 glBindVertexArray(0);
}

void Cube::cleanup() {
 if (VAO != 0) {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  VAO = 0;
  VBO = 0;
 }
}