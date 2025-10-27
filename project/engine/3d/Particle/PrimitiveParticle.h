#pragma once
#include "3d/Particle/BaseParticleGroup.h"
#include "Primitive/PrimitiveType.h"

//============================================================================
// PrimitiveParticle class
//============================================================================
class PrimitiveParticle : public BaseParticleGroup {

public:
	PrimitiveParticle(PrimitiveType type);
	~PrimitiveParticle() = default;

	//=========================================================================
	// functions
	//=========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ParticleCommon* particleCommon, const std::string& filePath) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate,const Vector3& direction);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos,const Vector3& direction, uint32_t particleCount);

	/// <summary>
	/// パーティクルの配列の結合処理
	/// </summary>
	/// <param name="particles"></param>
	void SpliceParticles(std::list<Particle> particles);

	/// <summary>
	/// パーティクルのプリセット設定
	/// </summary>
	void SetPreset(const ParticlePreset& preset) override;

	/// <summary>
	/// プリミティブのハンドル設定
	/// </summary>
	void SetPrimitiveHandle(uint32_t handle) { primitiveHandle_ = handle; }

private:
	uint32_t primitiveHandle_ = 0; // プリミティブのハンドル

	std::list<Particle> pendingParticles_; // 発生待ちパーティクルのリスト
private:

	/// <summary>
	/// パーティクルの移動更新
	/// </summary>
	/// <param name="particleIterator"></param>
	void UpdateMovement(std::list<Particle>::iterator particleIterator);
};