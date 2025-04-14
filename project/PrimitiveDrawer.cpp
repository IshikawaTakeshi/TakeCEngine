#include "PrimitiveDrawer.h"

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;
	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/PrimitiveDrawer/PrimitiveDrawer.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/PrimitiveDrawer/PrimitiveDrawer.PS.hlsl");
	//PSOの生成
	pso_->CreateGraphicPSO(
		dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		PSO::BlendState::NORMAL,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);
	rootSignature_ = pso_->GetGraphicRootSignature();
}

void PrimitiveDrawer::Update() {

}