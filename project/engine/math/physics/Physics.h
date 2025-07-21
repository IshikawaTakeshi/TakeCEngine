#pragma once
#include "engine/math/physics/Ray.h"
#include <vector>

class Collider;

namespace Physics {
	// Returns true if the ray intersects with any collider in the scene
	bool Raycast(const Ray& ray, RayCastHit* outHit, const std::vector<Collider*>& colliders);

}