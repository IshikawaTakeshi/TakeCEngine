#pragma once
#include "3d/Particle/BaseParticleGroup.h"

//加速フィールド
struct AccelerationField {
	Vector3 acceleration_; //加速度
	Vector3 position_;     //位置
	AABB aabb_;            //当たり判定
};

// 前方宣言
class ParticleCommon;

//============================================================================
// Particle3d class
//============================================================================
class Particle3d : public BaseParticleGroup {
public:

	Particle3d() = default;
	~Particle3d() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ParticleCommon* particleCommon,const std::string& filePath) override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	/// <summary>
	/// パーティクルのプリセット設定
	/// </summary>
	/// <param name="preset"></param>
	void SetPreset(const ParticlePreset& preset) override { particlePreset_ = preset; }

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
	void SpliceParticles(std::list<Particle> particles);

private:

	/// <summary>
	/// モデルの設定
	/// </summary>
	void SetModel(const std::string& filePath);

private:
	//モデル
	Model* model_ = nullptr;

private:

	/// <summary>
	/// パーティクルの移動更新
	/// </summary>
	void UpdateMovement(std::list<Particle>::iterator particleIterator);
};