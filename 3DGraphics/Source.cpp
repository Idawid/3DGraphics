#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/fwd.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

// Function prototypes
void initializeGLFW(GLFWwindow*& window);
void initializeOpenGL(GLFWwindow* window);
void initializeImGui(GLFWwindow* window);
void createShaders(unsigned int& shaderProgram);
void generateVertices(float* vertices);
void generateColors(float* colors);
void createVertexData(unsigned int& VAO, unsigned int& VBO, unsigned int& colorVBO);
glm::mat4 calculateProjectionMatrix(float aspectRatio, float fovRadians, float nearPlane, float farPlane);
glm::mat4 calculateViewMatrix(float cameraDistance, float rotationAngleX, float rotationAngleY);
glm::mat4 translateMatrix(const glm::mat4& matrix, const glm::vec3& translation);
glm::mat4 rotateMatrix(const glm::mat4& matrix, float rotationAngle, const glm::vec3& axis);
glm::quat axisAngleToQuaternion(float angle, const glm::vec3& axis);
glm::mat4 quaternionToMatrix(const glm::quat& quaternion);
void renderLoop(GLFWwindow* window, unsigned int shaderProgram, unsigned int VAO);
void processInput(GLFWwindow* window);
void cleanup(GLFWwindow* window, unsigned int VAO, unsigned int VBO);

// Rotation variables
float rotationAngleX = 0.0f;
float rotationAngleY = 0.0f;
float rotationSpeed = 100.0f;
// Initial distance from the camera
float cameraDistance = -5.0f;

int main() {
    GLFWwindow* window;
    initializeGLFW(window);
    initializeOpenGL(window);
    initializeImGui(window);

    unsigned int shaderProgram;
    createShaders(shaderProgram);

    unsigned int VAO, VBO, colorVBO;
    createVertexData(VAO, VBO, colorVBO);

    renderLoop(window, shaderProgram, VAO);

    cleanup(window, VAO, VBO);

    return 0;
}

void initializeGLFW(GLFWwindow*& window) {
    glfwInit();
    window = glfwCreateWindow(600, 900, "Cube Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
}

void initializeOpenGL(GLFWwindow* window) {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glEnable(GL_DEPTH_TEST); // Enable depth testing
}

void initializeImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void createShaders(unsigned int& shaderProgram) {
    const char* vertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n" 
        "out vec3 color;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 model;\n"
        "out float depth;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "   depth = gl_Position.z / gl_Position.w;\n"
        "   color = aColor;\n"
        "}\0";

    const char* fragmentShaderSource =
        "#version 330 core\n"
        "in vec3 color;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(color, 1.0);\n"
        "}\n";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
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

void createVertexData(unsigned int& VAO, unsigned int& VBO, unsigned int& colorVBO) {
    float vertices[36 * 3];
    float colors[36 * 3];

    generateVertices(vertices);
    generateColors(colors);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &colorVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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

bool isInsideTriangle(int x, int y, const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2) {
    glm::vec2 point(x, y);

    glm::vec2 edge0 = v1 - v0;
    glm::vec2 edge1 = v2 - v1;
    glm::vec2 edge2 = v0 - v2;

    glm::vec2 c0 = point - v0;
    glm::vec2 c1 = point - v1;
    glm::vec2 c2 = point - v2;

    float d0 = edge0.x * c0.y - edge0.y * c0.x;
    float d1 = edge1.x * c1.y - edge1.y * c1.x;
    float d2 = edge2.x * c2.y - edge2.y * c2.x;

    return (d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0);
}

glm::vec3 interpolateAttributes(int x, int y, const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec3& attr0, const glm::vec3& attr1, const glm::vec3& attr2) {
    glm::vec2 point(x, y);

    float area = 0.5f * ((v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y));

    float w0 = 0.5f * ((v1.y - v2.y) * (point.x - v2.x) + (v2.x - v1.x) * (point.y - v2.y)) / area;
    float w1 = 0.5f * ((v2.y - v0.y) * (point.x - v2.x) + (v0.x - v2.x) * (point.y - v2.y)) / area;
    float w2 = 1.0f - w0 - w1;

    glm::vec3 interpolatedAttr = w0 * attr0 + w1 * attr1 + w2 * attr2;
    return interpolatedAttr;
}

void setPixelColor(int x, int y, const glm::vec3& color) {
    // Set the color of the pixel at (x, y)
    glViewport(x, y, 1, 1);
    glClearColor(color.r, color.g, color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderLoop(GLFWwindow* window, unsigned int shaderProgram, unsigned int VAO) {
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    // Create Z-Buffer
    float* zBuffer = new float[screenWidth * screenHeight];
    for (int i = 0; i < screenWidth * screenHeight; ++i) {
        zBuffer[i] = -1.0f;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        processInput(window);

        // Clear and render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Calculate aspect ratio
        float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
        // Calculate projection matrix
        glm::mat4 projection = calculateProjectionMatrix(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        // Calculate the view matrix
        glm::mat4 view = calculateViewMatrix(cameraDistance, rotationAngleX, rotationAngleY);

        // Set the projection matrix in the shader
        GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        // Set the view matrix in the shader
        GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));


        glBindVertexArray(VAO);
        GLint modelLocation = glGetUniformLocation(shaderProgram, "model");

        // Render Cube 1
        glm::mat4 model1 = translateMatrix(glm::mat4(1.0f), glm::vec3(-0.7f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render Cube 2
        glm::mat4 model2 = translateMatrix(glm::mat4(1.0f), glm::vec3(0.7f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model2));
        glDrawArrays(GL_TRIANGLES, 0, 36);

       /* float vertices[36 * 3];
        generateVertices(vertices);
        float colors[36 * 3];
        generateColors(vertices);*/

        //for (int i = 0; i < 2; ++i) {
        //    // Calculate the model matrix for the current cube
        //    glm::mat4 model;
        //    if (i == 0)
        //        model = translateMatrix(glm::mat4(1.0f), glm::vec3(-0.7f, 0.0f, 0.0f));
        //    else if (i == 1)
        //        model = translateMatrix(glm::mat4(1.0f), glm::vec3(0.7f, 0.0f, 0.0f));
        //    // Set the model matrix in the shader
        //    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        //    // Render the cube using manual vertex and color data
        //    for (int j = 0; j < 36; j += 3) {
        //        // Retrieve the vertex position and color for the current triangle
        //        glm::vec3 vertex0(vertices[(i * 36 + j) * 3], vertices[(i * 36 + j) * 3 + 1], vertices[(i * 36 + j) * 3 + 2]);
        //        glm::vec3 vertex1(vertices[(i * 36 + j + 1) * 3], vertices[(i * 36 + j + 1) * 3 + 1], vertices[(i * 36 + j + 1) * 3 + 2]);
        //        glm::vec3 vertex2(vertices[(i * 36 + j + 2) * 3], vertices[(i * 36 + j + 2) * 3 + 1], vertices[(i * 36 + j + 2) * 3 + 2]);

        //        glm::vec3 color0(colors[(i * 36 + j) * 3], colors[(i * 36 + j) * 3 + 1], colors[(i * 36 + j) * 3 + 2]);
        //        glm::vec3 color1(colors[(i * 36 + j + 1) * 3], colors[(i * 36 + j + 1) * 3 + 1], colors[(i * 36 + j + 1) * 3 + 2]);
        //        glm::vec3 color2(colors[(i * 36 + j + 2) * 3], colors[(i * 36 + j + 2) * 3 + 1], colors[(i * 36 + j + 2) * 3 + 2]);

        //        // Perform any necessary transformations on the vertices
        //        vertex0 = glm::vec3(model * glm::vec4(vertex0, 1.0f));
        //        vertex1 = glm::vec3(model * glm::vec4(vertex1, 1.0f));
        //        vertex2 = glm::vec3(model * glm::vec4(vertex2, 1.0f));

        //        // Apply perspective division and viewport transformation
        //        glm::vec4 clipSpace0 = projection * view * glm::vec4(vertex0, 1.0f);
        //        glm::vec4 clipSpace1 = projection * view * glm::vec4(vertex1, 1.0f);
        //        glm::vec4 clipSpace2 = projection * view * glm::vec4(vertex2, 1.0f);

        //        glm::vec3 normalizedDeviceCoords0 = glm::vec3(clipSpace0) / clipSpace0.w;
        //        glm::vec3 normalizedDeviceCoords1 = glm::vec3(clipSpace1) / clipSpace1.w;
        //        glm::vec3 normalizedDeviceCoords2 = glm::vec3(clipSpace2) / clipSpace2.w;

        //        glm::vec2 screenCoords0 = glm::vec2((normalizedDeviceCoords0.x + 1.0f) * 0.5f * screenWidth, (normalizedDeviceCoords0.y + 1.0f) * 0.5f * screenHeight);
        //        glm::vec2 screenCoords1 = glm::vec2((normalizedDeviceCoords1.x + 1.0f) * 0.5f * screenWidth, (normalizedDeviceCoords1.y + 1.0f) * 0.5f * screenHeight);
        //        glm::vec2 screenCoords2 = glm::vec2((normalizedDeviceCoords2.x + 1.0f) * 0.5f * screenWidth, (normalizedDeviceCoords2.y + 1.0f) * 0.5f * screenHeight);

        //        // Perform rasterization
        //        int minX = std::min({ screenCoords0.x, screenCoords1.x, screenCoords2.x });
        //        int minY = std::min({ screenCoords0.y, screenCoords1.y, screenCoords2.y });
        //        int maxX = std::max({ screenCoords0.x, screenCoords1.x, screenCoords2.x });
        //        int maxY = std::max({ screenCoords0.y, screenCoords1.y, screenCoords2.y });

        //        for (int x = minX; x <= maxX; x++) {
        //            for (int y = minY; y <= maxY; y++) {

        //                if (isInsideTriangle(x, y, screenCoords0, screenCoords1, screenCoords2)) {
        //                    glm::vec3 interpolatedColor = interpolateAttributes(x, y, screenCoords0, screenCoords1, screenCoords2, color0, color1, color2);
        //                    setPixelColor(x, y, interpolatedColor);
        //                }
        //            }
        //        }

        //        // Perform fragment operations

        //        // Output the final color to the framebuffer
        //    }
        //}
 
        glReadPixels(0, 0, screenWidth, screenHeight, GL_DEPTH_COMPONENT, GL_FLOAT, zBuffer);

        // ImGui input handling
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    delete[] zBuffer;
}

//float vertices[36 * 3];
//generateVertices(vertices);
//float colors[36 * 3];
//generateColors(vertices);
//
//for (int i = 0; i < 2; ++i); // ... the implementation

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotationAngleY += rotationSpeed * ImGui::GetIO().DeltaTime;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotationAngleY -= rotationSpeed * ImGui::GetIO().DeltaTime;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotationAngleX += rotationSpeed * ImGui::GetIO().DeltaTime;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotationAngleX -= rotationSpeed * ImGui::GetIO().DeltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraDistance -= 0.01f;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraDistance += 0.01f;
}

void cleanup(GLFWwindow* window, unsigned int VAO, unsigned int VBO) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
}
