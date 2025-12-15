#pragma once
#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/3d/Light/DirectionalLight.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Light/SpotLight.h"
#include "engine/3d/Light/LightCameraInfo.h"
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

class Camera;
class PSO;
namespace TakeC {

class DirectXCommon;
class SrvManager;

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
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

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

		void UpdateShadowMatrix(Camera* camera);

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

		ID3D12Resource* GetPointLightResource() const {
			return pointLightResource_.Get();
		}

		ID3D12Resource* GetSpotLightResource() const {
			return spotLightResource_.Get();
		}

		ID3D12Resource* GetLightCameraInfoResource() const {
			return lightCameraInfoResource_.Get();
		}

		const LightCameraInfo& GetLightCameraInfo() const {
			return *lightCameraInfo_;
		}

		//----- setter ---------------------------

		/// ライト用リソースの設定
		void SetLightResources(PSO* pso);
	private:

		//========================================================================
		//	private variables
		//========================================================================

		TakeC::DirectXCommon* dxCommon_ = nullptr;
		TakeC::SrvManager* srvManager_ = nullptr;

		ComPtr<ID3D12Resource> dirLightResource_;
		DirectionalLightData* dirLightData_ = nullptr;

		Camera* lightCamera_ = nullptr;
		ComPtr<ID3D12Resource> lightCameraInfoResource_;
		LightCameraInfo* lightCameraInfo_ = nullptr;

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

		static const uint32_t kMaxPointLights = 512;   // 最大ポイントライト数
		static const uint32_t kMaxSpotLights = 128;    // 最大スポットライト数

		float lightCameraDistance_ = 100.0f; // ライトカメラ距離
		float shadowRange_ = 1500.0f;        // シャドウマップ範囲
		float farClip_ = 2500.0f;             // ライトカメラ遠クリップ距離
		float nearClip_ = 1.0f;             // ライトカメラ近クリップ距離
	};
}