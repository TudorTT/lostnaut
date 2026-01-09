#include "collision.h"
#include <algorithm>
#include <cstdio>

CollisionManager::CollisionManager()
    : collisionMargin(0.1f)
    , debugOutput(false)
{
}

CollisionManager::~CollisionManager()
{
    // Don't delete collidables - we don't own them
    collidables.clear();
}

void CollisionManager::addCollidable(ICollidable* collidable)
{
    if (collidable)
    {
        // Avoid duplicates
        auto it = std::find(collidables.begin(), collidables.end(), collidable);
        if (it == collidables.end())
        {
            collidables.push_back(collidable);
        }
    }
}

void CollisionManager::removeCollidable(ICollidable* collidable)
{
    auto it = std::find(collidables.begin(), collidables.end(), collidable);
    if (it != collidables.end())
    {
        collidables.erase(it);
    }
}

void CollisionManager::clearAll()
{
    collidables.clear();
}

const char* CollisionManager::faceToString(ContactFace face)
{
    switch (face)
    {
    case ContactFace::Top:    return "TOP";
    case ContactFace::Bottom: return "BOTTOM";
    case ContactFace::Left:   return "LEFT";
    case ContactFace::Right:  return "RIGHT";
    case ContactFace::Front:  return "FRONT";
    case ContactFace::Back:   return "BACK";
    default:                  return "NONE";
    }
}

bool CollisionManager::resolvePointAgainstAll(glm::vec3& point, float eyeHeight)
{
    bool anyResolved = false;
    lastCollision = CollisionInfo(); // Reset last collision info

    for (ICollidable* collidable : collidables)
    {
        if (collidable && collidable->isCollisionEnabled())
        {
            ContactFace face;
            bool collided = false;

            // Use OBB collision if the object requests it
            if (collidable->usesOBBCollision())
            {
                collided = resolvePointAgainstOBB(collidable, point, eyeHeight, face);
            }
            else
            {
                // Use standard AABB collision
                glm::vec3 minW, maxW;
                collidable->getWorldAABB(minW, maxW);
                collided = resolvePointAgainstAABB(minW, maxW, point, eyeHeight, face);
            }

            if (collided)
            {
                anyResolved = true;
                lastCollision.collidable = collidable;
                lastCollision.face = face;
                lastCollision.resolvedPosition = point;

                // Print collision info if debug is enabled
                if (debugOutput)
                {
                    printf("=== Collision with '%s' on %s face ===\n",
                        collidable->getName(),
                        faceToString(face));
                    printf("  Player pos: (%.2f, %.2f, %.2f), feet Y: %.2f\n",
                        point.x, point.y, point.z, point.y - eyeHeight);

                    glm::vec3 minW, maxW;
                    if (collidable->usesOBBCollision())
                    {
                        glm::vec3 localMin, localMax;
                        collidable->getLocalBounds(localMin, localMax);
                        printf("  OBJ Local bounds min: (%.2f, %.2f, %.2f)\n",
                            localMin.x, localMin.y, localMin.z);
                        printf("  OBJ Local bounds max: (%.2f, %.2f, %.2f)\n",
                            localMax.x, localMax.y, localMax.z);
                    }

                    collidable->getWorldAABB(minW, maxW);
                    printf("  OBJ World AABB min: (%.2f, %.2f, %.2f)\n",
                        minW.x, minW.y, minW.z);
                    printf("  OBJ World AABB max: (%.2f, %.2f, %.2f)\n",
                        maxW.x, maxW.y, maxW.z);
                }
            }
        }
    }

    return anyResolved;
}

bool CollisionManager::resolvePoint(const ICollidable* collidable, glm::vec3& point, float eyeHeight)
{
    if (!collidable || !collidable->isCollisionEnabled())
        return false;

    ContactFace face;

    if (collidable->usesOBBCollision())
    {
        return resolvePointAgainstOBB(collidable, point, eyeHeight, face);
    }
    else
    {
        glm::vec3 minW, maxW;
        collidable->getWorldAABB(minW, maxW);
        return resolvePointAgainstAABB(minW, maxW, point, eyeHeight, face);
    }
}

bool CollisionManager::resolvePointAgainstAABB(const glm::vec3& minW, const glm::vec3& maxW,
    glm::vec3& point, float eyeHeight, ContactFace& outFace)
{
    outFace = ContactFace::None;

    // Player's feet position (point is eye position, feet are below by eyeHeight)
    float feetY = point.y - eyeHeight;
    float headY = point.y;

    // First check if we're even near the AABB in XZ
    bool nearX = (point.x >= minW.x - 1.0f && point.x <= maxW.x + 1.0f);
    bool nearZ = (point.z >= minW.z - 1.0f && point.z <= maxW.z + 1.0f);

    if (!nearX || !nearZ)
        return false;

    // Check if player is within XZ bounds of the platform (with small tolerance)
    bool insideX = (point.x >= minW.x && point.x <= maxW.x);
    bool insideZ = (point.z >= minW.z && point.z <= maxW.z);
    bool insideXZ = insideX && insideZ;

    float topY = maxW.y;
    float bottomY = minW.y;

    // Case 1: Landing on TOP of platform
    // Player feet are penetrating from above
    if (insideXZ && feetY < topY && headY > topY)
    {
        point.y = topY + eyeHeight + collisionMargin;
        outFace = ContactFace::Top;
        return true;
    }

    // Case 2: Hitting BOTTOM of platform (ceiling)
    // Player head is penetrating from below
    if (insideXZ && headY > bottomY && feetY < bottomY)
    {
        point.y = bottomY - collisionMargin;
        outFace = ContactFace::Bottom;
        return true;
    }

    // Case 3: SIDE collisions - only if we're at the right height
    // Player is vertically aligned with the box
    if (feetY < topY && headY > bottomY)
    {
        // Check if we're penetrating in X or Z
        bool penetratingX = insideX;
        bool penetratingZ = insideZ;

        // Only resolve if we're penetrating on at least one axis
        if (penetratingX && penetratingZ)
        {
            // Inside on both axes - push out the shortest distance
            float distLeft = point.x - minW.x;
            float distRight = maxW.x - point.x;
            float distBack = point.z - minW.z;
            float distFront = maxW.z - point.z;

            float minDist = std::min({ distLeft, distRight, distBack, distFront });

            if (minDist == distLeft)
            {
                point.x = minW.x - collisionMargin;
                outFace = ContactFace::Left;
            }
            else if (minDist == distRight)
            {
                point.x = maxW.x + collisionMargin;
                outFace = ContactFace::Right;
            }
            else if (minDist == distBack)
            {
                point.z = minW.z - collisionMargin;
                outFace = ContactFace::Back;
            }
            else
            {
                point.z = maxW.z + collisionMargin;
                outFace = ContactFace::Front;
            }
            return true;
        }
    }

    return false;
}

bool CollisionManager::resolvePointAgainstOBB(const ICollidable* collidable,
    glm::vec3& point, float eyeHeight, ContactFace& outFace)
{
    outFace = ContactFace::None;

    // Get the model matrix and its inverse
    glm::mat4 model = collidable->getModelMatrix();
    glm::mat4 invModel = glm::inverse(model);

    // Transform player points to local space
    glm::vec3 feetWorld = point - glm::vec3(0.0f, eyeHeight, 0.0f);

    glm::vec4 localPoint4 = invModel * glm::vec4(point, 1.0f);
    glm::vec4 localFeet4 = invModel * glm::vec4(feetWorld, 1.0f);

    glm::vec3 localPoint(localPoint4.x, localPoint4.y, localPoint4.z);
    glm::vec3 localFeet(localFeet4.x, localFeet4.y, localFeet4.z);

    // Get local bounds (unrotated mesh bounds)
    glm::vec3 localMin, localMax;
    collidable->getLocalBounds(localMin, localMax);

    // Quick rejection test - are we even near the object?
    float maxDist = glm::length(localMax - localMin) * 0.5f + 2.0f;
    glm::vec3 center = (localMin + localMax) * 0.5f;
    if (glm::length(localPoint - center) > maxDist)
        return false;

    // Check XZ overlap in local space
    bool insideX = (localPoint.x >= localMin.x && localPoint.x <= localMax.x);
    bool insideZ = (localPoint.z >= localMin.z && localPoint.z <= localMax.z);
    bool insideXZ = insideX && insideZ;

    if (!insideXZ)
        return false;

    float topY = localMax.y;
    float bottomY = localMin.y;

    glm::vec3 resolvedLocal = localPoint;
    bool resolved = false;

    // Case 1: Collision with TOP (landing on surface)
    if (localFeet.y < topY && localPoint.y > topY)
    {
        resolvedLocal.y = topY + eyeHeight + collisionMargin;
        outFace = ContactFace::Top;
        resolved = true;
    }
    // Case 2: Collision with BOTTOM (ceiling)
    else if (localPoint.y > bottomY && localFeet.y < bottomY)
    {
        resolvedLocal.y = bottomY - collisionMargin;
        outFace = ContactFace::Bottom;
        resolved = true;
    }
    // Case 3: Collision with SIDES
    else if (localFeet.y < topY && localPoint.y > bottomY)
    {
        // We're at the right height and inside XZ - must be a side collision
        float distLeft = localPoint.x - localMin.x;
        float distRight = localMax.x - localPoint.x;
        float distBack = localPoint.z - localMin.z;
        float distFront = localMax.z - localPoint.z;

        float minDist = std::min({ distLeft, distRight, distBack, distFront });

        if (minDist == distLeft)
        {
            resolvedLocal.x = localMin.x - collisionMargin;
            outFace = ContactFace::Left;
        }
        else if (minDist == distRight)
        {
            resolvedLocal.x = localMax.x + collisionMargin;
            outFace = ContactFace::Right;
        }
        else if (minDist == distBack)
        {
            resolvedLocal.z = localMin.z - collisionMargin;
            outFace = ContactFace::Back;
        }
        else
        {
            resolvedLocal.z = localMax.z + collisionMargin;
            outFace = ContactFace::Front;
        }
        resolved = true;
    }

    if (resolved)
    {
        // Transform resolved position back to world space
        glm::vec4 worldResolved = model * glm::vec4(resolvedLocal, 1.0f);
        point = glm::vec3(worldResolved.x, worldResolved.y, worldResolved.z);
    }

    return resolved;
}

bool CollisionManager::isPointInsideAABB(const ICollidable* collidable, const glm::vec3& point) const
{
    if (!collidable)
        return false;

    glm::vec3 minW, maxW;
    collidable->getWorldAABB(minW, maxW);

    return (point.x >= minW.x && point.x <= maxW.x) &&
        (point.y >= minW.y && point.y <= maxW.y) &&
        (point.z >= minW.z && point.z <= maxW.z);
}