#include "ParticleCommon.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "engine/camera/CameraManager.h"
#include "engine/Utility/StringUtility.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Light/LightManager.h"


//==================================================================================
// インスタンス取得
//==================================================================================
ParticleCommon& ParticleCommon::GetInstance() {

	static ParticleCommon instance;
	return instance;
}

//==================================================================================
// 初期化
//==================================================================================
void ParticleCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,LightManager* lightManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	lightManager_ = lightManager;

	//各ブレンドステート用PSO生成
	for (int i = 0; i < int(BlendState::COUNT); i++) {
		auto pso = std::make_unique<PSO>();
		pso->CompileVertexShader(dxCommon_->GetDXC(), L"Particle.VS.hlsl");
		pso->CompilePixelShader(dxCommon_->GetDXC(), L"Particle.PS.hlsl");
		pso->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO, static_cast<BlendState>(i));
		
		std::string blendStateName = StringUtility::EnumToString(static_cast<BlendState>(i));
		pso->SetGraphicPipelineName("graphicPSO_" + blendStateName);
		graphicPso_[static_cast<BlendState>(i)] = std::move(pso);
	}

	//ルートシグネチャをBlendState::NORMALのものを共通で使う
	graphicRootSignature_ = graphicPso_[BlendState::NORMAL]->GetGraphicRootSignature();

	//GPUパーティクル用PSO生成
	graphicPsoForGPUParticle_ = std::make_unique<PSO>();
	graphicPsoForGPUParticle_->CompileVertexShader(dxCommon_->GetDXC(), L"GPUParticle.VS.hlsl");
	graphicPsoForGPUParticle_->CompilePixelShader(dxCommon_->GetDXC(), L"GPUParticle.PS.hlsl");
	graphicPsoForGPUParticle_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO,BlendState::ADD);
	graphicPsoForGPUParticle_->SetGraphicPipelineName("GPUParticlePSO");
	graphicRootSignatureForGPUParticle_ = graphicPsoForGPUParticle_->GetGraphicRootSignature();

	//particle初期化用PSO
	computePsoForGPUParticle_ = std::make_unique<PSO>();
	computePsoForGPUParticle_->CompileComputeShader(dxCommon_->GetDXC(), L"InitializeParticle.CS.hlsl");
	computePsoForGPUParticle_->CreateComputePSO(dxCommon_->GetDevice());
	computePsoForGPUParticle_->SetComputePipelineName("InitializeParticlePSO");
	computeRootSignatureForGPUParticle_ = computePsoForGPUParticle_->GetComputeRootSignature();

	//update用PSO
	psoUpdateParticle_ = std::make_unique<PSO>();
	psoUpdateParticle_->CompileComputeShader(dxCommon_->GetDXC(), L"UpdateParticle.CS.hlsl");
	psoUpdateParticle_->CreateComputePSO(dxCommon_->GetDevice());
	psoUpdateParticle_->SetComputePipelineName("UpdateParticlePSO");
	rootSignatureUpdateParticle_ = psoUpdateParticle_->GetComputeRootSignature();
}

//==================================================================================
// ImGuiの更新
//==================================================================================
void ParticleCommon::UpdateImGui() {
	//graphicPso_->UpdateImGui();
	graphicPsoForGPUParticle_->UpdateImGui();
}

//==================================================================================
// 終了・開放処理
//==================================================================================
void ParticleCommon::Finalize() {

	graphicRootSignature_.Reset();
	for(auto& [key, pso] : graphicPso_) {
		pso.reset();
	}

	graphicRootSignatureForGPUParticle_.Reset();
	graphicPsoForGPUParticle_.reset();

	computeRootSignatureForGPUParticle_.Reset();
	computePsoForGPUParticle_.reset();

	rootSignatureUpdateParticle_.Reset();
	psoUpdateParticle_.reset();
}

//==================================================================================
// 描画前処理
//==================================================================================
void ParticleCommon::PreDraw(BlendState state) {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicPso_[state]->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ライト情報セット
	lightManager_->SetLightResources(graphicPso_[state].get());
}

//==================================================================================
// GPUパーティクル描画前処理
//==================================================================================
void ParticleCommon::PreDrawForGPUParticle() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignatureForGPUParticle_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicPsoForGPUParticle_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//==================================================================================
// GPUパーティクル初期化用ディスパッチ
//==================================================================================
void ParticleCommon::DispatchForGPUParticle() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(computePsoForGPUParticle_->GetComputePipelineState());

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignatureForGPUParticle_.Get());
}

//==================================================================================
// GPUパーティクル更新用ディスパッチ
//==================================================================================
void ParticleCommon::DispatchUpdateParticle() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(psoUpdateParticle_->GetComputePipelineState());

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureUpdateParticle_.Get());
}
