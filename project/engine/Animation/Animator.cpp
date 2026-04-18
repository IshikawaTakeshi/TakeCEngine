#include "Animator.h"
#include "Easing.h"
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>
#include "Animator.h"

using namespace TakeC;

//=============================================================================
//	終了・開放処理
//=============================================================================
void AnimationManager::Finalize() {
	animations_.clear();
}
//=============================================================================
//	アニメーションの読み込み/登録
//=============================================================================

void AnimationManager::LoadAnimation(const std::string& filePath) {

	//読み込み済みアニメーションを検索
	if (animations_.contains(filePath)) {
		//すでに読み込み済みならreturn
		return;
	}

	//アニメーションの生成とファイル読み込み、初期化
	std::map<std::string, Animation*> animation = LoadAnimationFile(filePath);
	//アニメーションをコンテナに追加
	animations_.insert(std::make_pair(filePath, animation));
}

//=============================================================================
//	アニメーションの検索
//=============================================================================

Animation* AnimationManager::FindAnimation(const std::string& filePath, const std::string& animName) {
	//読み込み済みアニメーションを検索
	if (animations_.contains(filePath)) {
		if (animations_.at(filePath).contains(animName)) {

			//読み込みアニメーションを戻り値としてreturn
			return animations_.at(filePath).at(animName);
		}
	}

	//ファイル名一致なし
	assert(0 && "アニメーションが見つかりませんでした");
	return {};
}

//=============================================================================
//	アニメーションファイルの読み込み
//=============================================================================
std::map<std::string, Animation*> AnimationManager::LoadAnimationFile(const std::string& filename) {

	namespace fs = std::filesystem;

	// 拡張子から検索ルートを決める
	fs::path fileNamePath(filename);
	std::string ext = fileNamePath.extension().string();

	// ベースディレクトリ
	fs::path baseDir = "./Resources/Models/";

	// 最終的なフルパス
	fs::path fullPath = baseDir / filename;

	// ファイルが存在しない場合は、従来のフォルダ構成（fbx/, obj/, gltf/）を探索
	if (!fs::exists(fullPath)) {
		fs::path modelDir;
		if (ext == ".fbx") {
			modelDir = baseDir / "fbx/";
		} else if (ext == ".obj") {
			modelDir = baseDir / "obj/";
		} else if (ext == ".gltf" || ext == ".glb") {
			modelDir = baseDir / "gltf/";
		}
		fullPath = modelDir / filename;
	}

	std::map<std::string, Animation*> animations = {};
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fullPath.string().c_str(), 0);
    //アニメーションがない場合
	if(scene->mNumAnimations == 0) {
		return animations = {};
	}

	//複数のアニメーションの情報を取得
	for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
		aiAnimation* animationAssimp = scene->mAnimations[animationIndex];
		//animationインスタンスの生成
		Animation* animation = new Animation();
		//時間の単位を秒に変換
		animation->duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

		//assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報を取得する
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* NodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation->nodeAnimations[NodeAnimationAssimp->mNodeName.C_Str()];

			//position, rotation, scale keyframeを取得
			//position
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; //右手->左手に変換するので手動で対処する
				nodeAnimation.translate.keyframes.push_back(keyframe);
			}
			//rotation
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				aiQuatKey& keyAssimp = NodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyframeQuaternion keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
				nodeAnimation.rotate.keyframes.push_back(keyframe);
			}
			//scale
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.scale.keyframes.push_back(keyframe);
			}
		}

		//アニメーション名を取得
		std::string animationName = animationAssimp->mName.C_Str();
		animation->name = animationName;
		//アニメーションをmapに追加
		animations.insert(std::make_pair(animationName, animation));
	}

	//読み込んだアニメーションを返す
	return animations;
}

//=============================================================================
//	補間値の計算(Vector3用)
//=============================================================================

Vector3 AnimationManager::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	assert(!keyframes.empty()); //keyframesが空の場合はエラー
	if(keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if(time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			//範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	//最後のキーフレームを返す
	return (*keyframes.rbegin()).value;
}

//=============================================================================
//	補間値の計算(Quaternion用)
//=============================================================================
Quaternion AnimationManager::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	assert(!keyframes.empty()); //keyframesが空の場合はエラー
	if(keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if(time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			//範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	//最後のキーフレームを返す
	return (*keyframes.rbegin()).value;
}