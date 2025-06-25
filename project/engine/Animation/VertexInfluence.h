#pragma once
#include <array>

//最大の影響ボーン数
const uint32_t kNumMaxInfluence = 4;

struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};