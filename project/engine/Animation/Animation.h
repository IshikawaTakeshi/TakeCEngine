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
	
	/// <summary>
	/// ベクトルの補間値を計算
	/// </summary>
	static Vector3 CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time);

	/// <summary>
	/// クォータニオンの補間値を計算
	/// </summary>
	/// <param name="keyframes"></param>
	/// <param name="time"></param>
	/// <returns></returns>
	static Quaternion CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time);

	/// <summary>
	///　アニメーションの尺を取得
	/// </summary>
	/// <returns></returns>
	float GetDuration() const { return duration; }

	/// <summary>
	/// ノードアニメーションを取得
	/// </summary>
	NodeAnimation& GetNodeAnimation(const std::string& nodeName);

	std::map<std::string, NodeAnimation>& GetNodeAnimations() { return nodeAnimations; }
private:

	
	float duration; //アニメーションの全体の尺(秒単位)
	std::map<std::string, NodeAnimation> nodeAnimations;
};

