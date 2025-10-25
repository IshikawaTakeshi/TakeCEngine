#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"
#include "math/AABB.h"
#include "3d/Model.h"
#include "3d/Particle/ParticleAttribute.h"
#include "camera/PerView.h"

#include <d3d12.h>
#include <wrl.h>
#include <random>
#include <list>

//Particle1個分のデータ
struct Particle {
	EulerTransform transforms_;  //位置
	Vector3 velocity_; 	    //速度
	Vector4 color_;         //色
	float lifeTime_;        //寿命
	float currentTime_;     //経過時間
};

// 前方宣言
class ParticleCommon;

//============================================================================
// BaseParticleGroup class
//============================================================================
class BaseParticleGroup {
public:
	BaseParticleGroup() = default;
	virtual ~BaseParticleGroup() = default;

	//=========================================================================
	// functions
	//=========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleCommon"></param>
	/// <param name="filePath"></param>
	virtual void Initialize(ParticleCommon* particleCommon, const std::string& filePath) = 0;

	/// <summary>
	// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	virtual void UpdateImGui() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw();

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate,const Vector3& directoin);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos,const Vector3& direction, uint32_t particleCount);

	/// <summary>
	/// パーティクルの追加
	/// </summary>
	/// <param name="particles"></param>
	void SpliceParticles(std::list<Particle> particles);

public:

	//=========================================================================
	// accessor
	//=========================================================================

	//----- getter ---------------

	/// <summary>
	/// パーティクルのプリセット取得
	/// </summary>
	/// <returns></returns>
	const ParticlePreset& GetPreset() const { return particlePreset_; }

	//----- setter ---------------

	/// <summary>
	/// パーティクルのプリセット設定
	/// </summary>
	/// <param name="preset"></param>
	virtual void SetPreset(const ParticlePreset& preset);

	/// <summary>
	/// エミッターの座標設定
	/// </summary>
	/// <param name="position"></param>
	void SetEmitterPosition(const Vector3& position);

protected:

	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 10240; 
	//1フレームの時間
	float kDeltaTime_;
	//描画するインスタンス数
	uint32_t numInstance_ = 0; 
	//Particleの配列
	std::list<Particle> particles_; 

	//パーティクルの属性
	ParticlePreset particlePreset_;

protected:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;

	//エミッターの座標
	Vector3 emitterPos_ = { 0.0f, 0.0f, 0.0f };

	//instancing用のデータ
	ParticleForGPU* particleData_ = nullptr;
	//ビュー行列用データ
	PerView* perViewData_ = nullptr;
	//Srvのインデックス
	uint32_t particleSrvIndex_ = 0;
	//Srvのインデックス
	ComPtr<ID3D12Resource> particleResource_;
	//ビュー行列用リソース
	ComPtr<ID3D12Resource> perViewResource_;
};