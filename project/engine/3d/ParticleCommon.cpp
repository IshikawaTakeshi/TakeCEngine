#include "ParticleCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

#include "CameraManager.h"

ParticleCommon* ParticleCommon::instance_ = nullptr;

ParticleCommon* ParticleCommon::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ParticleCommon();
	}
	return instance_;
}

void ParticleCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;

	srvManager_ = srvManager;

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/Particle.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/Particle.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(),D3D12_FILL_MODE_SOLID);

	rootSignature_ = pso_->GetGraphicRootSignature();
}

void ParticleCommon::UpdateImGui() {
	pso_->UpdateImGui();
}

void ParticleCommon::Finalize() {

	rootSignature_.Reset();
	pso_.reset();
}

void ParticleCommon::PreDraw() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
