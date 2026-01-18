#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <cstdio>

// Interface for any object that can participate in collision detection
class ICollidable
{
public:
    virtual ~ICollidable() = default;

    // Get the world-space axis-aligned bounding box
    virtual void getWorldAABB(glm::vec3& outMin, glm::vec3& outMax) const = 0;

    // Get a name/identifier for this collidable (for debug output)
    virtual const char* getName() const { return "Unknown"; }

    // Optional: check if collision is enabled for this object
    virtual bool isCollisionEnabled() const { return true; }

    // Get the model matrix for OBB collision
    virtual glm::mat4 getModelMatrix() const { return glm::mat4(1.0f); }

    // Get local (unrotated) bounds
    virtual void getLocalBounds(glm::vec3& outMin, glm::vec3& outMax) const {
        getWorldAABB(outMin, outMax);  // Default: same as world AABB
    }

    // Check if this object uses OBB collision
    virtual bool usesOBBCollision() const { return false; }
};


// Centralized collision manager that handles collision detection and resolution
class CollisionManager
{
public:
    enum class ContactFace { None, Top, Bottom, Left, Right, Front, Back };

    // Struct to hold collision info
    struct CollisionInfo
    {
        ICollidable* collidable;
        ContactFace face;
        glm::vec3 resolvedPosition;

        CollisionInfo() : collidable(nullptr), face(ContactFace::None), resolvedPosition(0.0f) {}
    };

    CollisionManager();
    ~CollisionManager();

    // Register a collidable object with the manager
    void addCollidable(ICollidable* collidable);

    // Remove a collidable object from the manager
    void removeCollidable(ICollidable* collidable);

    // Clear all registered collidables
    void clearAll();

    // Resolve a point against ALL registered collidables
    // Returns true if any collision was resolved
    bool resolvePointAgainstAll(glm::vec3& point, float eyeHeight);

    // Resolve a point against a single collidable object
    // Returns true if collision was resolved
    bool resolvePoint(const ICollidable* collidable, glm::vec3& point, float eyeHeight);

    // Get info about the last collision that occurred
    const CollisionInfo& getLastCollisionInfo() const { return lastCollision; }

    // Check if a point is inside a collidable's AABB
    bool isPointInsideAABB(const ICollidable* collidable, const glm::vec3& point) const;

    // Set collision margin (small buffer to prevent clipping)
    void setCollisionMargin(float margin) { collisionMargin = margin; }
    float getCollisionMargin() const { return collisionMargin; }

    // Enable/disable debug output
    void setDebugOutput(bool enabled) { debugOutput = enabled; }

    // Helper to convert ContactFace to string for debugging
    static const char* faceToString(ContactFace face);

private:
    std::vector<ICollidable*> collidables;
    float collisionMargin;
    bool debugOutput;
    CollisionInfo lastCollision;

    // Internal helper to resolve point against an AABB
    bool resolvePointAgainstAABB(const glm::vec3& minW, const glm::vec3& maxW,
        glm::vec3& point, float eyeHeight, ContactFace& outFace);

    // OBB collision check
    bool resolvePointAgainstOBB(const ICollidable* collidable,
        glm::vec3& point, float eyeHeight, ContactFace& outFace);
};