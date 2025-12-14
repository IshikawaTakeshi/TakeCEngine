#pragma once
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

//前方宣言
class Camera;
namespace TakeC {
	class DirectXCommon;
	class LightManager;
}


//============================================================================
// Object3dCommon class
//============================================================================
class Object3dCommon {
private:

	//コンストラクタ・デストラクタ・コピー禁止
	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;


public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Object3dCommon& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::DirectXCommon* directXCommon,TakeC::LightManager* lightManager);

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 影描画前処理
	/// </summary>
	void PreDrawShadowPass();

	/// <summary>
	/// 加算ブレンド描画前処理
	/// </summary>
	void PreDrawAddBlend();

	/// <summary>
	/// スキニング計算
	/// </summary>
	void Dispatch();

public:
	//================================================================================================
	// accessors
	//================================================================================================

	//----- getter ---------------------------

	/// DirectXCommonの取得
	TakeC::DirectXCommon* GetDirectXCommon() const { return dxCommon_; }
	/// 平行光源データの取得
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	/// PSOの取得
	PSO* GetPSO() const { return pso_.get(); }
	/// 加算ブレンド用PSOの取得
	PSO* GetAddBlendPSO() const { return addBlendPso_.get(); }
	/// 影描画用PSOの取得
	PSO* GetShadowPassPSO() const { return shadowPassPso_.get(); }
	/// 平行光源リソースの取得
	ID3D12Resource* GetDirectionalLightResource() const;

	///----- setter ---------------------------

	/// デフォルトカメラの設定
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	/// 平行光源の方向の設定
	void SetDirectionalLightIntensity(const float intensity) { directionalLightData_->intensity_ = intensity; }

private:

	void SetCBufferViewCamera(PSO* pso);

	void SetCBufferViewLightCamera(PSO* pso);

private:

	//DirectXCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;
	//LightManager
	TakeC::LightManager* lightManager_ = nullptr;


	std::unique_ptr<DirectionalLightData> directionalLightData_ = nullptr;
	uint32_t pointLightIndex_ = 0;
	uint32_t spotLightIndex_ = 0;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;

	//PSO
	std::unique_ptr<PSO> pso_ = nullptr;
	//加算ブレンド用PSO
	std::unique_ptr<PSO> addBlendPso_ = nullptr;

	//影描画用PSO
	std::unique_ptr<PSO> shadowPassPso_ = nullptr;
	
	//RootSignature
	ComPtr<ID3D12RootSignature> graphicRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> computeRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> addBlendRootSignature_ = nullptr;
	//影描画用RootSignature
	ComPtr<ID3D12RootSignature> shadowPassRootSignature_ = nullptr;
};

