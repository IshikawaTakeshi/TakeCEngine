#include "SpriteCommon.h"
#include "../Include/DirectXCommon.h"
#include "../Include/PipelineStateObject.h"

void SpriteCommon::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = new PSO();
	pso_->CreatePSO(dxCommon_->GetDevice(),dxCommon_->GetDXC());
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

