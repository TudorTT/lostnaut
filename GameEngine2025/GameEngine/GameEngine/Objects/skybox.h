#pragma once

#include <../GLEW/include/glew.h>
#include <../glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "../Shaders/shader.h"

class Skybox {
public:
    Skybox();
    ~Skybox();

    // Load 6 textures for the cubemap (right, left, top, bottom, front, back)
    bool load(const std::vector<std::string>& faces);

    

    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    GLuint vao, vbo;
    GLuint cubemapTexture;
    Shader* shader;

    void setupMesh();
    GLuint loadCubemap(const std::vector<std::string>& faces);
};