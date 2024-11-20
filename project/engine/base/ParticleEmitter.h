#pragma once
#include "Transform.h"
#include <cstdint>
#include <memory>
#include <string>

class ParticleManager;
class ParticleEmitter {
public:

	ParticleEmitter() = default;
	~ParticleEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleName">パーティクルグループ名</param>
	/// <param name="transforms">SRT</param>
	/// <param name="count">発生させるパーティクルの数</param>
	/// <param name="frequency">発生頻度</param>
	void Initialize(const std::string& particleName, Transform transforms,uint32_t count, float frequency);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	void Emit();

private:

	static const uint32_t kNumMaxInstance_ = 100; //Particleの総数
	const float kDeltaTime_ = 1.0f / 60.0f; //1フレームの時間
	bool isEmit_; //発生フラグ
	Transform transforms_;   //エミッターの位置
	uint32_t particleCount_; //発生するParticleの数
	float frequency_;        //発生頻度
	float frequencyTime_;    //経過時間
	std::string particleName_; //発生するParticleの名前
};

