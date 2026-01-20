#include "alien.h"
#include <iostream>

Alien::Alien(Mesh *mesh, glm::vec3 position, float patrolDistance) {
  this->mesh = mesh;
  this->position = position;
  this->startPosition = position;
  this->patrolDistance = patrolDistance;

  this->moveSpeed = 15.0f; // Speed of the alien
  this->verticalVelocity = 0.0f;
  this->direction = 1; // Start moving positive X
  this->dead = false;

  // Approximate size of the alien based on typical model scale
  // We scale the model by 0.1f usually, so we account for that
  this->width = 5.0f;
  this->height = 5.0f;
  this->depth = 5.0f;

  this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);

  std::cout << "Alien Spawned at: " << position.x << ", " << position.y << ", "
            << position.z << " | Mesh Ptr: " << mesh << std::endl;
}

Alien::~Alien() {}

void Alien::update(float deltaTime, CollisionManager &collisionManager) {
  if (dead)
    return;

  // Debug Print every ~60 frames or similar (using simple counter or just spam
  // for now since user is waiting) Actually, let's just print if Y is weird or
  // every frame.
  std::cout << "Alien Pos: " << position.x << ", " << position.y << ", "
            << position.z << " | Vel: " << verticalVelocity << std::endl;

  // Clamp deltaTime to prevent tunneling on lag spikes
  float dt = deltaTime;
  if (dt > 0.1f)
    dt = 0.1f;

  // 1. Apply Gravity
  float gravity = 30.0f; // Strong gravity
  verticalVelocity -= gravity * dt;

  // 2. Propose Movement
  glm::vec3 nextPos = position;
  nextPos.x += moveSpeed * direction * dt;
  nextPos.y += verticalVelocity * dt;

  // 3. Resolve Collisions (Ground & Walls)
  // We use the Alien's height as "eyeHeight" so physics system knows where feet
  // are. Assuming position is "Head/Center", feet are at position.y - height.
  // Actually, usually models have origin at feet. collisionManager expects
  // 'point' to be head, and 'feet' to be point.y - eyeHeight. Let's assume
  // 'position' is at the bottom (feet). So we pass position.y + height as the
  // point? Or if position is center. The code used 'position' as the point to
  // resolve. Let's treat 'position' as the "Head" for physics, and visual
  // offset if needed. Or better: Treat 'position' as feet. Pass 'probe' =
  // position + height.

  glm::vec3 collisionProbe = nextPos;
  collisionProbe.y += 5.0f; // Lift probe up so feet are at nextPos.y

  if (collisionManager.resolvePointAgainstAll(collisionProbe, 5.0f)) {
    // Collision Resolved!
    // collisionProbe contains the new valid position (Head).
    // Update nextPos (Feet)
    nextPos = collisionProbe;
    nextPos.y -= 5.0f;

    const auto &info = collisionManager.getLastCollisionInfo();
    if (info.face == CollisionManager::ContactFace::Top) {
      // Landed on ground
      verticalVelocity = 0.0f;

      // 4. Cliff Detection / Wall Detection logic happens here (only if
      // grounded) Check Wall ahead (using simple bounds check or secondary
      // probe) Check Cliff ahead
      glm::vec3 cliffProbe = nextPos;
      cliffProbe.y += 5.0f;               // Eye height
      cliffProbe.x += (direction * 3.0f); // Look ahead

      // We want to check if there is ground *below* the look-ahead point.
      // The resolve function checks if the point is inside a box.
      // To check for a cliff, we want to see if a point slightly *below* the
      // floor level collides with something. Currently feet are at nextPos.y.
      // We want to check at nextPos.y - 1.0f.
      // So 'Head' for probe would be at (nextPos.y - 1.0f) + 5.0f = nextPos.y
      // + 4.0f

      glm::vec3 floorCheck = cliffProbe;
      floorCheck.y -= 2.0f; // Lower slightly

      // If resolving this point returns FALSE, it means it didn't hit anything
      // -> Empty space -> Cliff! Warning: resolvePointAgainstAll modifies the
      // vector. Use a copy.
      glm::vec3 testPoint = floorCheck;
      if (!collisionManager.resolvePointAgainstAll(testPoint, 5.0f)) {
        // Cliff ahead!
        direction *= -1;
        // std::cout << "Cliff detected! Turning." << std::endl;
      }
    } else if (info.face == CollisionManager::ContactFace::Left ||
               info.face == CollisionManager::ContactFace::Right) {
      // Hit a wall
      direction *= -1;
      // std::cout << "Wall hit! Turning." << std::endl;
    }
  }

  // Patrol Distance Check (Simple fallback)
  if (glm::distance(nextPos.x, startPosition.x) > patrolDistance) {
    direction *= -1;
  }

  position = nextPos;

  // Rotation
  if (direction > 0)
    rotation.y = 90.0f;
  else
    rotation.y = -90.0f;
}

void Alien::draw(Shader &shader, const glm::mat4 &view,
                 const glm::mat4 &projection) {
  if (dead)
    return;

  // Standard Draw Routine
  GLuint MatrixID = glGetUniformLocation(shader.getId(), "MVP");
  GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

  glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), position);
  ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y),
                            glm::vec3(0.0f, 1.0f, 0.0f));
  // Scale up! 0.1f was likely too small. Trying 2.5f.
  ModelMatrix = glm::translate(
      ModelMatrix,
      glm::vec3(0.0f, 1.5f,
                0.0f)); // Visual fix: Float above ground (Adjusted to 1.5)
  ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));

  glm::mat4 MVP = projection * view * ModelMatrix;

  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

  mesh->draw(shader);
}

bool Alien::checkPlayerCollision(glm::vec3 playerPos, glm::vec3 playerVelocity,
                                 bool &outStomped) {
  if (dead)
    return false;

  // AABB Collision Check
  // Alien Box
  float halfW = width / 2.0f;
  float halfH = height; // Origin at bottom?
  float halfD = depth / 2.0f;

  // Simple Box-Box or Point-Box
  // Let's treat player as a point or small box
  bool collisionX =
      playerPos.x > position.x - halfW && playerPos.x < position.x + halfW;
  bool collisionZ =
      playerPos.z > position.z - halfD && playerPos.z < position.z + halfD;
  bool collisionY =
      playerPos.y > position.y &&
      playerPos.y < position.y + halfH + 2.0f; // +2 for player height

  if (collisionX && collisionZ && collisionY) {
    // Collision Detected
    // Determine type: Stomp or Hit?

    // If player is falling and is above the alien center roughly
    // We give a bit of leeway.
    if (playerVelocity.y < 0 && playerPos.y > position.y + (height * 0.5f)) {
      // Stomp!
      outStomped = true;
      dead = true;
      return false; // Player safe
    } else {
      // Hit from side/bottom
      outStomped = false;
      return true; // Player dies
    }
  }

  return false;
}
