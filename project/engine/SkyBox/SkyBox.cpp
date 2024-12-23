#include "SkyBox.h"

void SkyBox::Initialize(DirectXCommon* directXCommon,const std::string& filename) {

	dxCommon_ = directXCommon;

	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CreatePSO(directXCommon, directXCommon->GetDXC(), D3D12_FILL_MODE_SOLID);
}
