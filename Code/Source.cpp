#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>

bool espParam = false;
bool affNorm = false;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(2.0f, 2.0f, 2.0f);

// Calcul des normales des triangles
void calculateNormals(const std::vector<std::vector<glm::vec3>>& bezierSurfacePoints, std::vector<std::vector<glm::vec3>>& normals) {
    normals.clear();
    normals.resize(bezierSurfacePoints.size());
    for (int i = 0; i < bezierSurfacePoints.size(); ++i) {
        normals[i].resize(bezierSurfacePoints[i].size());
    }
    // Parcours de chaque triangle pour calculer la normale
    for (int i = 0; i < bezierSurfacePoints.size() - 1; ++i) {
        for (int j = 0; j < bezierSurfacePoints[i].size() - 1; ++j) {
            // Calcul des normales pour chaque triangle
            glm::vec3 v0 = bezierSurfacePoints[i][j];
            glm::vec3 v1 = bezierSurfacePoints[i + 1][j];
            glm::vec3 v2 = bezierSurfacePoints[i][j + 1];

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            // Stockage de la normale pour chaque point du triangle
            normals[i][j] = normal;
            normals[i + 1][j] = normal;
            normals[i][j + 1] = normal;
        }
    }
}

// Algorithme de De Casteljau pour une seule dimension
glm::vec3 deCasteljau(const std::vector<glm::vec3>& points, float t) {
    std::vector<glm::vec3> temp(points);
    for (int i = points.size() - 1; i > 0; --i) {
        for (int j = 0; j < i; ++j) {
            temp[j] = (1.0f - t) * temp[j] + t * temp[j + 1];
        }
    }
    return temp[0];
}

glm::vec3 calculateBezierSurfacePoint(const std::vector<std::vector<glm::vec3>>& controlPoints, float u, float v) {
    int m = static_cast<int>(controlPoints.size()) - 1;
    int n = static_cast<int>(controlPoints[0].size()) - 1;
    // Appliquer l'algorithme de De Casteljau pour les points q avec u
    std::vector<glm::vec3> temp(m + 1);
    for (int j = 0; j <= m; ++j) {
        temp[j] = deCasteljau(controlPoints[j], u);
    }
    // Appliquer l'algorithme de De Casteljau pour chaque colonne avec v
    return deCasteljau(temp, v);
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("C:/Users/basti/Documents/M1/InfoGraphique/ProjectIGAI/ProjectIGAI/1.colors.vs", "C:/Users/basti/Documents/M1/InfoGraphique/ProjectIGAI/ProjectIGAI/1.colors.fs");
    Shader lightCubeShader("C:/Users/basti/Documents/M1/InfoGraphique/ProjectIGAI/ProjectIGAI/1.light_cube.vs", "C:/Users/basti/Documents/M1/InfoGraphique/ProjectIGAI/ProjectIGAI/1.light_cube.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // Definir les points de controle bidimensionnels pour la surface de Bezier
    std::vector<std::vector<glm::vec3>> controlPoints = {
        {glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(-0.5f, 1.0f, -1.0f), glm::vec3(0.5f, 1.0f, -1.0f), glm::vec3(1.0f, 0.0f, -1.0f)},
        {glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-0.5f, 0.25f, 0.0f), glm::vec3(0.5f, 0.25f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
        {glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(-0.5f, 1.0f, 1.0f), glm::vec3(0.5f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f)}
    };

    std::vector<float> controlPointsVertices;
    for (const auto& row : controlPoints) {
        for (const auto& point : row) {
            controlPointsVertices.push_back(point.x);
            controlPointsVertices.push_back(point.y);
            controlPointsVertices.push_back(point.z);
        }
    }

    // Creez les indices pour former des triangles pour le polygone de controle
    std::vector<unsigned int> controlPointsIndices;
    for (int i = 0; i < controlPoints.size() - 1; ++i) {
        for (int j = 0; j < controlPoints[i].size() - 1; ++j) {
            int currentIndex = i * controlPoints[i].size() + j;
            int nextIndex = currentIndex + 1;
            int belowIndex = (i + 1) * controlPoints[i].size() + j;
            // Premier triangle
            controlPointsIndices.push_back(currentIndex);
            controlPointsIndices.push_back(nextIndex);
            controlPointsIndices.push_back(belowIndex);
            // Deuxieme triangle
            controlPointsIndices.push_back(nextIndex);
            controlPointsIndices.push_back(belowIndex);
            controlPointsIndices.push_back(belowIndex + 1);
        }
    }

    unsigned int controlPointsVAO, controlPointsVBO, controlPointsEBO;
    glGenVertexArrays(1, &controlPointsVAO);
    glGenBuffers(1, &controlPointsVBO);
    glGenBuffers(1, &controlPointsEBO);

    glBindVertexArray(controlPointsVAO);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, controlPointsVertices.size() * sizeof(float), controlPointsVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, controlPointsEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, controlPointsIndices.size() * sizeof(unsigned int), controlPointsIndices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Calculez les points de la surface de Bezier
    std::vector<std::vector<glm::vec3>> bezierSurfacePoints;
    const int numSegmentsU = 25;
    const int numSegmentsV = 25;

    // Creez les indices pour former des triangles
    std::vector<unsigned int> indices;
    for (int i = 0; i < numSegmentsU; ++i) {
        for (int j = 0; j < numSegmentsV; ++j) {
            int index0 = i * (numSegmentsV + 1) + j;
            int index1 = index0 + 1;
            int index2 = (i + 1) * (numSegmentsV + 1) + j;
            int index3 = index2 + 1;
            // Premier triangle
            indices.push_back(index0);
            indices.push_back(index1);
            indices.push_back(index2);
            // Deuxieme triangle
            indices.push_back(index1);
            indices.push_back(index3);
            indices.push_back(index2);
        }
    }

    for (int i = 0; i <= numSegmentsU; ++i) {
        float u = static_cast<float>(i) / static_cast<float>(numSegmentsU);
        std::vector<glm::vec3> row;
        for (int j = 0; j <= numSegmentsV; ++j) {
            float v = static_cast<float>(j) / static_cast<float>(numSegmentsV);
            glm::vec3 point = calculateBezierSurfacePoint(controlPoints, u, v);
            row.push_back(point);
        }
        bezierSurfacePoints.push_back(row);
    }

    // Creez les donn�es de sommet pour la surface de Bezier
    std::vector<float> bezierSurfaceVertices;
    for (const auto& row : bezierSurfacePoints) {
        for (const auto& point : row) {
            bezierSurfaceVertices.push_back(point.x);
            bezierSurfaceVertices.push_back(point.y);
            bezierSurfaceVertices.push_back(point.z);
        }
    }

    // Creation et affichage normales
    std::vector<std::vector<glm::vec3>> normals;
    calculateNormals(bezierSurfacePoints, normals);
    std::vector<glm::vec3> normalVertices;
    for (int i = 0; i < bezierSurfacePoints.size(); ++i) {
        for (int j = 0; j < bezierSurfacePoints[i].size(); ++j) {
            glm::vec3 vertex = bezierSurfacePoints[i][j];
            glm::vec3 normal = normals[i][j];
            glm::vec3 endPoint = vertex - normal * 0.1f;

            normalVertices.push_back(endPoint);
            normalVertices.push_back(vertex);
        }
    }
    
    unsigned int normalVVBO, normalVVAO;
    glGenVertexArrays(1, &normalVVAO);
    glGenBuffers(1, &normalVVBO);

    glBindVertexArray(normalVVAO);

    glBindBuffer(GL_ARRAY_BUFFER, normalVVBO);
    glBufferData(GL_ARRAY_BUFFER, normalVertices.size() * sizeof(glm::vec3), normalVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<glm::vec3> finalData;
    for (unsigned int i = 0; i < bezierSurfacePoints.size(); i++) {
        for (unsigned int j = 0; j < bezierSurfacePoints[i].size(); j++) {
            finalData.push_back(bezierSurfacePoints[i][j]);
            finalData.push_back(-normals[i][j]);
        }
    }

    unsigned int normalVBO, normalVAO, normalEBO;
    glGenVertexArrays(1, &normalVAO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &normalEBO);

    glBindVertexArray(normalVAO);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, finalData.size() * sizeof(glm::vec3), finalData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);

    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        ourShader.use();
        ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("lightPos", lightPos);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(controlPointsVAO);
        glDrawElements(GL_TRIANGLES, controlPointsIndices.size(), GL_UNSIGNED_INT, 0);

        if (!espParam) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(normalVAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBindVertexArray(normalVAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

        if (affNorm) {
            glBindVertexArray(normalVVAO);
            glDrawArrays(GL_LINES, 0, normalVertices.size());
        }
        
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.1f));
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    glDeleteBuffers(1, &controlPointsVBO);
    glDeleteVertexArrays(1, &controlPointsVAO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteVertexArrays(1, &normalVAO);
    glDeleteBuffers(1, &normalVVBO);
    glDeleteVertexArrays(1, &normalVVAO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        espParam = !espParam;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
    {
        affNorm = !affNorm;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}