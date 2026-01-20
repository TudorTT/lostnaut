#include "Algorithms\collision.h"
#include "Algorithms\physics.h"
#include "Camera\camera.h"
#include "Graphics\window.h"
#include "Model Loading\mesh.h"
#include "Model Loading\meshLoaderObj.h"
#include "Model Loading\texture.h"
#include "Objects\alien.h" // NEW
#include "Objects\platform.h"
#include "Shaders\shader.h"

void processKeyboardInput();
void resetPlayer();

// 0 = None, 1 = Plant, 2 = Coin, 3 = Treat
int heldItemID = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isNearItem = false;

Window window("Game Engine", 800, 800);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 300.0f, -300.0f);

// Item Positions
glm::vec3 plantPos = glm::vec3(-93.14f, 62.0f, -193.34f);
glm::vec3 coinPos = glm::vec3(6.33f, 3.5f, -406.48f);
glm::vec3 treatPos = glm::vec3(105.08f, 50.74f, -199.93f);
// Spaceship Position
glm::vec3 spaceshipPos = glm::vec3(-30.0f, 10.0f, -30.0f);

// NEW: Dropped treats from aliens
struct DroppedTreat {
  glm::vec3 pos;
  bool collected;
};
std::vector<DroppedTreat> droppedTreats;

CollisionManager collisionManager;
PlayerPhysics playerPhysics;

const glm::vec3 SPAWN_POSITION = glm::vec3(0.0f, 5.0f, 0.0f);

Platform *g_platform = nullptr;
Platform *platform1 = nullptr;
Platform *platform2 = nullptr;
Platform *platform3 = nullptr;
Platform *platform4 = nullptr;
Platform *platform5 = nullptr;
Platform *platform6 = nullptr;
Platform *plantPlatform = nullptr;
Platform *fence = nullptr;
Platform *mountain1 = nullptr;
Platform *mountain2 = nullptr;
Platform *mountain3 = nullptr;
Platform *mountain4 = nullptr;
Platform *mountain5 = nullptr;
Platform *mountain6 = nullptr;
Platform *spike1 = nullptr;
Platform *spike2 = nullptr;
Platform *spike3 = nullptr;
Platform *spaceshipPlatform = nullptr;
Platform *dogPlatform = nullptr; // NEW

std::vector<Alien *> aliens; // NEW

const float PLAYER_EYE_HEIGHT = 1.0f;

int main() {
  glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

  Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
  Shader sunShader("Shaders/sun_vertex_shader.glsl",
                   "Shaders/sun_fragment_shader.glsl");

  // Textures
  GLuint tex = loadBMP("Resources/Textures/wood.bmp");
  GLuint tex2 = loadBMP("Resources/Textures/fence.bmp");
  GLuint tex3 = loadBMP("Resources/Textures/mars.bmp");
  GLuint tex4 = loadBMP("Resources/Textures/platform.bmp");
  GLuint tex5 = loadBMP("Resources/Textures/rockwall.bmp");
  GLuint tex6 = loadBMP("Resources/Textures/spikes.bmp");

  GLuint texPlant = loadBMP("Resources/Textures/Plant_BaseColor.bmp");
  GLuint texCoin = loadBMP("Resources/Textures/GoldColor.bmp");
  GLuint texTreat = loadBMP("Resources/Textures/TreatTex.bmp");
  GLuint texPressE = loadBMP("Resources/Textures/PressE.bmp");
  GLuint texSpaceship = loadBMP("Resources/Textures/spaceship_texture.bmp");
  GLuint texAlien = loadBMP("Resources/Textures/alien_texture.bmp");
  GLuint texDog = loadBMP("Resources/Textures/dog.bmp"); // NEW

  glEnable(GL_DEPTH_TEST);
  window.lockCursor(true);

  // Texture Vectors
  std::vector<Texture> textures;
  textures.push_back({tex, "texture_diffuse"});
  std::vector<Texture> textures2;
  textures2.push_back({tex2, "texture_diffuse"});
  std::vector<Texture> textures3;
  textures3.push_back({tex3, "texture_diffuse"});
  std::vector<Texture> textures4;
  textures4.push_back({tex4, "texture_diffuse"});
  std::vector<Texture> textures5;
  textures5.push_back({tex5, "texture_diffuse"});
  std::vector<Texture> texturesSpike;
  texturesSpike.push_back({tex6, "texture_diffuse"});

  std::vector<Texture> texturesPlant;
  texturesPlant.push_back({texPlant, "texture_diffuse"});
  std::vector<Texture> texturesCoin;
  texturesCoin.push_back({texCoin, "texture_diffuse"});
  std::vector<Texture> texturesTreat;
  texturesTreat.push_back({texTreat, "texture_diffuse"});
  std::vector<Texture> texturesPressE;
  texturesPressE.push_back({texPressE, "texture_diffuse"});
  std::vector<Texture> texturesSpaceship;
  texturesSpaceship.push_back({texSpaceship, "texture_diffuse"});
  std::vector<Texture> texturesAlien;
  texturesAlien.push_back({texAlien, "texture_diffuse"});
  std::vector<Texture> texturesDog;
  texturesDog.push_back({texDog, "texture_diffuse"}); // NEW

  MeshLoaderObj loader;
  Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
  Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
  Mesh plane = loader.loadObj("Resources/Models/plane_mars.obj", textures3);
  Mesh fenceMesh = loader.loadObj("Resources/Models/fence.obj", textures2);
  Mesh platformMesh =
      loader.loadObj("Resources/Models/floatingplatform.obj", textures4);
  Mesh mountainMesh =
      loader.loadObj("Resources/Models/Rockwall.obj", textures5);
  Mesh spikeMesh = loader.loadObj("Resources/Models/spikes.obj", texturesSpike);

  Mesh plantModel = loader.loadObj("Resources/Models/Plant.obj", texturesPlant);
  Mesh marioCoin =
      loader.loadObj("Resources/Models/MarioCoin.obj", texturesCoin);
  Mesh dogTreat =
      loader.loadObj("Resources/Models/DogTreat.obj", texturesTreat);
  Mesh spaceshipModel =
      loader.loadObj("Resources/Models/spaceship.obj", texturesSpaceship);
  Mesh alienModel =
      loader.loadObj("Resources/Models/alien_object.obj", texturesAlien);
  Mesh dogModel =
      loader.loadObj("Resources/Models/dog.obj", texturesDog); // NEW

  // Platforms Init
  g_platform = new Platform(plane, "Ground");
  g_platform->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
  g_platform->setScale(glm::vec3(300.0f, 2.0f, 1000.0f));

  // ... (Lines 150-241 same, just ensuring we insert before collision manager
  // if needed) Actually, I can just append the dog init code here or where
  // other platforms are. I'll skip forward to the end of platform init if I
  // can, but I only have this chunk. Let me replace the start of initialization
  // and I'll add the Dog instantiation later in the file with a separate edit
  // or include it if I can see where spaceshipPlatform is. Wait, I can't see
  // spaceshipPlatform init in this chunk. I will just add the Loader/Texture
  // parts here. I will do a second edit for the instantiation.

  platform1 = new Platform(platformMesh, "P1");
  platform1->setPosition(glm::vec3(50.0f, 10.0f, -38.0f));
  platform1->setScale(glm::vec3(10.0f, 10.0f, 10.0f));
  platform2 = new Platform(platformMesh, "P2");
  platform2->setPosition(glm::vec3(0.0f, 30.0f, -60.0f));
  platform2->setScale(glm::vec3(15.0f, 15.0f, 15.0f));
  platform3 = new Platform(platformMesh, "P3");
  platform3->setPosition(glm::vec3(40.0f, 40.0f, -110.0f));
  platform3->setScale(glm::vec3(20.0f, 10.0f, 25.0f));
  platform4 = new Platform(platformMesh, "P4");
  platform4->setPosition(glm::vec3(9.0f, 50.0f, -150.0f));
  platform4->setScale(glm::vec3(10.0f, 10.0f, 10.0f));
  platform5 = new Platform(platformMesh, "P5");
  platform5->setPosition(glm::vec3(110.0f, 30.0f, -260.0f));
  platform5->setScale(glm::vec3(13.0f, 10.0f, 13.0f));
  platform6 = new Platform(platformMesh, "P6");
  platform6->setPosition(glm::vec3(76.0f, 20.0f, -242.0f));
  platform6->setScale(glm::vec3(13.0f, 10.0f, 13.0f));

  plantPlatform = new Platform(platformMesh, "PlantPlatform");
  plantPlatform->setPosition(glm::vec3(-93.14f, 60.0f, -193.34f));
  plantPlatform->setScale(glm::vec3(10.0f, 5.0f, 10.0f));

  fence = new Platform(fenceMesh, "fence");
  fence->setPosition(glm::vec3(60.0f, 0.0f, -200.0f));
  fence->setScale(glm::vec3(175.0f, 25.0f, 20.0f));

  mountain1 = new Platform(mountainMesh, "M1");
  mountain1->setPosition(glm::vec3(100.0f, 0.0f, 100.0f));
  mountain1->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
  mountain1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
  mountain1->setUseOBBCollision(true);
  mountain2 = new Platform(mountainMesh, "M2");
  mountain2->setPosition(glm::vec3(-100.0f, 0.0f, 100.0f));
  mountain2->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
  mountain2->setRotation(glm::vec3(0.0f, -180.0f, 0.0f));
  mountain2->setUseOBBCollision(true);
  mountain4 = new Platform(mountainMesh, "M4");
  mountain4->setPosition(glm::vec3(223.0f, 0.0f, -275.0f));
  mountain4->setScale(glm::vec3(300.0f, 270.0f, 300.0f));
  mountain4->setRotation(glm::vec3(0.0f, 80.0f, 0.0f));
  mountain4->setUseOBBCollision(true);
  mountain5 = new Platform(mountainMesh, "M5");
  mountain5->setPosition(glm::vec3(-223.0f, 0.0f, -275.0f));
  mountain5->setScale(glm::vec3(300.0f, 350.0f, 300.0f));
  mountain5->setRotation(glm::vec3(0.0f, 80.0f, 0.0f));
  mountain5->setUseOBBCollision(true);
  mountain3 = new Platform(mountainMesh, "M3");
  mountain3->setPosition(glm::vec3(0.0f, 0.0f, 150.0f));
  mountain3->setScale(glm::vec3(130.0f, 130.0f, 130.0f));
  mountain6 = new Platform(mountainMesh, "M6");
  mountain6->setPosition(glm::vec3(50.0f, 0.0f, -470.0f));
  mountain6->setScale(glm::vec3(200.0f, 400.0f, 200.0f));

  spike1 = new Platform(spikeMesh, "S1");
  spike1->setPosition(glm::vec3(-100.0f, 1.0f, -200.0f));
  spike1->setScale(glm::vec3(50.0f, 5.0f, 25.0f));
  spike1->setIsHazard(true);
  spike2 = new Platform(spikeMesh, "S2");
  spike2->setPosition(glm::vec3(37.0f, 42.0f, -115.0f));
  spike2->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
  spike2->setIsHazard(true);
  spike3 = new Platform(spikeMesh, "S3");
  spike3->setPosition(glm::vec3(9.0f, 49.0f, -196.0f));
  spike3->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
  spike3->setIsHazard(true);

  // Spaceship as a Platform for Collision
  spaceshipPlatform = new Platform(spaceshipModel, "Spaceship");
  spaceshipPlatform->setPosition(spaceshipPos);
  spaceshipPlatform->setScale(glm::vec3(10.0f, 10.0f, 10.0f));
  spaceshipPlatform->setUseOBBCollision(true);

  // Dog Object (Static)
  dogPlatform = new Platform(dogModel, "Dog");
  glm::vec3 dogPos = spaceshipPos;
  dogPos.x += 15.0f; // Next to ship
  dogPos.y = 2.5f;   // Lower to ground (Fix floating)
  dogPlatform->setPosition(dogPos);
  dogPlatform->setScale(glm::vec3(3.0f, 3.0f, 3.0f)); // Adjust scale as needed
  dogPlatform->setUseOBBCollision(true);

  collisionManager.addCollidable(g_platform);
  collisionManager.addCollidable(platform1);
  collisionManager.addCollidable(platform2);
  collisionManager.addCollidable(platform3);
  collisionManager.addCollidable(platform4);
  collisionManager.addCollidable(platform5);
  collisionManager.addCollidable(platform6);
  collisionManager.addCollidable(plantPlatform);
  collisionManager.addCollidable(fence);
  collisionManager.addCollidable(mountain1);
  collisionManager.addCollidable(mountain2);
  collisionManager.addCollidable(mountain3);
  collisionManager.addCollidable(mountain4);
  collisionManager.addCollidable(mountain5);
  collisionManager.addCollidable(mountain6);
  collisionManager.addCollidable(spike1);
  collisionManager.addCollidable(spike2);
  collisionManager.addCollidable(spike3);
  collisionManager.addCollidable(spike3);
  collisionManager.addCollidable(spaceshipPlatform);
  collisionManager.addCollidable(dogPlatform); // NEW

  collisionManager.setCollisionMargin(0);
  collisionManager.setDebugOutput(false);

  // Initialize Aliens
  // Spawning them closer to player start (0,0,0) for visibility testing
  // Alien 1: In front and slightly to the right
  // Alien 1: In front and slightly to the right, high up to settle
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(15.0f, 15.0f, 10.0f), 40.0f));
  // Alien 2: Near the spaceship (Moved slightly to avoid getting stuck)
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(-40.0f, 15.0f, -40.0f), 40.0f));
  // Alien 3: On Platform 1
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(50.0f, 25.0f, -38.0f), 30.0f));
  // Alien 4: On Platform 2
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(0.0f, 45.0f, -60.0f), 30.0f));
  // Alien 5: On Mountain 1
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(100.0f, 20.0f, 100.0f), 60.0f));
  // Alien 6: Random Ground
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(-50.0f, 15.0f, 50.0f), 50.0f));

  // Aliens guarding the Fuel/Treat (treatPos approx 105, 50, -200)
  // Guard 1
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(100.0f, 55.0f, -195.0f), 20.0f));
  // Guard 2
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(110.0f, 55.0f, -195.0f), 20.0f));
  // Guard 3
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(100.0f, 55.0f, -205.0f), 20.0f));
  // Guard 4
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(110.0f, 55.0f, -205.0f), 20.0f));

  // EXTRA Guards (Tight circle around treat)
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(105.0f, 55.0f, -190.0f), 10.0f));
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(105.0f, 55.0f, -210.0f), 10.0f));
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(95.0f, 55.0f, -200.0f), 10.0f));
  aliens.push_back(
      new Alien(&alienModel, glm::vec3(115.0f, 55.0f, -200.0f), 10.0f));

  // HUD Setup
  std::vector<Vertex> hudVerts;
  std::vector<int> hudIndices = {0, 1, 2, 0, 2, 3};
  Vertex v;
  v.textureCoords = glm::vec2(0, 0);
  v.pos = glm::vec3(-0.02f, 0.02f, 0.0f);
  hudVerts.push_back(v);
  v.textureCoords = glm::vec2(1, 0);
  v.pos = glm::vec3(0.02f, 0.02f, 0.0f);
  hudVerts.push_back(v);
  v.textureCoords = glm::vec2(1, 1);
  v.pos = glm::vec3(0.02f, -0.02f, 0.0f);
  hudVerts.push_back(v);
  v.textureCoords = glm::vec2(0, 1);
  v.pos = glm::vec3(-0.02f, -0.02f, 0.0f);
  hudVerts.push_back(v);
  Mesh hudSquare(hudVerts, hudIndices, texturesPressE);

  while (!window.isPressed(GLFW_KEY_ESCAPE) &&
         glfwWindowShouldClose(window.getWindow()) == 0) {
    window.clear();
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processKeyboardInput();

    sunShader.use();
    glm::mat4 ProjectionMatrix = glm::perspective(
        glm::radians(70.0f), window.getWidth() * 1.0f / window.getHeight(),
        0.1f, 10000.0f);
    glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(),
                                       camera.getCameraPosition() +
                                           camera.getCameraViewDirection(),
                                       camera.getCameraUp());

    GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");
    glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), lightPos);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    sun.draw(sunShader);

    shader.use();
    GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
    GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");
    glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"),
                lightColor.x, lightColor.y, lightColor.z);
    glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x,
                lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"),
                camera.getCameraPosition().x, camera.getCameraPosition().y,
                camera.getCameraPosition().z);

    // Box
    ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    box.draw(shader);

    // Platforms
    g_platform->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform1)
      platform1->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform2)
      platform2->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform3)
      platform3->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform4)
      platform4->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform5)
      platform5->draw(shader, ViewMatrix, ProjectionMatrix);
    if (platform6)
      platform6->draw(shader, ViewMatrix, ProjectionMatrix);
    if (plantPlatform)
      plantPlatform->draw(shader, ViewMatrix, ProjectionMatrix);
    if (fence)
      fence->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain1)
      mountain1->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain2)
      mountain2->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain3)
      mountain3->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain4)
      mountain4->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain5)
      mountain5->draw(shader, ViewMatrix, ProjectionMatrix);
    if (mountain6)
      mountain6->draw(shader, ViewMatrix, ProjectionMatrix);
    if (spike1)
      spike1->draw(shader, ViewMatrix, ProjectionMatrix);
    if (spike2)
      spike2->draw(shader, ViewMatrix, ProjectionMatrix);
    if (spike3)
      spike3->draw(shader, ViewMatrix, ProjectionMatrix);
    if (spaceshipPlatform)
      spaceshipPlatform->draw(shader, ViewMatrix, ProjectionMatrix);
    if (dogPlatform)
      dogPlatform->draw(shader, ViewMatrix, ProjectionMatrix);

    // Aliens
    for (auto &alien : aliens) {
      alien->update(deltaTime, collisionManager);
      alien->draw(shader, ViewMatrix, ProjectionMatrix);
      bool stomped = false;
      if (alien->checkPlayerCollision(camera.getCameraPosition(),
                                      playerPhysics.velocity, stomped)) {
        // Player died
        resetPlayer();
      } else if (stomped) {
        // Create dropped treat
        DroppedTreat t;
        t.pos = alien->getPosition();
        t.pos.y += 2.0f;
        t.collected = false;
        droppedTreats.push_back(t);

        // Bounce player
        playerPhysics.velocity.y = 10.0f; // Bounce
      }
    }

    // --- Items ---
    if (heldItemID == 1) {
      ModelMatrix = glm::translate(
          glm::mat4(1.0), camera.getCameraPosition() +
                              (camera.getCameraViewDirection() * 1.5f) -
                              (camera.getCameraUp() * 0.5f) +
                              (glm::cross(camera.getCameraViewDirection(),
                                          camera.getCameraUp()) *
                               0.8f));
    } else {
      float plantWobble = sin(currentFrame * 2.0f) * 0.5f;
      ModelMatrix = glm::translate(glm::mat4(1.0),
                                   plantPos + glm::vec3(0, plantWobble, 0));
    }
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 100.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    plantModel.draw(shader);

    if (heldItemID == 2) {
      ModelMatrix = glm::translate(
          glm::mat4(1.0), camera.getCameraPosition() +
                              (camera.getCameraViewDirection() * 1.5f) -
                              (camera.getCameraUp() * 0.5f) +
                              (glm::cross(camera.getCameraViewDirection(),
                                          camera.getCameraUp()) *
                               0.8f));
    } else {
      float coinWobble = sin(currentFrame * 3.0f) * 0.5f;
      ModelMatrix =
          glm::translate(glm::mat4(1.0), coinPos + glm::vec3(0, coinWobble, 0));
    }
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 150.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    marioCoin.draw(shader);

    if (heldItemID == 3) {
      ModelMatrix = glm::translate(
          glm::mat4(1.0), camera.getCameraPosition() +
                              (camera.getCameraViewDirection() * 1.5f) -
                              (camera.getCameraUp() * 0.5f) +
                              (glm::cross(camera.getCameraViewDirection(),
                                          camera.getCameraUp()) *
                               0.8f));
    } else {
      float treatWobble = sin(currentFrame * 1.5f) * 0.5f;
      ModelMatrix = glm::translate(glm::mat4(1.0),
                                   treatPos + glm::vec3(0, treatWobble, 0));
    }
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 40.0f),
                              glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    dogTreat.draw(shader);

    // NEW: Draw Dropped Treats
    for (auto &t : droppedTreats) {
      if (t.collected)
        continue;

      float treatWobble = sin(currentFrame * 1.5f) * 0.5f;
      ModelMatrix =
          glm::translate(glm::mat4(1.0), t.pos + glm::vec3(0, treatWobble, 0));
      ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 40.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
      ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
      MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
      glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
      glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
      dogTreat.draw(shader);
    }

    // HUD
    bool nearAnyTreat = false;
    if (!droppedTreats.empty()) {
      for (auto &t : droppedTreats) {
        if (!t.collected &&
            glm::distance(camera.getCameraPosition(), t.pos) < 5.0f) {
          nearAnyTreat = true;
          break;
        }
      }
    }

    if ((isNearItem || nearAnyTreat) && heldItemID == 0) {
      glDisable(GL_DEPTH_TEST);
      sunShader.use();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texPressE);
      glUniform1i(glGetUniformLocation(sunShader.getId(), "texture_diffuse1"),
                  0);
      glm::mat4 hudProj = glm::mat4(1.0);
      glm::mat4 hudView = glm::mat4(1.0);
      glm::mat4 hudModel = glm::mat4(1.0);
      hudModel = glm::translate(hudModel, glm::vec3(0.0f, -0.8f, 0.0f));
      hudModel = glm::scale(hudModel, glm::vec3(15.0f, 4.0f, 1.0f));
      GLuint hudMatrixID = glGetUniformLocation(sunShader.getId(), "MVP");
      glm::mat4 hudMVP = hudProj * hudView * hudModel;
      glUniformMatrix4fv(hudMatrixID, 1, GL_FALSE, &hudMVP[0][0]);
      hudSquare.draw(sunShader);
      glEnable(GL_DEPTH_TEST);
    }
    window.update();
  }

  collisionManager.clearAll();
  delete g_platform;
  delete platform1;
  delete platform2;
  delete platform3;
  delete platform4;
  delete platform5;
  delete platform6;
  delete plantPlatform;
  delete fence;
  delete mountain1;
  delete mountain2;
  delete mountain3;
  delete mountain4;
  delete mountain5;
  delete mountain6;
  delete spike1;
  delete spike2;
  delete spike3;
  delete spaceshipPlatform;
  delete dogPlatform; // NEW
  for (auto a : aliens)
    delete a;
}

void processKeyboardInput() {
  float distPlant = glm::distance(camera.getCameraPosition(), plantPos);
  float distCoin = glm::distance(camera.getCameraPosition(), coinPos);
  float distTreat = glm::distance(camera.getCameraPosition(), treatPos);
  float distToShip = glm::distance(camera.getCameraPosition(), spaceshipPos);
  // Dog Interaction
  float distToDog = 1000.0f;
  if (dogPlatform)
    distToDog =
        glm::distance(camera.getCameraPosition(), dogPlatform->getPosition());

  float interactionDist = 5.0f;
  float shipDepositDist = 12.0f;

  bool nearStaticItem =
      (distPlant < interactionDist || distCoin < interactionDist ||
       distTreat < interactionDist);
  isNearItem = nearStaticItem;

  static bool eWasPressed = false;
  if (window.isPressed(GLFW_KEY_E) && !eWasPressed) {
    if (heldItemID == 0) {
      if (distPlant < interactionDist)
        heldItemID = 1;
      else if (distCoin < interactionDist)
        heldItemID = 2;
      else if (distTreat < interactionDist)
        heldItemID = 3;
      else {
        // Check dropped treats
        for (auto &t : droppedTreats) {
          if (!t.collected && glm::distance(camera.getCameraPosition(), t.pos) <
                                  interactionDist) {
            t.collected = true;
            heldItemID = 3;
            break;
          }
        }
      }
    } else {
      if ((heldItemID == 1 || heldItemID == 2) &&
          distToShip < shipDepositDist) {
        if (heldItemID == 1)
          plantPos = glm::vec3(0, -500, 0);
        if (heldItemID == 2)
          coinPos = glm::vec3(0, -500, 0);

        heldItemID = 0;
        printf("Item deposited in spaceship!\n");
      } else if (heldItemID == 3 && distToDog < shipDepositDist) {
        // Give treat to Dog
        treatPos = glm::vec3(0, -500, 0); // Hide static treat
        heldItemID = 0;
        printf("Treat given to Dog! Good boy!\n");
      } else {
        glm::vec3 dropPos = camera.getCameraPosition() +
                            (camera.getCameraViewDirection() * 3.0f);
        if (camera.getCameraPosition().y < 10.0f)
          dropPos.y = 2.5f;
        else
          dropPos.y = camera.getCameraPosition().y - 0.5f;

        if (heldItemID == 1)
          plantPos = dropPos;
        if (heldItemID == 2)
          coinPos = dropPos;
        if (heldItemID == 3)
          treatPos = dropPos;
        heldItemID = 0;
      }
    }
  }
  eWasPressed = window.isPressed(GLFW_KEY_E);

  static bool tabWasPressed = false;
  bool tabPressed = window.isPressed(GLFW_KEY_TAB);
  if (tabPressed && !tabWasPressed)
    window.lockCursor(!window.isCursorLocked());
  tabWasPressed = tabPressed;

  if (window.isCursorLocked()) {
    double deltaX, deltaY;
    window.getMouseDelta(deltaX, deltaY);
    camera.mouseLook((float)deltaX, (float)deltaY, 0.1f);
    window.resetMouseDelta();
  }

  static bool pWasPressed = false;
  if (window.isPressed(GLFW_KEY_P) && !pWasPressed)
    resetPlayer();
  pWasPressed = window.isPressed(GLFW_KEY_P);

  glm::vec3 currentPos = camera.getCameraPosition();
  glm::vec3 moveDirection(0.0f);
  glm::vec3 forward = camera.getCameraViewDirection();
  forward.y = 0.0f;
  if (glm::length(forward) > 0.0001f)
    forward = glm::normalize(forward);
  glm::vec3 right =
      glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

  if (window.isPressed(GLFW_KEY_W))
    moveDirection += forward;
  if (window.isPressed(GLFW_KEY_S))
    moveDirection -= forward;
  if (window.isPressed(GLFW_KEY_A))
    moveDirection -= right;
  if (window.isPressed(GLFW_KEY_D))
    moveDirection += right;

  bool isRunning = window.isPressed(GLFW_KEY_LEFT_SHIFT) ||
                   window.isPressed(GLFW_KEY_RIGHT_SHIFT);
  playerPhysics.applyMovementInput(moveDirection, isRunning);

  static bool spaceWasPressed = false;
  if (window.isPressed(GLFW_KEY_SPACE) && !spaceWasPressed &&
      playerPhysics.isGrounded)
    playerPhysics.jump();
  spaceWasPressed = window.isPressed(GLFW_KEY_SPACE);

  glm::vec3 delta = playerPhysics.update(deltaTime);
  glm::vec3 proposed = currentPos + delta;
  bool collided =
      collisionManager.resolvePointAgainstAll(proposed, PLAYER_EYE_HEIGHT);
  bool groundContactThisFrame = false;

  if (collided) {
    const auto &info = collisionManager.getLastCollisionInfo();
    Platform *hitPlatform = dynamic_cast<Platform *>(info.collidable);
    if (hitPlatform && hitPlatform->getIsHazard()) {
      resetPlayer();
      return;
    }
    if (info.face == CollisionManager::ContactFace::Top) {
      playerPhysics.onGroundCollision(proposed.y);
      groundContactThisFrame = true;
    } else if (info.face == CollisionManager::ContactFace::Bottom)
      playerPhysics.onCeilingCollision();
    else {
      glm::vec3 probePos = proposed;
      probePos.y -= 0.2f;
      if (collisionManager.resolvePointAgainstAll(probePos,
                                                  PLAYER_EYE_HEIGHT)) {
        if (collisionManager.getLastCollisionInfo().face ==
            CollisionManager::ContactFace::Top) {
          groundContactThisFrame = true;
          playerPhysics.isGrounded = true;
        }
      }
    }
  } else if (playerPhysics.isGrounded && playerPhysics.velocity.y <= 0.1f) {
    glm::vec3 probePos = proposed;
    probePos.y -= 0.3f;
    if (collisionManager.resolvePointAgainstAll(probePos, PLAYER_EYE_HEIGHT)) {
      if (collisionManager.getLastCollisionInfo().face ==
          CollisionManager::ContactFace::Top) {
        groundContactThisFrame = true;
        playerPhysics.isGrounded = true;
      }
    }
  }

  if (!groundContactThisFrame)
    playerPhysics.isGrounded = false;
  camera.setCameraPosition(proposed);
}

void resetPlayer() {
  camera.setCameraPosition(SPAWN_POSITION);
  playerPhysics.reset();
}