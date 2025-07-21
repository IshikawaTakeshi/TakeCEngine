#include "Physics.h"
#include "engine//Collision/Collider.h"

bool Physics::Raycast(const Ray& ray, RayCastHit* outHit, const std::vector<Collider*>& colliders) {
    bool result = false;
    float closestDistance = ray.distance;
	RayCastHit closestHit;

    for (auto* collider : colliders) {
        if (collider->Intersects(ray, *outHit)) {
            if (closestHit.distance < closestDistance) {
                closestDistance = closestHit.distance;
                *outHit = closestHit;
                result = true;
            }
        }
    }

	return result;
}