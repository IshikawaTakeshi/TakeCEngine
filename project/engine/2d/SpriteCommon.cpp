#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"

SpriteCommon* SpriteCommon::instance_ = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {	
	if(instance_ == nullptr) {
		instance_ = new SpriteCommon();
	}
	return instance_;
}

void SpriteCommon::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = std::make_unique<PSO>();
	pso_->CreatePSOForSprite(dxCommon_->GetDevice(),dxCommon_->GetDXC(), D3D12_CULL_MODE_NONE); //PSO生成
	rootSignature_ = pso_->GetRootSignature();
}

void SpriteCommon::Finalize() {
	rootSignature_.Reset();
	dxCommon_ = nullptr;
	pso_.reset();
	delete instance_;
	instance_ = nullptr;
}

void SpriteCommon::PreDraw() {

	
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

