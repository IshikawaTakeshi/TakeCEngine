#include "AABB.h"

void AABB::Reset() {
	min = Vector3{
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max()
	};
	max = Vector3{ 
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest()
	};
}

void AABB::Expand(const Vector3& point) {
	min. x = std::min(min.x, point.x);
	min.y = std::min(min.y, point. y);
	min.z = std:: min(min.z, point.z);
	max.x = std::max(max.x, point.x);
	max.y = std::max(max. y, point.y);
	max.z = std::max(max.z, point.z);
}

Vector3 AABB::GetSize() const {
	return Vector3{
		max.x - min.x,
		max.y - min.y,
		max.z - min.z
	};
}

Vector3 AABB::GetCenter() const {
	return Vector3{
		(min.x + max.x) * 0.5f,
		(min.y + max.y) * 0.5f,
		(min.z + max.z) * 0.5f
	};
}