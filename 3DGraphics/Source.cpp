#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>

// The texture parameters
GLuint textureId;
int textureWidth = 600, textureHeight = 600;
glm::vec3 pixels[600 * 600];
GLuint VAO, VBO, EBO;
GLuint shaderProgram;
float rotationAngleX = 0.0f;
float rotationAngleY = 0.0f;
float rotationSpeed = 100.0f;
float cameraDistance = -5.0f;

void initializeGLFW(GLFWwindow*& window);
void initializeOpenGL(GLFWwindow* window);
void createTexture();
void setPixelColor(int x, int y, const glm::vec3& color);
void createShaders();
void createQuad();
void generateVertices(float* vertices);
void generateColors(float* colors);
glm::mat4 calculateProjectionMatrix(float aspectRatio, float fovRadians, float nearPlane, float farPlane);
glm::mat4 calculateViewMatrix(float cameraDistance, float rotationAngleX, float rotationAngleY);
glm::mat4 translateMatrix(const glm::mat4& matrix, const glm::vec3& translation);
glm::mat4 rotateMatrix(const glm::mat4& matrix, float rotationAngle, const glm::vec3& axis);
glm::quat axisAngleToQuaternion(float angle, const glm::vec3& axis);
glm::mat4 quaternionToMatrix(const glm::quat& quaternion);
void drawCubes(GLFWwindow* window, const float* vertices, const float* colors);
void drawScene(GLFWwindow* window);
void processInput(GLFWwindow* window, double deltaTime);
void cleanup();


int main() {
    GLFWwindow* window;

    // Initialize GLFW
    initializeGLFW(window);

    // Initialize OpenGL
    initializeOpenGL(window);

    // Create Texture
    createTexture();

    // Create Shaders
    createShaders();

    // Create Quad
    createQuad();

    // Draw Scene
    drawScene(window);

    // Cleanup
    cleanup();

    return 0;
}

void initializeGLFW(GLFWwindow*& window) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }
    window = glfwCreateWindow(textureWidth, textureHeight, "Cube Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
}

void initializeOpenGL(GLFWwindow* window) {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void createTexture() {
    // Generate and bind texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
}

void updateTexture() {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGB, GL_FLOAT, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void setPixelColor(int x, int y, const glm::vec3& color) {
    // Calculate the pixel index
    int index = y * textureWidth + x;

    // Set the color of the pixel
    pixels[index] = color;
}

void createShaders() {
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main()
        {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        })";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D ourTexture;
        void main()
        {
            FragColor = texture(ourTexture, TexCoord);
        })";

    // Create and compile the vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);

    // Create a shader program and link the shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete the shader objects, we don't need them anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void createQuad() {
    float vertices[] = {
        // positions   // tex coords
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    };

    // Create a VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create a VBO and copy the vertex data
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create an EBO and copy the index data
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void generateVertices(float* vertices) {
    // Cube 1
    // Back face
    vertices[0] = -0.5f; vertices[1] = -0.5f; vertices[2] = -0.5f; // Vertex 0
    vertices[3] = 0.5f; vertices[4] = -0.5f; vertices[5] = -0.5f; // Vertex 1
    vertices[6] = 0.5f; vertices[7] = 0.5f; vertices[8] = -0.5f;  // Vertex 2
    vertices[9] = 0.5f; vertices[10] = 0.5f; vertices[11] = -0.5f; // Vertex 3
    vertices[12] = -0.5f; vertices[13] = 0.5f; vertices[14] = -0.5f; // Vertex 4
    vertices[15] = -0.5f; vertices[16] = -0.5f; vertices[17] = -0.5f; // Vertex 5

    // Front face
    vertices[18] = -0.5f; vertices[19] = -0.5f; vertices[20] = 0.5f; // Vertex 6
    vertices[21] = 0.5f; vertices[22] = -0.5f; vertices[23] = 0.5f; // Vertex 7
    vertices[24] = 0.5f; vertices[25] = 0.5f; vertices[26] = 0.5f; // Vertex 8
    vertices[27] = 0.5f; vertices[28] = 0.5f; vertices[29] = 0.5f; // Vertex 9
    vertices[30] = -0.5f; vertices[31] = 0.5f; vertices[32] = 0.5f; // Vertex 10
    vertices[33] = -0.5f; vertices[34] = -0.5f; vertices[35] = 0.5f; // Vertex 11

    // Left face
    vertices[36] = -0.5f; vertices[37] = 0.5f; vertices[38] = 0.5f; // Vertex 12
    vertices[39] = -0.5f; vertices[40] = 0.5f; vertices[41] = -0.5f; // Vertex 13
    vertices[42] = -0.5f; vertices[43] = -0.5f; vertices[44] = -0.5f; // Vertex 14
    vertices[45] = -0.5f; vertices[46] = -0.5f; vertices[47] = -0.5f; // Vertex 15
    vertices[48] = -0.5f; vertices[49] = -0.5f; vertices[50] = 0.5f; // Vertex 16
    vertices[51] = -0.5f; vertices[52] = 0.5f; vertices[53] = 0.5f; // Vertex 17

    // Right face
    vertices[54] = 0.5f; vertices[55] = 0.5f; vertices[56] = 0.5f; // Vertex 18
    vertices[57] = 0.5f; vertices[58] = 0.5f; vertices[59] = -0.5f; // Vertex 19
    vertices[60] = 0.5f; vertices[61] = -0.5f; vertices[62] = -0.5f; // Vertex 20
    vertices[63] = 0.5f; vertices[64] = -0.5f; vertices[65] = -0.5f; // Vertex 21
    vertices[66] = 0.5f; vertices[67] = -0.5f; vertices[68] = 0.5f; // Vertex 22
    vertices[69] = 0.5f; vertices[70] = 0.5f; vertices[71] = 0.5f; // Vertex 23

    // Bottom face
    vertices[72] = -0.5f; vertices[73] = -0.5f; vertices[74] = -0.5f; // Vertex 24
    vertices[75] = 0.5f; vertices[76] = -0.5f; vertices[77] = -0.5f; // Vertex 25
    vertices[78] = 0.5f; vertices[79] = -0.5f; vertices[80] = 0.5f; // Vertex 26
    vertices[81] = 0.5f; vertices[82] = -0.5f; vertices[83] = 0.5f; // Vertex 27
    vertices[84] = -0.5f; vertices[85] = -0.5f; vertices[86] = 0.5f; // Vertex 28
    vertices[87] = -0.5f; vertices[88] = -0.5f; vertices[89] = -0.5f; // Vertex 29

    // Top face
    vertices[90] = -0.5f; vertices[91] = 0.5f; vertices[92] = -0.5f; // Vertex 30
    vertices[93] = 0.5f; vertices[94] = 0.5f; vertices[95] = -0.5f; // Vertex 31
    vertices[96] = 0.5f; vertices[97] = 0.5f; vertices[98] = 0.5f; // Vertex 32
    vertices[99] = 0.5f; vertices[100] = 0.5f; vertices[101] = 0.5f; // Vertex 33
    vertices[102] = -0.5f; vertices[103] = 0.5f; vertices[104] = 0.5f; // Vertex 34
    vertices[105] = -0.5f; vertices[106] = 0.5f; vertices[107] = -0.5f; // Vertex 35
}

void generateColors(float* colors) {
    // Back face
    colors[0] = 1.0f; colors[1] = 0.0f; colors[2] = 0.0f;   // Vertex 0 color (red)
    colors[3] = 1.0f; colors[4] = 0.0f; colors[5] = 0.0f;   // Vertex 1 color (red)
    colors[6] = 1.0f; colors[7] = 0.0f; colors[8] = 0.0f;   // Vertex 2 color (red)
    colors[9] = 1.0f; colors[10] = 0.0f; colors[11] = 0.0f; // Vertex 3 color (red)
    colors[12] = 1.0f; colors[13] = 0.0f; colors[14] = 0.0f; // Vertex 4 color (red)
    colors[15] = 1.0f; colors[16] = 0.0f; colors[17] = 0.0f; // Vertex 5 color (red)

    // Front face
    colors[18] = 0.0f; colors[19] = 1.0f; colors[20] = 0.0f;   // Vertex 6 color (green)
    colors[21] = 0.0f; colors[22] = 1.0f; colors[23] = 0.0f;   // Vertex 7 color (green)
    colors[24] = 0.0f; colors[25] = 1.0f; colors[26] = 0.0f;   // Vertex 8 color (green)
    colors[27] = 0.0f; colors[28] = 1.0f; colors[29] = 0.0f;   // Vertex 9 color (green)
    colors[30] = 0.0f; colors[31] = 1.0f; colors[32] = 0.0f;   // Vertex 10 color (green)
    colors[33] = 0.0f; colors[34] = 1.0f; colors[35] = 0.0f;   // Vertex 11 color (green)

    // Left face
    colors[36] = 0.0f; colors[37] = 0.0f; colors[38] = 1.0f;   // Vertex 12 color (blue)
    colors[39] = 0.0f; colors[40] = 0.0f; colors[41] = 1.0f;   // Vertex 13 color (blue)
    colors[42] = 0.0f; colors[43] = 0.0f; colors[44] = 1.0f;   // Vertex 14 color (blue)
    colors[45] = 0.0f; colors[46] = 0.0f; colors[47] = 1.0f;   // Vertex 15 color (blue)
    colors[48] = 0.0f; colors[49] = 0.0f; colors[50] = 1.0f;   // Vertex 16 color (blue)
    colors[51] = 0.0f; colors[52] = 0.0f; colors[53] = 1.0f;   // Vertex 17 color (blue)

    // Right face
    colors[54] = 1.0f; colors[55] = 1.0f; colors[56] = 0.0f;   // Vertex 18 color (yellow)
    colors[57] = 1.0f; colors[58] = 1.0f; colors[59] = 0.0f;   // Vertex 19 color (yellow)
    colors[60] = 1.0f; colors[61] = 1.0f; colors[62] = 0.0f;   // Vertex 20 color (yellow)
    colors[63] = 1.0f; colors[64] = 1.0f; colors[65] = 0.0f;   // Vertex 21 color (yellow)
    colors[66] = 1.0f; colors[67] = 1.0f; colors[68] = 0.0f;   // Vertex 22 color (yellow)
    colors[69] = 1.0f; colors[70] = 1.0f; colors[71] = 0.0f;   // Vertex 23 color (yellow)

    // Bottom face
    colors[72] = 0.0f; colors[73] = 1.0f; colors[74] = 1.0f;   // Vertex 24 color (cyan)
    colors[75] = 0.0f; colors[76] = 1.0f; colors[77] = 1.0f;   // Vertex 25 color (cyan)
    colors[78] = 0.0f; colors[79] = 1.0f; colors[80] = 1.0f;   // Vertex 26 color (cyan)
    colors[81] = 0.0f; colors[82] = 1.0f; colors[83] = 1.0f;   // Vertex 27 color (cyan)
    colors[84] = 0.0f; colors[85] = 1.0f; colors[86] = 1.0f;   // Vertex 28 color (cyan)
    colors[87] = 0.0f; colors[88] = 1.0f; colors[89] = 1.0f;   // Vertex 29 color (cyan)

    // Top face
    colors[90] = 1.0f; colors[91] = 0.0f; colors[92] = 1.0f;   // Vertex 30 color (magenta)
    colors[93] = 1.0f; colors[94] = 0.0f; colors[95] = 1.0f;   // Vertex 31 color (magenta)
    colors[96] = 1.0f; colors[97] = 0.0f; colors[98] = 1.0f;   // Vertex 32 color (magenta)
    colors[99] = 1.0f; colors[100] = 0.0f; colors[101] = 1.0f;   // Vertex 33 color (magenta)
    colors[102] = 1.0f; colors[103] = 0.0f; colors[104] = 1.0f;   // Vertex 34 color (magenta)
    colors[105] = 1.0f; colors[106] = 0.0f; colors[107] = 1.0f;   // Vertex 35 color (magenta)
}

glm::mat4 calculateProjectionMatrix(float aspectRatio, float fovRadians, float nearPlane, float farPlane) {
    // from: https://ogldev.org/www/tutorial12/tutorial12.html
    float tanHalfFov = tanf(fovRadians / 2.0f);

    glm::mat4 projectionMatrix(0.0f);
    projectionMatrix[0][0] = 1 / (tanHalfFov * aspectRatio); //1row [.. 0 0 0]
    projectionMatrix[1][1] = 1 / (tanHalfFov); // 2row [0 .. 0 0]
    projectionMatrix[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane); // 3row [0 0 .. ..]
    projectionMatrix[3][2] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    projectionMatrix[2][3] = -1.0f; // 4row [0 0 -1 0]

    return projectionMatrix;
}

glm::mat4 calculateViewMatrix(float cameraDistance, float rotationAngleX, float rotationAngleY) {
    // Calculate the view matrix manually
    glm::mat4 viewMatrix(1.0f);
    viewMatrix = translateMatrix(viewMatrix, glm::vec3(0.0f, 0.0f, cameraDistance));
    viewMatrix = rotateMatrix(viewMatrix, glm::radians(rotationAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    viewMatrix = rotateMatrix(viewMatrix, glm::radians(rotationAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

    return viewMatrix;
}

glm::mat4 translateMatrix(const glm::mat4& matrix, const glm::vec3& translation) {
    glm::mat4 result = matrix;

    result[3][0] = matrix[0][0] * translation.x + matrix[1][0] * translation.y + matrix[2][0] * translation.z + matrix[3][0];
    result[3][1] = matrix[0][1] * translation.x + matrix[1][1] * translation.y + matrix[2][1] * translation.z + matrix[3][1];
    result[3][2] = matrix[0][2] * translation.x + matrix[1][2] * translation.y + matrix[2][2] * translation.z + matrix[3][2];
    result[3][3] = matrix[0][3] * translation.x + matrix[1][3] * translation.y + matrix[2][3] * translation.z + matrix[3][3];

    return result;
}

glm::mat4 rotateMatrix(const glm::mat4& matrix, float rotationAngle, const glm::vec3& axis) {
    glm::quat rotationQuat = axisAngleToQuaternion(rotationAngle, axis);
    glm::mat4 rotationMatrix = quaternionToMatrix(rotationQuat);
    return matrix * rotationMatrix;
}

glm::quat axisAngleToQuaternion(float angle, const glm::vec3& axis) {
    float halfAngle = angle * 0.5f;
    float sinHalfAngle = sin(halfAngle);

    glm::quat quaternion;
    quaternion.w = cos(halfAngle);
    quaternion.x = axis.x * sinHalfAngle;
    quaternion.y = axis.y * sinHalfAngle;
    quaternion.z = axis.z * sinHalfAngle;

    return quaternion;
}

glm::mat4 quaternionToMatrix(const glm::quat& quaternion) {
    float xx = quaternion.x * quaternion.x;
    float xy = quaternion.x * quaternion.y;
    float xz = quaternion.x * quaternion.z;
    float xw = quaternion.x * quaternion.w;

    float yy = quaternion.y * quaternion.y;
    float yz = quaternion.y * quaternion.z;
    float yw = quaternion.y * quaternion.w;

    float zz = quaternion.z * quaternion.z;
    float zw = quaternion.z * quaternion.w;

    glm::mat4 rotationMatrix;
    rotationMatrix[0][0] = 1.0f - 2.0f * (yy + zz);
    rotationMatrix[0][1] = 2.0f * (xy - zw);
    rotationMatrix[0][2] = 2.0f * (xz + yw);
    rotationMatrix[0][3] = 0.0f;

    rotationMatrix[1][0] = 2.0f * (xy + zw);
    rotationMatrix[1][1] = 1.0f - 2.0f * (xx + zz);
    rotationMatrix[1][2] = 2.0f * (yz - xw);
    rotationMatrix[1][3] = 0.0f;

    rotationMatrix[2][0] = 2.0f * (xz - yw);
    rotationMatrix[2][1] = 2.0f * (yz + xw);
    rotationMatrix[2][2] = 1.0f - 2.0f * (xx + yy);
    rotationMatrix[2][3] = 0.0f;

    rotationMatrix[3][0] = 0.0f;
    rotationMatrix[3][1] = 0.0f;
    rotationMatrix[3][2] = 0.0f;
    rotationMatrix[3][3] = 1.0f;

    return rotationMatrix;
}

bool pointInTriangle(const glm::vec2& p, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
    auto det = [](const glm::vec2& u, const glm::vec2& v) { return u.x * v.y - u.y * v.x; };

    glm::vec2 a = glm::vec2(v0.x, v0.y) - p;
    glm::vec2 b = glm::vec2(v1.x, v1.y) - p;
    glm::vec2 c = glm::vec2(v2.x, v2.y) - p;
    
    float alpha = det(b, c) / 2.0f;
    float beta = det(c, a) / 2.0f;
    float gamma = det(a, b) / 2.0f;

    float sum = alpha + beta + gamma;

    alpha /= sum;
    beta /= sum;
    gamma /= sum;

    return alpha > 0 && beta > 0 && gamma > 0;
}

void barycentric(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, float& u, float& v, float& w) {
    glm::vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

float interpolateDepth(const glm::vec2& point, const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2) {
    // Compute barycentric coordinates (u, v, w) for point with respect to triangle (vertex0, vertex1, vertex2)
    float u, v, w;
    barycentric(point, glm::vec2(vertex0.x, vertex0.y), glm::vec2(vertex1.x, vertex1.y), glm::vec2(vertex2.x, vertex2.y), u, v, w);

    // Interpolate the depth using the barycentric coordinates
    return u * vertex0.z + v * vertex1.z + w * vertex2.z;
}

void drawCubes(GLFWwindow* window, const float* vertices, const float* colors, std::vector<float>& depthBuffer) {
    std::fill_n(pixels, textureWidth * textureHeight, glm::vec3(0.0f));

    // Calculate aspect ratio
    float aspectRatio = static_cast<float>(textureWidth) / static_cast<float>(textureHeight);
    // Calculate projection matrix
    glm::mat4 projection = calculateProjectionMatrix(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    // Calculate the view matrix
    glm::mat4 view = calculateViewMatrix(cameraDistance, rotationAngleX, rotationAngleY);

    for (int i = 0; i < 2; ++i) {
        // Calculate the model matrix for the current cube
        glm::mat4 model;
        if (i == 0)
            model = translateMatrix(glm::mat4(1.0f), glm::vec3(-0.7f, 0.0f, 0.0f));
        else if (i == 1)
            model = translateMatrix(glm::mat4(1.0f), glm::vec3(0.7f, 0.0f, 0.0f));

        // Render the cube using manual vertex and color data
        for (int j = 0; j < 36; j += 3) {
            // Retrieve the vertex position and color for the current triangle
            glm::vec3 vertex0(vertices[(j) * 3] , vertices[(j) * 3 + 1], vertices[(j) * 3 + 2]);
            glm::vec3 vertex1(vertices[(j + 1) * 3], vertices[(j + 1) * 3 + 1], vertices[(j + 1) * 3 + 2]);
            glm::vec3 vertex2(vertices[(j + 2) * 3], vertices[(j + 2) * 3 + 1], vertices[(j + 2) * 3 + 2]);

            glm::vec3 color0(colors[(j) * 3], colors[(j) * 3 + 1], colors[(j) * 3 + 2]);
            glm::vec3 color1(colors[(j + 1) * 3], colors[(j + 1) * 3 + 1], colors[(j + 1) * 3 + 2]);
            glm::vec3 color2(colors[(j + 2) * 3], colors[(j + 2) * 3 + 1], colors[(j + 2) * 3 + 2]);

            glm::vec4 vertex0_4d = projection * view * model * glm::vec4(vertex0, 1.0f);
            glm::vec4 vertex1_4d = projection * view * model * glm::vec4(vertex1, 1.0f);
            glm::vec4 vertex2_4d = projection * view * model * glm::vec4(vertex2, 1.0f);

            // Perspective division
            glm::vec3 vertex0_2d = glm::vec3(vertex0_4d) / vertex0_4d.w;
            glm::vec3 vertex1_2d = glm::vec3(vertex1_4d) / vertex1_4d.w;
            glm::vec3 vertex2_2d = glm::vec3(vertex2_4d) / vertex2_4d.w;

            // Rasterize the triangle
            float minX = std::min(vertex0_2d.x, std::min(vertex1_2d.x, vertex2_2d.x));
            float minY = std::min(vertex0_2d.y, std::min(vertex1_2d.y, vertex2_2d.y));
            float maxX = std::max(vertex0_2d.x, std::max(vertex1_2d.x, vertex2_2d.x));
            float maxY = std::max(vertex0_2d.y, std::max(vertex1_2d.y, vertex2_2d.y));

            // Convert to pixel coordinates
            int start_x = static_cast<int>((minX + 1.0f) * 0.5f * textureWidth - 1);
            int start_y = static_cast<int>((minY + 1.0f) * 0.5f * textureHeight - 1);
            int end_x = static_cast<int>((maxX + 1.0f) * 0.5f * textureWidth + 1);
            int end_y = static_cast<int>((maxY + 1.0f) * 0.5f * textureHeight + 1);

            // Clamp
            start_x = std::max(0, start_x);
            start_y = std::max(0, start_y);
            end_x = std::min(textureWidth, end_x);
            end_y = std::min(textureHeight, end_y);

            // Only iterate over pixels within the bounding box of the triangle
            for (int y = start_y; y < end_y; ++y) {
                for (int x = start_x; x < end_x; ++x) {
                    glm::vec2 pixelPosNDC = glm::vec2(2.0f * x / textureWidth - 1.0f, 2.0f * y / textureHeight - 1.0f);
                    if (pointInTriangle(pixelPosNDC, vertex0_2d, vertex1_2d, vertex2_2d)) {
                        float newDepth = interpolateDepth(pixelPosNDC, vertex0_2d, vertex1_2d, vertex2_2d);
                        float& currentDepth = depthBuffer[y * textureWidth + x];
                        if (newDepth < currentDepth) {
                            // If the new depth is smaller, update the color and depth
                            glm::vec3 color = (color0 + color1 + color2) / 3.0f;
                            setPixelColor(x, y, color);
                            currentDepth = newDepth;
                        }
                    }
                }
            }
        }
    }

    updateTexture();
}

void drawScene(GLFWwindow* window) {
    double lastFrameTime = glfwGetTime();
    std::vector<float> depthBuffer(textureWidth * textureHeight, std::numeric_limits<float>::infinity());
    float vertices[36 * 3];
    float colors[36 * 3];
    generateVertices(vertices);
    generateColors(colors);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        double currentFrameTime = glfwGetTime();
        processInput(window, currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<float>::infinity());

        // Use the shader program and bind the VAO
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        drawCubes(window, vertices, colors, depthBuffer);

        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Draw the quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }
}

void processInput(GLFWwindow* window, double deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotationAngleY += rotationSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotationAngleY -= rotationSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotationAngleX += rotationSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotationAngleX -= rotationSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraDistance -= 0.1f;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraDistance += 0.1f;
}

void cleanup() {
    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureId);
    glfwTerminate();
}