#include "Animation.h"
#include "Easing.h"
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>


Animation Animation::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
    Animation animation;
    Assimp::Importer importer;
    std::string filePath ="./Resources/" + directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
    //アニメーションがない場合
	if(scene->mNumAnimations == 0) {
		return animation;
	}
    aiAnimation* animationAssimp = scene->mAnimations[0];
    animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換

    //assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報を取得する
    for(uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* NodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[NodeAnimationAssimp->mNodeName.C_Str()];

		//position, rotation, scaleのkeyflameを取得
		//position
        for(uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = NodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyflameVector3 keyflame;
			keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
			keyflame.value = {-keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z}; //右手->左手に変換するので手動で対処する
			nodeAnimation.translate.keyflames.push_back(keyflame);
		}
		//rotation
		for(uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = NodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyflameQuaternion keyflame;
			keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
			keyflame.value = {keyAssimp.mValue.w, keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.rotate.keyflames.push_back(keyflame);
		}
		//scale
		for(uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = NodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyflameVector3 keyflame;
			keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
			keyflame.value = {keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z};
			nodeAnimation.scale.keyflames.push_back(keyflame);
		}

	}
	return animation;
}

Vector3 Animation::CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time) {
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

Quaternion Animation::CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time) {
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

NodeAnimation& Animation::GetNodeAnimation(const std::string& nodeName) {
	return nodeAnimations.at(nodeName);
}


