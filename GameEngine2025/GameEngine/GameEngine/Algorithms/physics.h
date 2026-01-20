#pragma once
#include <glm.hpp>

// simple player physics controller
class PlayerPhysics
{
public:
    PlayerPhysics();

    // physics constants
    float gravity;          // gravity acceleration (units/sec^2)
    float jumpForce;        // initial jump velocity
    float walkSpeed;        // normal walking speed
    float runSpeed;         // speed when holding shift

    // current state 
    glm::vec3 velocity;     // current velocity
    bool isGrounded;        // whether player is on the ground
    bool wasGrounded;       // was grounded last frame (for landing detection)

    // update physics for this frame
    glm::vec3 update(float deltaTime);

    // apply movement input (horizontal only, normalized direction)
    void applyMovementInput(const glm::vec3& direction, bool isRunning);

    // try to jump (only works if grounded)
    void jump();

    // called when collision detected with ground
    void onGroundCollision(float groundY);

    // called when collision detected with ceiling
    void onCeilingCollision();

    // reset grounded state (call before collision checks each frame)
    void beginFrame();

    // get current speed multiplier based on running state
    float getCurrentSpeed(bool isRunning) const;

    // reset all physics state
    void reset();

    // print status to console
    void printStatus(const glm::vec3& position) const;
};