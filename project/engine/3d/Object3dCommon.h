#pragma once
#include "ResourceDataStructure.h"
#include "PipelineStateObject.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

class Camera;
class DirectXCommon;
class Object3dCommon {

public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

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
	void PreDrawForObject3d();
	void PreDrawForSkinningObject3d();

//================================================================================================
// 	   getter
//================================================================================================

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	Camera* GetDefaultCamera() const { return defaultCamera_; }

	PSO* GetPSO() const { return psoForObject3d_.get(); }

//================================================================================================
// 	   setter
//================================================================================================

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

private:

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

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
	PointLightData* pointLightData_ = nullptr;
	//スポットライトのリソース
	ComPtr<ID3D12Resource> spotLightResource_;
	SpotLightData* spotLightData_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;

	//PSO
	std::unique_ptr<PSO> psoForObject3d_ = nullptr;
	std::unique_ptr<PSO> psoForSkinningObject3d_ = nullptr;
	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignatureForObject3d_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignatureForSkinningObject3d_ = nullptr;

};

