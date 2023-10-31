#include<iostream>
#include<vector>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "shaderClass.h";

int width = 1000;
int height = 1000;

bool firstmouse = true;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = width / 2, lastY = height / 2;
float zoom = 45.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstmouse) {
        lastX = xpos;
        lastY = ypos;
        firstmouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sens = 0.1f;
    xoffset *= sens;
    yoffset *= sens;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 cameraDirection;
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(cameraDirection);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom -= float(yoffset);
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Terrain",NULL,NULL);
    if (window == NULL) {
        std::cout << "nuh uh";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 1000, 1000);

    Shader shaderprog("default.vert","default.frag");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    int widthi, heighti, nchan,len = 0;

    unsigned char *data = stbi_load("height_map.jpg", &widthi, &heighti, &nchan,0);

    if (data)
        cout << widthi * heighti * nchan;
    else
        cout << "nuh uh";
    
    //to store the vertices data
    std::vector<float> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    unsigned bytePerPixel = nchan;

    for (int i = 0; i < heighti; i++) {
        for (int j = 0; j < widthi; j++) {
            unsigned char* offset = data + ((j + widthi * i) * bytePerPixel);
            unsigned char y = offset[0]; //the heightmap value from the image
        
            vertices.push_back(-heighti / 2.0f + i);
            vertices.push_back(y * yScale - yShift);
            vertices.push_back(-widthi / 2.0f + j);
        }
    }

    stbi_image_free(data);
    cout << vertices.size();

    //to store the indices data
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < heighti-1; i++) {
        for (unsigned int j = 0; j < widthi; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                indices.push_back(j + widthi * (i + k));
            }
        }
    }
    
    unsigned int STRIP = heighti - 1;
    unsigned int VERTS_PER_STRIP = widthi * 2;

    unsigned int VAO,VBO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    float rotation = 0.0f;
    double prevTime = glfwGetTime();
    float trans = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);

        if(glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderprog.Activate();
        double crntTime = glfwGetTime();

        if (crntTime - prevTime >= 1 / 6) {
            rotation += 0.1f;
            prevTime = crntTime;
            trans = sin(rotation / 10);
        }

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);


        float currentframe = glfwGetTime();
        deltaTime = currentframe - lastFrame;
        lastFrame = currentframe;

        const float cameraSpeed = 10.0f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos += cameraSpeed * cameraFront;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos -= cameraSpeed * cameraFront;
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }

        glm::mat4 model = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        proj = glm::perspective(glm::radians(zoom), (float)width / height, 0.1f, 1000.0f);

        int modelLoc = glGetUniformLocation(shaderprog.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        int viewLoc = glGetUniformLocation(shaderprog.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projLoc = glGetUniformLocation(shaderprog.ID, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        glBindVertexArray(VAO);

        for (unsigned i = 0; i < STRIP; ++i) {
            glDrawElements(GL_TRIANGLE_STRIP, VERTS_PER_STRIP, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * (VERTS_PER_STRIP) * i));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    shaderprog.Delete();

    glfwTerminate();
	return 0;
}