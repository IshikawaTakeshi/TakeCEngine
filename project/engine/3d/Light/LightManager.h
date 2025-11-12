#pragma once
#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/3d/Light/DirectionalLight.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Light/SpotLight.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <span>

struct LightCountData {
	uint32_t pointLightCount;
	uint32_t spotLightCount;
	uint32_t padding[2];
};

//============================================================================
//		LightManager class
//============================================================================

class DirectXCommon;
class SrvManager;
class PSO;
class LightManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================
	LightManager() = default;
	~LightManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// ポイントライト追加
	/// </summary>
	/// <param name="lightData"></param>
	/// <returns></returns>
	uint32_t AddPointLight(const PointLightData& lightData);

	uint32_t AddSpotLight(const SpotLightData& lightData);

	/// <summary>
	/// ポイントライト更新
	/// </summary>
	/// <param name="index"></param>
	/// <param name="light"></param>
	void UpdatePointLight(uint32_t index, const PointLightData& light);

	void UpdateSpotLight(uint32_t index, const SpotLightData& light);

	/// <summary>
	/// GPUへデータ転送
	/// </summary>
	void TransferToGPU();

public:

	//========================================================================
	// accessors
	//========================================================================

	//----- getter ---------------------------
	
	/// ポイントライトの読み取り専用ビューを取得
	std::span<const PointLightData> GetPointLights() const {
		return std::span<const PointLightData>(pointLightData_);
	}
	/// スポットライトの読み取り専用ビューを取得
	std::span<const SpotLightData> GetSpotLights() const {
		return std::span<const SpotLightData>(spotLightData_);
	}

	//----- setter ---------------------------

	/// ライト用リソースの設定
	void SetLightResources(PSO* pso);
private:

	//========================================================================
	//	private variables
	//========================================================================

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	DirectionalLightData* dirLightData_ = nullptr;
	std::vector<PointLightData> pointLightData_;
	std::vector<SpotLightData> spotLightData_;
	LightCountData* lightCountData_ = nullptr;

	ComPtr<ID3D12Resource> dirLightResource_;
	ComPtr<ID3D12Resource> pointLightResource_;
	ComPtr<ID3D12Resource> spotLightResource_;
	ComPtr<ID3D12Resource> lightCountResource_;

	uint32_t pointLightSrvIndex_ = 0;
	uint32_t spotLightSrvIndex_ = 0;

	static const uint32_t kMaxPointLights = 32;   // 最大ポイントライト数
	static const uint32_t kMaxSpotLights = 32;    // 最大スポットライト数
};