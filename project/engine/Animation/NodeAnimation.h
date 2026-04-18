#pragma once
#include "Animation/Keyflame.h"
#include <vector>
#include <string>
#include <map>

//============================================================================
// NodeAnimation.h
//============================================================================

// アニメーションカーブ構造体
template <typename T>
struct AnimationCurve {
	std::vector<Keyframe<T>> keyframes; //キーフレームの配列
};

//ノードアニメーション構造体
struct NodeAnimation {
	AnimationCurve<Vector3> translate; //移動
	AnimationCurve<Quaternion> rotate; //回転
	AnimationCurve<Vector3> scale;     //拡縮
};

//アニメーション構造体
struct Animation {
	std::string name; //アニメーション名
	float duration; //アニメーションの全体の尺(秒単位)
	std::map<std::string, NodeAnimation> nodeAnimations; //ノードアニメーションのマップ
};