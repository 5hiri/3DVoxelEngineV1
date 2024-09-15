#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow *window);
unsigned int compileShader(const char* shaderSource, GLenum shaderType);
std::string readShaderFile(const char* filePath);

bool gladLoadGL(GLADloadproc gla_dloadproc);

// camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f; // time between current and last frame
float lastFrame = 0.0f; //time of last frame

// for mouse movement
float yaw = -90.0f; // yaw is initialized to -90.0 degrees
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

void checkOpenGLError(const char* stmt, const char* fname, const int line)
{
    if (const GLenum err = glGetError(); err != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error " << err << " at " << fname << ":" << line << " - for " << stmt << std::endl;
        exit(1);
    }
}

#define GL_CHECK(stmt) do { \
    stmt; \
    checkOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)

int main() {
    //std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW3" << std::endl;
        return -1;
    }

    // Configure GLFW (OpenGL version 3.3, Core profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For macOS
#endif

    // create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Voxel Engine", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers with GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Build and compile shaders
    const std::string vertexCode = readShaderFile(SHADER_DIR "/vertex_shader.glsl");
    const std::string fragmentCode = readShaderFile(SHADER_DIR "/fragment_shader.glsl");
    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    const unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cerr << "Failed to compile vertex shader" << std::endl;
        return -1;
    }
    const unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cerr << "Failed to compile fragment shader" << std::endl;
        return -1;
    }

    // Shader Program
    const unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
        glDeleteProgram(shaderProgram);
        return -1;
    }

    // Delete shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // setup vertex data
    constexpr float vertices[] = {
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

    // Generate VAO and VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    //Bind the VAO
    glBindVertexArray(VAO);

    //Bind and Set the VBO Data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // set viewport and callback
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Register Mouse Callback
    glfwSetCursorPosCallback(window, mouse_callback);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Set clear color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the triangle
        glUseProgram(shaderProgram);

        // Set up the transformation matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)800/600, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Cleanup
    glfwTerminate();
    return 0;
}

// Process all input
void processInput(GLFWwindow *window) {
    float cameraSpeed = 2.5f * deltaTime; // Adjust accordingly

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraFront * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraFront * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

// GLFW: Whenever the window size changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Compile shaders
unsigned int compileShader(const char* shaderSource, GLenum shaderType) {
    unsigned int shader = glCreateShader(shaderType);
    if (!shader)
    {
        std::cerr << "ERROR::SHADER::GLCREATESHADER_FAILED\n";
        return 0;
    }

    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Rendering Shader from Files
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::stringstream shaderStream;

    try {
        shaderFile.open(filePath);
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
    }
    return shaderStream.str();
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Constrain the pitch angle
    if(pitch > 89.0f)
        pitch =  89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    // Update cameraFront vector
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
}