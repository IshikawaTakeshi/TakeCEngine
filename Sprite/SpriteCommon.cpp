#include "SpriteCommon.h"
#include "../Include/DirectXCommon.h"
#include "../Include/PipelineStateObject.h"

SpriteCommon* SpriteCommon::instance_ = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {	
	if(instance_ == nullptr) {
		instance_ = new SpriteCommon();
	}
	return instance_;
}

void SpriteCommon::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = new PSO();
	pso_->CreatePSO(dxCommon_->GetDevice(),dxCommon_->GetDXC(), D3D12_CULL_MODE_NONE); //PSO生成
}

void SpriteCommon::Finalize() {

	dxCommon_ = nullptr;
	delete pso_;
	pso_ = nullptr;
	delete instance_;
	instance_ = nullptr;

}

void SpriteCommon::PreDraw() {

	rootSignature_ = pso_->GetRootSignature();
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

