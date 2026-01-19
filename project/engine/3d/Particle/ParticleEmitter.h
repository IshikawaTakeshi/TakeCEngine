#pragma once
#include "engine/math/Transform.h"
#include "engine/base/PipelineStateObject.h"
#include "engine/base/PerFrame.h"
#include "engine/3d/Particle/ParticleAttribute.h"
#include <cstdint>
#include <memory>
#include <string>

//球状エミッター情報構造体
struct EmitterSphereInfo {
	Vector3 translate; //エミッターの位置
	float radius; //射出半径
	float frequency; //発生頻度
	float frequencyTime; //経過時間
	uint32_t particleCount; //発生するParticleの数
	uint32_t isEmit; //発生許可
};

// 前方宣言
namespace TakeC {
	class DirectXCommon;
	class SrvManager;
}
class GPUParticle;

//============================================================================
// ParticleEmitter class
//============================================================================
class ParticleEmitter {
public:

	ParticleEmitter() = default;
	~ParticleEmitter() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="emitterName">emitter名</param>
	/// <param name="transforms">SRT</param>
	/// <param name="count">発生させるパーティクルの数</param>
	/// <param name="frequency">発生頻度</param>
	void Initialize(const std::string& emitterName, const std::string& presetInfo);

	/// <summary>
	/// 球状エミッター初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void InitializeEmitterSphere(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// GPUパーティクル用の更新処理
	/// </summary>
	void UpdateForGPU();

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理(ワイヤーフレーム)
	/// </summary>
	void DrawWireFrame();

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	void Emit();
	void Emit(const Vector3& position);

	void EmitParticle(GPUParticle* gpuParticle);

public:

	//==================================================================================
	//		accessor
	//==================================================================================

	//----- getter ---------------

	//発生させるParticleの名前取得
	const std::string& GetEmitterName() const { return emitterName_; }
	//発生させるParticleの名前取得
	const bool IsEmit() const { return isEmit_; }
	//エミッターの位置取得
	const EulerTransform& GetTransforms() const { return transforms_; }
	//発生させるParticleの数取得
	const uint32_t GetParticleCount() const { return particleCount_; }
	//発生頻度取得
	const float GetFrequency() const { return frequency_; }
	//発生方向取得
	const Vector3& GetEmitDirection() const { return emitDirection_; }

	//----- setter ---------------

	//発生させるParticleの名前設定
	void SetParticleName(const std::string& particleName);
	//emitter名の設定
	void SetEmitterName(const std::string& emitterName) { emitterName_ = emitterName; }

	//エミッターの位置設定
	void SetTranslate(const Vector3& translate) {transforms_.translate = translate; }
	//エミッターの回転設定
	void SetRotate(const Vector3& rotate) { transforms_.rotate = rotate; }
	//エミッターのスケール設定
	void SetScale(const Vector3& scale) { transforms_.scale = scale; }

	//発生フラグ設定
	void SetIsEmit(bool isEmit) { isEmit_ = isEmit; }
	//発生頻度設定
	void SetFrequency(float frequency) { frequency_ = frequency; }
	//発生させるParticleの数設定
	void SetParticleCount(uint32_t count) { particleCount_ = count; }
	//発生方向設定
	void SetEmitDirection(const Vector3& direction) { emitDirection_ = direction; }

private:
	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 1024;

	//DirectXCommon
	TakeC::DirectXCommon* dxCommon_;
	//SrvManager
	TakeC::SrvManager* srvManager_;
	//PSO
	std::unique_ptr<PSO> emitParticlePso_;
	//RootSignature
	ComPtr<ID3D12RootSignature> emitParticleRootSignature_;

	//球状エミッター情報
	EmitterSphereInfo* emitterSphereInfo_ = nullptr;
	uint32_t emitterSphereSrvIndex_ = 0;
	//フレーム時間の情報
	PerFrame* perFrameData_ = nullptr;

	//リソース
	ComPtr<ID3D12Resource> emitterSphereResource_;
	ComPtr<ID3D12Resource> perFrameResource_;

	bool isEmit_; //発生フラグ
	ParticlePreset preset_; //プリセット情報
	EulerTransform transforms_;   //エミッターの位置
	Vector3 prevTranslate_; //前回のエミッター位置
	uint32_t particleCount_; //発生するParticleの数
	float frequency_;        //発生頻度
	float frequencyTime_;    //経過時間
	Vector3 emitDirection_; //発生方向
	std::string emitterName_; //emitterの名前
	std::string particleName_; //発生させるParticleの名前

	uint32_t particlesPerInterpolation = 5; //一度の補間で生成するパーティクル数
	float trailEmitInterval = 0.016f; //トレイルエフェクトの生成間隔
	int maxInterpolationCount_ = 0; //最大補間回数
};