#pragma once
#include "ResourceDataStructure.h"
#include "engine/base/PipelineStateObject.h"
#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/3d/Light/DirectionalLight.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Light/SpotLight.h"
#include "engine/3d/Light/LightCounter.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

class Camera;
class DirectXCommon;
class Object3dCommon {

public:

	static Object3dCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon);

	void UpdateImGui();

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	void PreDrawAddBlend();

	void Dispatch();
//================================================================================================
// 	   getter
//================================================================================================

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	Camera* GetDefaultCamera() const { return defaultCamera_; }

	PSO* GetPSO() const { return pso_.get(); }

//================================================================================================
// 	   setter
//================================================================================================

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	void SetDirectionalLightIntensity(const float intensity) { directionalLightData_->intensity_ = intensity; }

private:

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

	void SetGraphicCBufferViewLighting(PSO* pso);

	void SetCBufferViewCamera(PSO* pso);

private:

	//インスタンス
	static Object3dCommon* instance_;

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//平行光源用のリソース
	ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;
	//ポイントライトのリソース
	ComPtr<ID3D12Resource> pointLightResource_;
	LightCounter<PointLightData> pointLightCounter_;
	PointLightData* pointLightData_ = nullptr;

	//スポットライトのリソース
	ComPtr<ID3D12Resource> spotLightResource_;
	LightCounter<SpotLightData> spotLightCounter_;
	SpotLightData* spotLightData_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;

	//PSO
	std::unique_ptr<PSO> pso_ = nullptr;
	//
	std::unique_ptr<PSO> addBlendPso_ = nullptr;
	
	//RootSignature
	ComPtr<ID3D12RootSignature> graphicRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> computeRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> addBlendRootSignature_ = nullptr;
};

