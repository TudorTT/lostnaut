#pragma once
#include <glm.hpp>

// Simple player physics controller
class PlayerPhysics
{
public:
    PlayerPhysics();
    
    // Physics constants
    float gravity;          // Gravity acceleration (units/sec^2)
    float jumpForce;        // Initial jump velocity
    float walkSpeed;        // Normal walking speed
    float runSpeed;         // Speed when holding shift
    
    // Current state (public for easy access)
    glm::vec3 velocity;     // Current velocity
    bool isGrounded;        // Whether player is on the ground
    bool wasGrounded;       // Was grounded last frame (for landing detection)
    
    // Update physics for this frame
    glm::vec3 update(float deltaTime);
    
    // Apply movement input (horizontal only, normalized direction)
    void applyMovementInput(const glm::vec3& direction, bool isRunning);
    
    // Try to jump (only works if grounded)
    void jump();
    
    // Called when collision detected with ground
    void onGroundCollision(float groundY);
    
    // Called when collision detected with ceiling
    void onCeilingCollision();
    
    // Reset grounded state (call before collision checks each frame)
    void beginFrame();
    
    // Get current speed multiplier based on running state
    float getCurrentSpeed(bool isRunning) const;
    
    // Reset all physics state
    void reset();
    
    // Print status to console
    void printStatus(const glm::vec3& position) const;
};
