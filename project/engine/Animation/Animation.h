#pragma once
#include "Animation/NodeAnimation.h"
#include "Matrix4x4.h"
#include <string>
#include <map>

class Animation {
public:

	Animation() = default;
	~Animation() = default;

	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	static Vector3 CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time);

	static Quaternion CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time);

	float GetDuration() const { return duration; }

	NodeAnimation& GetNodeAnimation(const std::string& nodeName);

private:

	
	float duration; //アニメーションの全体の尺(秒単位)
	std::map<std::string, NodeAnimation> nodeAnimations;
};

