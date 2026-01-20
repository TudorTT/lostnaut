#pragma once
#include "../Algorithms/collision.h"
#include "../Model Loading/mesh.h"
#include "../Shaders/shader.h"

// GLM Headers
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Alien {
public:
  Alien(Mesh *mesh, glm::vec3 position, float patrolDistance = 100.0f);
  ~Alien();

  void update(float deltaTime, CollisionManager &collisionManager);
  void draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);

  // Collision checks
  // Returns true if player should die
  bool checkPlayerCollision(glm::vec3 playerPos, glm::vec3 playerVelocity,
                            bool &outStomped);

  bool isDead() const { return dead; }
  glm::vec3 getPosition() const { return position; }

private:
  Mesh *mesh; // Shared mesh pointer
  glm::vec3 position;
  glm::vec3 startPosition;

  // Movement
  float moveSpeed;
  float verticalVelocity; // NEW: For gravity
  int direction;          // 1 or -1
  float patrolDistance;

  // State
  bool dead;

  // Size for collision
  float width;
  float height;
  float depth;

  glm::vec3 rotation;
};
