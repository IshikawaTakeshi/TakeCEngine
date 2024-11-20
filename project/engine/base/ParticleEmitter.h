#pragma once
#include "Transform.h"
#include <cstdint>
#include <memory>
#include <string>

class Model;
class ParticleManager;
class ParticleEmitter {
public:

	ParticleEmitter() = default;
	~ParticleEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="emitterName">emitter名</param>
	/// <param name="transforms">SRT</param>
	/// <param name="count">発生させるパーティクルの数</param>
	/// <param name="frequency">発生頻度</param>
	void Initialize(const std::string& emitterName, Transform transforms,uint32_t count, float frequency);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	//void DrawEmitRange();

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	void Emit();

	void SetParticleName(const std::string& particleName) {
		particleName_ = particleName;
	}

private:

	//Model* model_ = nullptr;

	static const uint32_t kNumMaxInstance_ = 100; //Particleの総数
	const float kDeltaTime_ = 1.0f / 60.0f; //1フレームの時間
	bool isEmit_; //発生フラグ
	Transform transforms_;   //エミッターの位置
	uint32_t particleCount_; //発生するParticleの数
	float frequency_;        //発生頻度
	float frequencyTime_;    //経過時間
	std::string emitterName_; //emitterの名前
	std::string particleName_; //発生させるParticleの名前
};

