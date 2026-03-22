#include <iostream>
#include <cmath>

struct Vector3 {
    float x, y, z;
};

float length(const Vector3& v) {
    return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

int main() {
    Vector3 weaponPos = {0, 0, 0};
    Vector3 targetPos = {1000, 0, 0}; // 1000 distance
    Vector3 targetVel = {0, 0, 50};   // 50 speed perpendicular
    float speed_ = 500.0f;
    
    float distance = length({targetPos.x - weaponPos.x, targetPos.y - weaponPos.y, targetPos.z - weaponPos.z});
    float travelTime = distance / speed_;
    
    Vector3 predictedTargetPos = {
        targetPos.x + targetVel.x * travelTime,
        targetPos.y + targetVel.y * travelTime,
        targetPos.z + targetVel.z * travelTime
    };
    
    Vector3 diff = {
        predictedTargetPos.x - weaponPos.x,
        predictedTargetPos.y - weaponPos.y,
        predictedTargetPos.z - weaponPos.z
    };
    
    float len = length(diff);
    Vector3 direction = { diff.x / len, diff.y / len, diff.z / len };
    
    std::cout << "Distance: " << distance << "\n";
    std::cout << "Travel Time: " << travelTime << "\n";
    std::cout << "Predicted Pos: " << predictedTargetPos.x << ", " << predictedTargetPos.y << ", " << predictedTargetPos.z << "\n";
    std::cout << "Direction: " << direction.x << ", " << direction.y << ", " << direction.z << "\n";
    
    return 0;
}
