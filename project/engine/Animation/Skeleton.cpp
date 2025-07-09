#include "Skeleton.h"
#include "engine/math/MatrixMath.h"
#include "engine/base/TakeCFrameWork.h"

void Skeleton::Create(const Node& rootNode) {

	root = CreateJoint(rootNode, {});

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : joints) {
		jointMap.emplace(joint.name, joint.index);
	}

	for (Joint& joint : joints) {

		//ローカル行列を更新
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Update() {
	//全てのJointを更新
	for (Joint& joint : joints) {

		//ローカル行列を更新
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Draw(const Matrix4x4& worldMatrix) {

	//jointの描画
	for (const Joint& joint : joints) {

		// ボーンのワールド行列を計算
		Matrix4x4 jointWorldMatrix =joint.skeletonSpaceMatrix * worldMatrix;

		// jointWorldMatrix からボーンの位置を取得して描画
		// 例: Jointの原点（0,0,0）をjointWorldMatrixで変換してボーンのワールド位置に
		Vector3 jointWorldPos = MatrixMath::Transform(
			Vector3(0,0,0), jointWorldMatrix);
		//親がいない場合はルートJointなので球で描画
		if (!joint.parent) {
			TakeCFrameWork::GetWireFrame()->DrawSphere(
				{ joint.skeletonSpaceMatrix.m[3][0], joint.skeletonSpaceMatrix.m[3][1], joint.skeletonSpaceMatrix.m[3][2] },
				0.1f, {1.0f,1.0f,0.1f,1.0f});
		} else {
			//親がいる場合は親との線を描画
			if (joint.parent.has_value()) {
				Vector3 parentWorldPos = MatrixMath::Transform(Vector3(0,0,0), joints[joint.parent.value()].skeletonSpaceMatrix * worldMatrix);
				TakeCFrameWork::GetWireFrame()->DrawLine(jointWorldPos, parentWorldPos,{1.0f,1.0f,1.0f,1.0f}); // 仮想関数
			}

		}
	}
}

void Skeleton::ApplyAnimation(Animation* animation, float animationTime) {
	for (Joint& joint : joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。
		if (auto it = animation->nodeAnimations.find(joint.name); it != animation->nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
			joint.transform.rotate = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
			joint.transform.translate = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
		}
	}
}

int32_t Skeleton::CreateJoint(const Node& node, const std::optional<int32_t>& parent) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MatrixMath::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint); //SkeletonのJoint列に追加
	for (const Node& child : node.children) {
		//子Jointを作成して、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index);
		joints[joint.index].children.push_back(childIndex);
	}

	//自身のIndexを返す
	return joint.index;
}
