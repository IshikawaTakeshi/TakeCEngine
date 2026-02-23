#include "Skeleton.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

//====================================================================
// スケルトン生成
//====================================================================
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

//====================================================================
// スケルトン更新
//====================================================================
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

//====================================================================
// ImGui更新
//====================================================================
void Skeleton::UpdateImGui() {
#ifdef _DEBUG
	for (Joint& joint : joints) {
		ImGui::SeparatorText(joint.name.c_str());
		ImGui::DragFloat3("Scale", &joint.transform.scale.x, 0.01f);
		ImGui::DragFloat4("Rotate", &joint.transform.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &joint.transform.translate.x, 0.01f);
	}
#endif
}

//====================================================================
// スケルトン描画
//====================================================================
void Skeleton::Draw(const Matrix4x4& worldMatrix) {

	//jointの描画
	for (const Joint& joint : joints) {

		// ボーンのワールド行列を計算
		Matrix4x4 jointWorldMatrix = joint.skeletonSpaceMatrix * worldMatrix;

		// jointWorldMatrix からボーンの位置を取得して描画
		// 例: Jointの原点（0,0,0）をjointWorldMatrixで変換してボーンのワールド位置に
		Vector3 jointWorldPos = MatrixMath::Transform(
			Vector3(0, 0, 0), jointWorldMatrix);
		//親がいない場合はルートJointなので球で描画
		if (!joint.parent) {
			TakeCFrameWork::GetWireFrame()->DrawSphere(
				jointWorldPos,
				0.1f, { 1.0f,1.0f,0.1f,1.0f });
		} else {
			//親がいる場合は親との線を描画
			if (joint.parent.has_value()) {
				Vector3 parentWorldPos = MatrixMath::Transform(Vector3(0, 0, 0), joints[joint.parent.value()].skeletonSpaceMatrix * worldMatrix);
				TakeCFrameWork::GetWireFrame()->DrawLine(jointWorldPos, parentWorldPos, { 1.0f,1.0f,1.0f,1.0f }); // 仮想関数
			}

			//Jointを球で描画
			TakeCFrameWork::GetWireFrame()->DrawSphere(
				jointWorldPos,
				0.1f, { 1.0f,0.1f,0.1f,1.0f });
		}
	}
}


//====================================================================
// アニメーションの適用
//====================================================================
void Skeleton::ApplyAnimation(Animation* animation, float animationTime) {
	for (Joint& joint : joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。
		if (auto it = animation->nodeAnimations.find(joint.name); it != animation->nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.scale.keyframes, animationTime);
			joint.transform.rotate = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime);
			joint.transform.translate = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.translate.keyframes, animationTime);
		}
	}
}

//====================================================================
// ブレンドアニメーションの適用（クロスフェード）
//====================================================================
void Skeleton::ApplyBlendedAnimation(Animation* from, float tFrom, Animation* to, float tTo, float blend) {
	for (Joint& joint : joints) {
		QuaternionTransform fromTransform = joint.transform;

		// 遷移元アニメーションのサンプリング
		if (from && from->duration > 0.0f) {
			auto it = from->nodeAnimations.find(joint.name);
			if (it != from->nodeAnimations.end()) {
				const NodeAnimation& nodeAnim = it->second;
				fromTransform.scale = TakeC::AnimationManager::CalculateValue(nodeAnim.scale.keyframes, tFrom);
				fromTransform.rotate = TakeC::AnimationManager::CalculateValue(nodeAnim.rotate.keyframes, tFrom);
				fromTransform.translate = TakeC::AnimationManager::CalculateValue(nodeAnim.translate.keyframes, tFrom);
			}
		}

		// ブレンド比率が0ならfromのみ適用
		if (!to || to->duration <= 0.0f || blend <= 0.0f) {
			joint.transform = fromTransform;
			continue;
		}

		// 遷移先アニメーションのサンプリング
		QuaternionTransform toTransform = fromTransform;
		auto it = to->nodeAnimations.find(joint.name);
		if (it != to->nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = it->second;
			toTransform.scale = TakeC::AnimationManager::CalculateValue(nodeAnim.scale.keyframes, tTo);
			toTransform.rotate = TakeC::AnimationManager::CalculateValue(nodeAnim.rotate.keyframes, tTo);
			toTransform.translate = TakeC::AnimationManager::CalculateValue(nodeAnim.translate.keyframes, tTo);
		}

		// ブレンド比率が1ならtoのみ適用
		if (blend >= 1.0f) {
			joint.transform = toTransform;
			continue;
		}

		// Lerp/Slerpによるブレンド
		joint.transform.scale = Easing::Lerp(fromTransform.scale, toTransform.scale, blend);
		joint.transform.rotate = Easing::Slerp(fromTransform.rotate, toTransform.rotate, blend);
		joint.transform.translate = Easing::Lerp(fromTransform.translate, toTransform.translate, blend);
	}
}

//====================================================================
// ジョイント名から値を取得
//====================================================================
std::optional<Joint> Skeleton::GetJointByName(const std::string& name) const {
	auto it = jointMap.find(name);
	if (it != jointMap.end()) {
		return joints[it->second];
	}
	return std::nullopt; //見つからなかった場合はstd::nulloptを返す
}

//====================================================================
// ジョイント名からワールド行列を取得
//====================================================================
std::optional<Matrix4x4> Skeleton::GetJointWorldMatrix(const std::string& jointName, const Matrix4x4& characterWorldMatrix) const {
	auto it = jointMap.find(jointName);
	if (it != jointMap.end()) {
		return joints[it->second].skeletonSpaceMatrix * characterWorldMatrix;
	}
	return std::nullopt; //見つからなかった場合はstd::nulloptを返す
}

//====================================================================
// ジョイント名からワールド位置を取得
//====================================================================
std::optional<Vector3> Skeleton::GetJointPosition(const std::string& jointName, const Matrix4x4& modelWorldMatrix) const {
	auto it = jointMap.find(jointName);
	if (it != jointMap.end()) {
		int32_t index = it->second;
		Matrix4x4 jointWorldMatrix = joints[index].skeletonSpaceMatrix * modelWorldMatrix;
		Vector3 worldPos = { jointWorldMatrix.m[3][0], jointWorldMatrix.m[3][1], jointWorldMatrix.m[3][2] };
		return worldPos; // Jointのワールド位置を返す
	}

	return std::nullopt;
}

//====================================================================
// NodeからJointを作成
//====================================================================
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
