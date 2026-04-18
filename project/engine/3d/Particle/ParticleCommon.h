#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include "engine/base/PipelineStateObject.h"
#include "engine/base/ComPtrAliasTemplates.h"

// 前方宣言
class Camera;
namespace TakeC {
class DirectXCommon;
class LightManager;
class SrvManager;
}

//============================================================================
// ParticleCommon class
//============================================================================
class ParticleCommon {
private:

	//コンストラクタ・デストラクタ・コピー禁止
	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static ParticleCommon& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager,TakeC::LightManager* lightManager);

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	//// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw(BlendState state);

	/// <summary>
	/// 描画前処理（GPUパーティクル用）
	/// </summary>
	void PreDrawForGPUParticle();

	/// <summary>
	/// パーティクル初期化ディスパッチ
	/// </summary>
	void DispatchForGPUParticle();

	/// <summary>
	/// パーティクル更新ディスパッチ
	/// </summary>
	void DispatchUpdateParticle();

public:

	//================================================================================================
	//	accessor
	//================================================================================================

	//----- getter ---------------------------

	//DirectXCommonの取得
	TakeC::DirectXCommon* GetDirectXCommon() const { return dxCommon_; }
	//SrvManagerの取得
	TakeC::SrvManager* GetSrvManager() const { return srvManager_; }
	//RootSignatureの取得
	PSO* GetGraphicPSO(BlendState state) const { return graphicPso_.at(state).get(); }
	//GPUパーティクル用PSOの取得
	PSO* GetGraphicPSOForGPUParticle() const { return graphicPsoForGPUParticle_.get(); }
	//GPUパーティクル初期化用PSOの取得
	PSO* GetComputePSOForGPUParticle() const { return computePsoForGPUParticle_.get(); }
	//カメラ情報の取得
	Camera* GetDefaultCamera() const { return defaultCamera_; }

	//----- setter ---------------------------
	
	//デフォルトカメラの設定
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }


private:

	//DirectXCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;
	//LightManager
	TakeC::LightManager* lightManager_ = nullptr;

	//PSO
	std::unordered_map<BlendState, std::unique_ptr<PSO>> graphicPso_;
	std::unique_ptr<PSO> graphicPsoForGPUParticle_ = nullptr;
	std::unique_ptr<PSO> computePsoForGPUParticle_ = nullptr;
	std::unique_ptr<PSO> psoUpdateParticle_ = nullptr;

	//SrvManager
	TakeC::SrvManager* srvManager_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> graphicRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> graphicRootSignatureForGPUParticle_ = nullptr;
	ComPtr<ID3D12RootSignature> computeRootSignatureForGPUParticle_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignatureUpdateParticle_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;
};