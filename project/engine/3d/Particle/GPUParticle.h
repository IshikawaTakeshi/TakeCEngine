#pragma once
#include "engine/base/PerFrame.h"
#include "engine/3d/Model.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/3d/Particle/ParticleAttribute.h"
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"
#include "engine/math/Matrix4x4.h"
#include "engine/camera/Camera.h"
#include "engine/camera/PerView.h"
#include <memory>

//ComputeShader用パーティクルデータ構造体
struct ParticleForCS {
	Vector3 translate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

//=============================================================================
// GPUParticle class
//=============================================================================
class GPUParticle {
public:

	GPUParticle() = default;
	~GPUParticle() = default;

	//==================================================================================
	//	functions
	//==================================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleCommon"></param>
	/// <param name="filePath"></param>
	void Initialize(ParticleCommon* particleCommon,  const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// パーティクル初期化ディスパッチ
	/// </summary>
	void DisPatchInitializeParticle();

	/// <summary>
	/// パーティクル更新ディスパッチ
	/// </summary>
	void DisPatchUpdateParticle();

public:
	//==================================================================================
	//		accessor
	//==================================================================================

	//----- getter ---------------

	//particleUavIndexの取得
	uint32_t GetParticleUavIndex() const { return particleUavIndex_; }
	//freeListIndexUavIndexの取得
	uint32_t GetFreeListIndexUavIndex() const { return freeListIndexUavIndex_; }
	//freeListUavIndexの取得
	uint32_t GetFreeListUavIndex() const { return freeListUavIndex_; }
	//attributeSrvIndexの取得
	uint32_t GetAttributeSrvIndex() const { return attributeSrvIndex_; }
	//particleUavResourceの取得
	ID3D12Resource* GetParticleUavResource() const { return particleUavResource_.Get(); }

	//----- setter ---------------

	//プリセットの設定
	void SetPreset(const ParticlePreset& preset) {
		particlePreset_ = preset;
	}

private:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;

	//ComputeShader用パーティクルデータ
	ParticleForCS* particleData_ = nullptr;
	//UAVIndex
	uint32_t particleUavIndex_ = 0;
	uint32_t freeListIndexUavIndex_ = 0;
	uint32_t freeListUavIndex_ = 0;

	//PerViewData
	PerView* perViewData_ = nullptr;
	//PerFrameData
	PerFrame* perFrameData_ = nullptr;
	//ParticlePreset
	ParticlePreset particlePreset_;
	//ParticleAttributes
	ParticleAttributes* particleAttributes_;

	//SRVIndex
	uint32_t attributeSrvIndex_ = 0;
	uint32_t primitiveHandle_ = 0;

	//パーティクルの最大数
	static const uint32_t kNumMaxInstance_ = 1024000;

	//リソース
	ComPtr<ID3D12Resource> particleUavResource_ = nullptr;
	ComPtr<ID3D12Resource> perViewResource_ = nullptr;
	ComPtr<ID3D12Resource> freeListResource_ = nullptr;
	ComPtr<ID3D12Resource> freeListIndexResource_ = nullptr;
	ComPtr<ID3D12Resource> perFrameResource_ = nullptr;
	ComPtr<ID3D12Resource> attributeResource_ = nullptr;
};