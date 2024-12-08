#pragma once
#include "Animation/Keyflame.h"
#include <vector>


template <typename T>
struct AnimationCurve {
	std::vector<Keyflame<T>> keyflames;
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animatoin {
	
};
