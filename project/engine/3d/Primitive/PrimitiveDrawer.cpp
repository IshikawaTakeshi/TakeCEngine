#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include <numbers>

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/Object3d.PS.hlsl");
	//PSOの生成
	pso_->CreateGraphicPSO(
		dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		PSO::BlendState::NORMAL,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);
	rootSignature_ = pso_->GetGraphicRootSignature();

	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	ringData_ = new RingData();
	CreateRingVertexData();
}

void PrimitiveDrawer::Finalize() {

	if (ringData_) {
		ringData_->primitiveData_.vertexResource_->Release();
		delete ringData_;
		ringData_ = nullptr;
	}

	wvpResource_->Release();

	rootSignature_ = nullptr;
	pso_.reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PrimitiveDrawer::Update() {

	TransformMatrixData_->WVP = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
}

void PrimitiveDrawer::DrawRing(const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color) {

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringDivide_);

	for (uint32_t index = 0; index < ringDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringDivide_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide_);

		ringData_->vertexData_[ringVertexIndex_].position = { center.x + outerRadius * cos, center.y + outerRadius * sin, center.z };
		ringData_->vertexData_[ringVertexIndex_ + 1].position = { center.x + innerRadius * cosNext, center.y + innerRadius * sinNext, center.z };
		ringData_->vertexData_[ringVertexIndex_ + 2].position = { center.x + innerRadius * cos, center.y + innerRadius * sin, center.z };
		ringData_->vertexData_[ringVertexIndex_ + 3].position = { center.x + outerRadius * cosNext, center.y + outerRadius * sinNext, center.z };

		ringData_->vertexData_[ringVertexIndex_].texcoord = { uv, 0.0f };
		ringData_->vertexData_[ringVertexIndex_ + 1].texcoord = { uvNext, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 2].texcoord = { uv, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 3].texcoord = { uvNext, 0.0f };

		ringData_->vertexData_[ringVertexIndex_].normal = { cos, sin, 0.0f };
		ringData_->vertexData_[ringVertexIndex_ + 1].normal = { cosNext, sinNext, 0.0f };
		ringData_->vertexData_[ringVertexIndex_ + 2].normal = { cos, sin, 0.0f };
		ringData_->vertexData_[ringVertexIndex_ + 3].normal = { cosNext, sinNext, 0.0f };

		ringVertexIndex_ += 4;
	}

	ringData_->material_->SetMaterialColor(color);
}

void PrimitiveDrawer::Draw() {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//--------------------------------------------------
	//		ringの描画
	//--------------------------------------------------

	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	commandList->SetPipelineState(pso_->GetGraphicPipelineState());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VBVを設定
	commandList->IASetVertexBuffers(0, 1, &ringData_->primitiveData_.vertexBufferView_);
	//0.wvpResource
	commandList->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());
	//1.materialResource
	commandList->SetGraphicsRootConstantBufferView(1, ringData_->material_->GetMaterialResource()->GetGPUVirtualAddress());
	//2.textureSRV
	srvManager_->SetGraphicsRootDescriptorTable(6, TextureManager::GetInstance()->GetSrvIndex(ringData_->material_->GetTextureFilePath()));
	//描画
	commandList->DrawInstanced(ringVertexIndex_, ringVertexIndex_ / ringVertexCount_, 0, 0);

}

void PrimitiveDrawer::CreateRingVertexData() {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData_->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * ringDivide_ * kMaxVertexCount_);
	//bufferview設定
	ringData_->primitiveData_.vertexBufferView_.BufferLocation = ringData_->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	ringData_->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData_->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData_->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&ringData_->vertexData_));

	ringData_->material_ = new Material();
	ringData_->material_->Initialize(dxCommon_, "Resources/images/white.png");
}
