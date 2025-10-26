#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "Matrix4x4.h"
#include "Animation/Animator.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <map>

//jointの構造体
struct Joint {
	QuaternionTransform transform; 
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;    //SkeletonSpaceでの変換行列
	std::string name;
	std::vector<int32_t> children;    //子Jointのインデックス
	int32_t index;                    //自身のインデックス
	std::optional<uint32_t> parent;   //親Jointのインデックス
};

//==============================================================
// Skeletonクラス
//==============================================================
class Skeleton {
public:

	//==========================================================
	// functions
	//==========================================================

	/// <summary>
	/// Skeletonの作成
	/// </summary>
	void Create(const Node& node);

	/// <summary>
	/// スケルトンの更新
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// スケルトンの描画
	/// </summary>
	/// <param name="worldMatrix"></param>
	void Draw(const Matrix4x4& worldMatrix);

	/// <summary>
	/// アニメーションの適用
	/// </summary>
	void ApplyAnimation(Animation* animation,float animationTime);


	//=====================================================
	// accessors
	//=====================================================

	//----- getter ---------------

	//スケルトンのRootJointのインデックスを取得
	const int32_t GetRoot() const { return root; }
	//ジョイントのリストを取得
	const std::vector<Joint>& GetJoints() { return joints; }
	//ジョイント名からインデックスのマップを取得
	const std::map<std::string, int32_t>& GetJointMap() { return jointMap; }

	// ジョイント名から値を取得
	std::optional<Joint> GetJointByName(const std::string& name) const;
	// ジョイント名からワールド行列を取得
	std::optional<Matrix4x4> GetJointWorldMatrix(const std::string& jointName, const Matrix4x4& characterWorldMatrix) const;
	// ジョイント名からワールド位置を取得
	std::optional<Vector3> GetJointPosition(const std::string& jointName, const Matrix4x4& modelWorldMatrix) const;

private:

	/// <summary>
	/// NodeからJointを作成
	/// </summary>
	/// <param name="node"></param>
	/// <param name="parent"></param>
	/// <param name="joints"></param>
	/// <returns></returns>
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent);


	int32_t root; //RootJointのインデックス
	std::map<std::string, int32_t> jointMap; //Joint名からindexとのマップ
	std::vector<Joint> joints; //Jointのリスト
};