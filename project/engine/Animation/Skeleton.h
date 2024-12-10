#pragma once
#include "Transform.h"
#include "Matrix4x4.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <map>

struct Joint {
	QuaternionTransform transform; 
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;    //SkeletonSpaceでの変換行列
	std::string name;
	std::vector<int32_t> children;    //子Jointのインデックス
	int32_t index;                    //自身のインデックス
	std::optional<uint32_t> parent;   //親Jointのインデックス
};

struct Skeleton {
	int32_t root; //RootJointのインデックス
	std::map<std::string, int32_t> jointMap; //Joint名からindexとのマップ
	std::vector<Joint> joints; //Jointのリスト
};