#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <memory>
#include "PipelineStateObject.h"

class Camera;
class DirectXCommon;
class SrvManager;
class ParticleCommon {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static ParticleCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	void UpdateImGui();

	void Finalize();

	void PreDraw();

	//================================================================================================
	// 	   getter
	//================================================================================================

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	SrvManager* GetSrvManager() const { return srvManager_; }

	PSO* GetPSO() const { return pso_.get(); }

	Camera* GetDefaultCamera() const { return defaultCamera_; }


	//================================================================================================
	// 	   setter
	//================================================================================================

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }


private:

	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

private:

	static ParticleCommon* instance_;

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<PSO> pso_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;
};

