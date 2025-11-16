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
	/// ImGui更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// ポイントライト追加
	/// </summary>
	/// <param name="lightData"></param>
	/// <returns></returns>
	uint32_t AddPointLight(const PointLightData& lightData);

	/// <summary>
	/// スポットライト追加
	/// </summary>
	/// <param name="lightData"></param>
	/// <returns></returns>
	uint32_t AddSpotLight(const SpotLightData& lightData);

	/// <summary>
	/// ポイントライト削除
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	bool RemovePointLight(uint32_t index);

	/// <summary>
	/// スポットライト削除
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	bool RemoveSpotLight(uint32_t index);

	/// <summary>
	/// ポイントライト更新
	/// </summary>
	/// <param name="index"></param>
	/// <param name="light"></param>
	void UpdatePointLight(uint32_t index, const PointLightData& light);

	/// <summary>
	/// スポットライト更新
	/// </summary>
	/// <param name="index"></param>
	/// <param name="light"></param>
	void UpdateSpotLight(uint32_t index, const SpotLightData& light);

	/// <summary>
	/// ポイントライト描画
	/// </summary>
	void DrawPointLights();

	/// <summary>
	/// スポットライト描画
	/// </summary>
	void DrawSpotLights();

public:

	//========================================================================
	// accessors
	//========================================================================

	//----- getter ---------------------------
	
	PointLightData* GetPointLightData(uint32_t index) const;
	SpotLightData* GetSpotLightData(uint32_t index) const;

	ID3D12Resource* GetDirectionalLightResource() const {
		return dirLightResource_.Get();
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

	ComPtr<ID3D12Resource> dirLightResource_;
	DirectionalLightData* dirLightData_ = nullptr;

	ComPtr<ID3D12Resource> pointLightResource_;
	PointLightData* pointLightData_;
	uint32_t activePointLightCount_ = 0; // アクティブなポイントライト数
	uint32_t pointLightSrvIndex_ = 0;

	ComPtr<ID3D12Resource> spotLightResource_;
	uint32_t activeSpotLightCount_ = 0; // アクティブなスポットライト数
	SpotLightData* spotLightData_;
	uint32_t spotLightSrvIndex_ = 0;

	ComPtr<ID3D12Resource> lightCountResource_;
	LightCountData* lightCountData_ = nullptr;

	static const uint32_t kMaxPointLights = 32;   // 最大ポイントライト数
	static const uint32_t kMaxSpotLights = 32;    // 最大スポットライト数
};