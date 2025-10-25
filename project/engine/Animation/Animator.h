#pragma once
#include "Animation/NodeAnimation.h"
#include "Matrix4x4.h"
#include <string>
#include <unordered_map>

//============================================================================
// Animator class
//============================================================================
class Animator {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Animator() = default;
	~Animator() = default;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// アニメーション読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filePath"></param>
	void LoadAnimation(const std::string& directoryPath,const std::string& filePath);

	/// <summary>
	/// アニメーション検索
	/// </summary>
	/// <param name="filePath"></param>
	/// <param name="animName"></param>
	/// <returns></returns>
	Animation* FindAnimation(const std::string& filePath,const std::string& animName);

	/// <summary>
	/// アニメーションファイル読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	static std::map<std::string, Animation*> LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	
	/// <summary>
	/// ベクトルの補間値を計算
	/// </summary>
	static Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);

	/// <summary>
	/// クォータニオンの補間値を計算
	/// </summary>
	/// <param name="keyframes"></param>
	/// <param name="time"></param>
	/// <returns></returns>
	static Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

private:

	/// <summary>
	/// アニメーションのコンテナ
	/// (ファイル名, (アニメーション名,アニメーション))
	/// </summary>
	std::map<std::string,std::map<std::string, Animation*>> animations_;
};

