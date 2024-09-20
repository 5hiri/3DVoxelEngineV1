//
// Created by Salva on 20/09/2024.
//

#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cube {
public:
    // Constructors
    Cube();
    Cube(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale);
    // Desctructor
    ~Cube();

    // Static method to initialize buffers
    static void initBuffers();

    // Methods
    void updateModelMatrix();
    void draw(unsigned int shaderProgram);
    static void cleanup();

    // Transformation properties
    glm::vec3 position;
    glm::vec3 rotationAxis;
    float rotationAngle;
    glm::vec3 scale;

    // Uniform location cache
    static unsigned int modelLoc;

private:
    // Model matrix
    glm::mat4 modelMatrix;

    // Static VAO and VBO
    static unsigned int VAO, VBO;

    // Static vertex data
    static float vertices[];
};

#endif //CUBE_H
