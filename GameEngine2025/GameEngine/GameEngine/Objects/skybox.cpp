#include "skybox.h"
#include <cstdio>

// Skybox vertices (cube centered at origin)
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::Skybox() : vao(0), vbo(0), cubemapTexture(0), shader(nullptr) {}

Skybox::~Skybox() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (cubemapTexture) glDeleteTextures(1, &cubemapTexture);
    delete shader;
}

void Skybox::setupMesh() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

GLuint Skybox::loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    for (unsigned int i = 0; i < faces.size(); i++) {
        FILE* file = nullptr;
        if (fopen_s(&file, faces[i].c_str(), "rb") != 0) {
            printf("Failed to load cubemap texture: %s\n", faces[i].c_str());
            continue;
        }

        unsigned char header[54];
        fread(header, 1, 54, file);

        unsigned int width = *(int*)&(header[0x12]);
        unsigned int height = *(int*)&(header[0x16]);
        unsigned int imageSize = width * height * 3;

        unsigned char* data = new unsigned char[imageSize];
        fread(data, 1, imageSize, file);
        fclose(file);

        // After reading data and before BGR->RGB conversion, flip vertically
        unsigned int rowSize = width * 3;
        unsigned char* flippedData = new unsigned char[imageSize];
        for (unsigned int row = 0; row < height; row++) {
            memcpy(flippedData + row * rowSize, 
                   data + (height - 1 - row) * rowSize, 
                   rowSize);
        }
        delete[] data;
        data = flippedData;

        // BMP is BGR, convert to RGB
        for (unsigned int j = 0; j < imageSize; j += 3) {
            unsigned char temp = data[j];
            data[j] = data[j + 2];
            data[j + 2] = temp;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        delete[] data;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

bool Skybox::load(const std::vector<std::string>& faces) {
    setupMesh();
    shader = new Shader("Shaders/skybox_vertex.glsl", "Shaders/skybox_fragment.glsl");
    cubemapTexture = loadCubemap(faces);
    return cubemapTexture != 0;
}



void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) {
    glDepthFunc(GL_LEQUAL);
    shader->use();

    // Remove translation from view matrix
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    glUniformMatrix4fv(glGetUniformLocation(shader->getId(), "view"), 1, GL_FALSE, &skyboxView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader->getId(), "projection"), 1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glUniform1i(glGetUniformLocation(shader->getId(), "skybox"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}