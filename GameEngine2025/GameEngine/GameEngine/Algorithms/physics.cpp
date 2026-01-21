#include "physics.h"
#include <iostream>
#include <iomanip>

PlayerPhysics::PlayerPhysics()
    : gravity(30.0f)
    , jumpForce(35.0f)
    , walkSpeed(30.0f)
    , runSpeed(60.0f)
    , velocity(0.0f)
    , isGrounded(true)
    , wasGrounded(true)
{
}

glm::vec3 PlayerPhysics::update(float deltaTime)
{
    // Clamp deltaTime to prevent huge jumps on lag spikes
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    
    // Apply gravity if not grounded
    if (!isGrounded)
    {
        velocity.y -= gravity * deltaTime;
        
        // Terminal velocity clamp
        if (velocity.y < -50.0f) velocity.y = -50.0f;
    }
    else
    {
        // Keep a small downward velocity when grounded to maintain ground contact
        if (velocity.y < 0.0f)
            velocity.y = -0.1f;
    }
    
    // Calculate movement delta for this frame
    glm::vec3 delta = velocity * deltaTime;
    
    return delta;
}

void PlayerPhysics::applyMovementInput(const glm::vec3& direction, bool isRunning)
{
    float speed = getCurrentSpeed(isRunning);
    
    // Only apply horizontal movement
    glm::vec3 horizontalDir = direction;
    horizontalDir.y = 0.0f;
    
    if (glm::length(horizontalDir) > 0.0001f)
    {
        horizontalDir = glm::normalize(horizontalDir);
        // Direct velocity set for responsive controls (no sliding)
        velocity.x = horizontalDir.x * speed;
        velocity.z = horizontalDir.z * speed;
    }
    else
    {
        // No input = stop horizontal movement immediately (no sliding)
        velocity.x = 0.0f;
        velocity.z = 0.0f;
    }
}

void PlayerPhysics::jump()
{
    if (isGrounded)
    {
        velocity.y = jumpForce;
        isGrounded = false;  // Immediately set to not grounded
    }
}

void PlayerPhysics::onGroundCollision(float groundY)
{
    // Only ground if moving downward or stationary
    if (velocity.y <= 0.0f)
    {
        velocity.y = 0.0f;
        isGrounded = true;
    }
}

void PlayerPhysics::onCeilingCollision()
{
    if (velocity.y > 0.0f)
    {
        velocity.y = 0.0f;
    }
}

void PlayerPhysics::beginFrame()
{
    wasGrounded = isGrounded;
    // Don't reset isGrounded here - keep it from last frame
    // It will be set false only if we start falling
}

float PlayerPhysics::getCurrentSpeed(bool isRunning) const
{
    return isRunning ? runSpeed : walkSpeed;
}

void PlayerPhysics::reset()
{
    velocity = glm::vec3(0.0f);
    isGrounded = true;
    wasGrounded = true;
}

void PlayerPhysics::printStatus(const glm::vec3& position) const
{
    std::cout << "\r";
    std::cout << "Pos: (" 
              << std::fixed << std::setprecision(2) 
              << position.x << ", " 
              << position.y << ", " 
              << position.z << ") | "
              << "Vel Y: " << velocity.y << " | "
              << "Grounded: " << (isGrounded ? "YES" : "NO ") << "   "
              << std::flush;
}