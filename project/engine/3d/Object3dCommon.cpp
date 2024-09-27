#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"

Object3dCommon* Object3dCommon::instance_ = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Object3dCommon();
	}
	return instance_;
	
}

void Object3dCommon::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = new PSO();
	pso_->CreatePSO(dxCommon_->GetDevice(), dxCommon_->GetDXC(), D3D12_CULL_MODE_NONE); //PSO生成
	rootSignature_ = pso_->GetRootSignature();
}

void Object3dCommon::Finalize() {
	rootSignature_.Reset();
	dxCommon_ = nullptr;
	delete pso_;
	pso_ = nullptr;

	delete instance_;
	instance_ = nullptr;
}

void Object3dCommon::PreDraw() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
