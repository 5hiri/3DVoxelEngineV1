#include <any>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Cube.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <array>
#include "Frustum.h"

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow *window);
unsigned int compileShader(const char* shaderSource, GLenum shaderType);
std::string readShaderFile(const char* filePath);

bool gladLoadGL(GLADloadproc gla_dloadproc);

// camera settings
glm::vec3 cameraPos = glm::vec3(-3.0f, 3.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.63f, -0.49f, -0.61f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f; // time between current and last frame
float lastFrame = 0.0f; //time of last frame

// for mouse movement
float yaw = -90.0f; // yaw is initialized to -90.0 degrees
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

const int CHUNK_SIZE = 10; // Adjust this value as needed

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

struct CubeHandler {
    Cube cube;
    float size;
    bool isSplit;
    std::array<CubeHandler*, 8> children{};

    CubeHandler(const Cube& cube_, float s)
        : cube(cube_), size(s), isSplit(false) {
        children.fill(nullptr);
    }

    ~CubeHandler() {
        // Recursively delete child cubes
        for (auto child : children) {
            delete child;
        }
    }
};

struct Layer {
    int y;
    std::array<std::array<CubeHandler*, CHUNK_SIZE>, CHUNK_SIZE> cubes;

    Layer(int y_)
        : y(y_) {
        for (auto& row : cubes) {
            row.fill(nullptr);
        }
    }

    ~Layer() {
        for (auto& row : cubes) {
            for (auto cube : row) {
                delete cube;
            }
        }
    }
};

struct Chunk {
    int x, y, z;
    std::array<Layer*, CHUNK_SIZE> layers;

    Chunk(int x_, int y_, int z_)
        : x(x_), y(y_), z(z_) {
        layers.fill(nullptr);
    }

    ~Chunk() {
        for (auto layer : layers) {
            delete layer;
        }
    }
};

void buildTestCubeTree(CubeHandler& cubeHandler, int maxDepth);
void buildCubeTree(CubeHandler& cubeHandler, int maxDepth);
void renderCubes(CubeHandler& cubeHandler, unsigned int shaderProgram, int& n, const Frustum& frustum);
void generateChunk(Chunk& chunk);
void renderChunk(Chunk& chunk, unsigned int shaderProgram, int& n, const Frustum& frustum);

int main() {
    // std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
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
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Voxel Engine", nullptr, nullptr);
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

    // set viewport and callback
    glViewport(0, 0, 1200, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Register Mouse Callback
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize cube buffers (call only once)
    Cube::initBuffers();

    // Use shader program
    glUseProgram(shaderProgram);

    // Get uniform locations
    Cube::modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc  = glGetUniformLocation(shaderProgram, "projection");

    // std::vector<glm::vec3> cubePositions;
    // float cubeScales[];
    std::vector<Cube> cubes;
    Cube rootCube(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.0f));
    CubeHandler root(rootCube, 1.0f);
    // buildTestCubeTree(root, 3);
    // std::vector<Cube> cubes;
    // for (unsigned int i = 0; i < cubeLayers[].size(); i++) {
    //     cubes.push_back(Cube(glm::vec3(cubeLayers[i][0]), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(cubeLayers[i][1])));
    // }

    Chunk rootChunk(0, 0, 0);
    generateChunk(rootChunk);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // In your main loop
    int frameCount = 0;
    double lastTime = glfwGetTime();
    double fps = 0.0;

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Frustum frustum;
    float frustumMargin = 0.9f; // Adjust this value as needed

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        int cubeNum = 0;
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        frameCount++;
        if (currentFrame - lastTime >= 1.0) { // If a second has passed
            fps = frameCount / (currentFrame - lastTime);
            frameCount = 0;
            lastTime = currentFrame;
        }

        // input
        processInput(window);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Set clear color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up the transformation matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        float fov = 90.0f;
        glm::mat4 projection = glm::perspective(glm::radians(fov), static_cast<float>(1200)/800, 0.1f, 100.0f);
        //glm::mat4 model = glm::mat4(1.0f);

        // Update the frustum
        frustum.update(projection * view, frustumMargin);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Update and draw each cube
        // for (auto& cube : cubes) {
        //     // Update cube's transformation (if needed)
        //     // cube.rotationAngle += deltaTime * 20.0f; // Rotate over time
        //     // cube.updateModelMatrix();
        //
        //     // Draw the cube
        //     cube.draw(shaderProgram);
        // }
        // renderCubes(root, shaderProgram, cubeNum);
        renderChunk(rootChunk, shaderProgram, cubeNum, frustum);

        // Create an ImGui window to display stats
        ImGui::SetNextWindowPos(ImVec2(10, 10)); // Position at (10,10)
        ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS: %f", fps);
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Camera Direction: (%.2f, %.2f, %.2f)", cameraFront.x, cameraFront.y, cameraFront.z);
        ImGui::Text("Number of Cubes: %d", cubeNum);
        ImGui::End();

        // Render ImGui on top of the scene
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate resources
    glDeleteProgram(shaderProgram);

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    Cube::cleanup();
    glfwTerminate();
    return 0;
}

// Process all input
void processInput(GLFWwindow *window) {
    float cameraSpeed = 3.0f * deltaTime; // Adjust accordingly

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

// Split cube
void splitCube(CubeHandler& cubeHandler){
    if (cubeHandler.isSplit) return; // Already split
    float childSize = cubeHandler.size / 2.0f; // Each child cube is eight the size
    float offset = childSize / 2.0f; // Offset from the parent cubes center

    int index = 0;
    for (int dx = -1; dx <= 1; dx+= 2) {
        for (int dy = -1; dy <= 1; dy += 2) {
            for (int dz = -1; dz <= 1; dz += 2) {
                glm::vec3 childPos = cubeHandler.cube.position + glm::vec3(dx*offset, dy*offset, dz*offset);
                Cube childCube(childPos, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(childSize));
                CubeHandler* newHandler = new CubeHandler(childCube, childSize);
                cubeHandler.children[index] = newHandler;
                index++;
            }
        }
    }
    cubeHandler.isSplit = true;
}

// Build and traverse the cube tree
// void buildCubeTree(CubeHandler& cubeHandler, int maxDepth) {
//     if (maxDepth <= 0) return; // Base case: stop recursion
//
//     // Decide whether to split this cube
//     if (/* condition to split*/) {
//         splitCube(cubeHandler);
//         // Recursively build the tree for each child
//         for (auto child : cubeHandler.children) {
//             if (child != nullptr) {
//                 buildCubeTree(*child, maxDepth - 1);
//             }
//         }
//     }
// }

// Test build cube tree
void buildTestCubeTree(CubeHandler& cubeHandler, int maxDepth) {
    if (maxDepth <= 0) return; // Base case: stop recursion

    splitCube(cubeHandler);
    int index = 0;
    for (auto child : cubeHandler.children) {
        if (index == 0 || index == 3 || index == 5 || index == 7) {
            if (child != nullptr) {
                buildTestCubeTree(*child, maxDepth-1);
            }
        }
        index++;
    }
}


void renderCubes(CubeHandler& cubeHandler, unsigned int shaderProgram, int& n, const Frustum& frustum) {
    if (cubeHandler.isSplit) {
        // Traverse and render child cubes
        for (auto child : cubeHandler.children) {
            if (child != nullptr) {
                // Check if the child's bounding box is in the frustum
                glm::vec3 childMin = child->cube.position - (child->size / 2.0f);
                glm::vec3 childMax = child->cube.position + (child->size / 2.0f);
                if (frustum.isAABBInFrustum(childMin, childMax)) {
                    renderCubes(*child, shaderProgram, n, frustum);
                }
            }
        }
    } else {
        // Check if the cube is in the frustum before rendering
        if (frustum.isPointInFrustum(cubeHandler.cube.position)) {
            // Render the cube
            cubeHandler.cube.draw(shaderProgram);
            n++;
        }
    }
}

void generateChunk(Chunk& chunk) {
    int baseX = chunk.x;
    int baseY = chunk.y;
    int baseZ = chunk.z;

    for (int layerIndex = 0; layerIndex < CHUNK_SIZE; layerIndex++) {
        int currentY = baseY + layerIndex;
        Layer* newLayer = new Layer(currentY);
        chunk.layers[layerIndex] = newLayer;

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                float cubeX = baseX + x;
                float cubeY = currentY;
                float cubeZ = baseZ + z;

                // Create the cube at the calculated position
                Cube cube(glm::vec3(cubeX, cubeY, cubeZ),
                          glm::vec3(0.0f, 1.0f, 0.0f),
                          0.0f,
                          glm::vec3(1.0f));
                CubeHandler* newCubeHandler = new CubeHandler(cube, 1.0f);

                newLayer->cubes[x][z] = newCubeHandler;
            }
        }
    }
}

void renderChunk(Chunk& chunk, unsigned int shaderProgram, int& n, const Frustum& frustum) {
    glm::vec3 chunkMin(chunk.x, chunk.y, chunk.z);
    glm::vec3 chunkMax(chunk.x + CHUNK_SIZE, chunk.y + CHUNK_SIZE, chunk.z + CHUNK_SIZE);

    // Check if the entire chunk is in the frustum
    if (!frustum.isAABBInFrustum(chunkMin, chunkMax)) {
        return; // Skip this chunk if its not in the frustum
    }

    for (auto layer : chunk.layers) {
        if (layer != nullptr) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    CubeHandler* cube = layer->cubes[x][z];
                    if (cube != nullptr) {
                        renderCubes(*cube, shaderProgram, n, frustum);
                    }
                }
            }
        }
    }
}