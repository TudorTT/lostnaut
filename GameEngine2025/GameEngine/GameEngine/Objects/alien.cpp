#include "alien.h"
#include <iostream>

Alien::Alien(Mesh* mesh, glm::vec3 position, float patrolDistance) {
    this->mesh = mesh;
    this->position = position;
    this->startPosition = position;
    this->patrolDistance = patrolDistance;

    this->moveSpeed = 15.0f; // Speed of the alien
    this->verticalVelocity = 0.0f;
    this->direction = 1; // Start moving positive X
    this->dead = false;

    // Approximate size of the alien based on model scale (now 5.0f)
    this->width = 10.0f;
    this->height = 10.0f;
    this->depth = 10.0f;

    this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    std::cout << "Alien Spawned at: " << position.x << ", " << position.y << ", "
        << position.z << " | Mesh Ptr: " << mesh << std::endl;
}

Alien::~Alien() {}

void Alien::update(float deltaTime, CollisionManager& collisionManager) {
    if (dead)
        return;

  
   

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


  

    glm::vec3 collisionProbe = nextPos;
    collisionProbe.y += height; // Use actual height instead of hardcoded 5.0f

    if (collisionManager.resolvePointAgainstAll(collisionProbe, height)) {
        // Collision Resolved!
        // collisionProbe contains the new valid position (Head).
        // Update nextPos (Feet)
        nextPos = collisionProbe;
        nextPos.y -= height; // Use height here too

        const auto& info = collisionManager.getLastCollisionInfo();
        if (info.face == CollisionManager::ContactFace::Top) {
            // Landed on ground
            verticalVelocity = 0.0f;

            // 4. Cliff Detection / Wall Detection logic happens here (only if
            // grounded) Check Wall ahead (using simple bounds check or s   econdary
            // probe) Check Cliff ahead
            glm::vec3 cliffProbe = nextPos;
            cliffProbe.y += 5.0f;               // Eye height
            cliffProbe.x += (direction * 3.0f); // Look ahead


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
        }
        else if (info.face == CollisionManager::ContactFace::Left ||
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

void Alien::draw(Shader& shader, const glm::mat4& view,
    const glm::mat4& projection) {
    if (dead)
        return;

    // Standard Draw Routine
    GLuint MatrixID = glGetUniformLocation(shader.getId(), "MVP");
    GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

    glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), position);
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y),
        glm::vec3(0.0f, 1.0f, 0.0f));
    // Visual fix: No extra offset needed - position is already at feet level
    // The model origin should be at feet, scale handles the rest
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));

    glm::mat4 MVP = projection * view * ModelMatrix;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

    mesh->draw(shader);
}

bool Alien::checkPlayerCollision(glm::vec3 playerPos, glm::vec3 playerVelocity,
    bool& outStomped) {
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
        }
        else {
            // Hit from side/bottom
            outStomped = false;
            return true; // Player dies
        }
    }

    return false;
}
