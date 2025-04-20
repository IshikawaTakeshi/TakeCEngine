#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ImGuiManager.h"
#include "MatrixMath.h"
#include "Particle/Particle3d.h"
#include <numbers>

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	ringData_ = new RingData();
}

void PrimitiveDrawer::Finalize() {

	if (ringData_) {
		ringData_->primitiveData_.vertexResource_.Reset();
		delete ringData_;
		ringData_ = nullptr;
	}

	//wvpResource_.Reset();

	//rootSignature_.Reset();
	//pso_.reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PrimitiveDrawer::Update() {

	
}

void PrimitiveDrawer::UpdateImGui() {

	ImGui::Begin("PrimitiveDrawer");
	ImGui::DragFloat3("RingScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("RingRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("RingTranslate", &transform_.translate.x, 0.01f);
	ringData_->material_->UpdateMaterialImGui();
	ImGui::End();
}

void PrimitiveDrawer::GenerateRing(const float outerRadius, const float innerRadius) {

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringDivide_);
	uint32_t ringVertexIndex = 0;

	for (uint32_t index = 0; index < ringDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringDivide_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide_);

		ringData_->vertexData_[ringVertexIndex].position = {outerRadius * -sin,outerRadius * cos, 0.0f, 1.0f }; // outer[i]
		ringData_->vertexData_[ringVertexIndex + 1].position = {innerRadius * -sin,     innerRadius * cos,    0.0f, 1.0f }; // inner[i]
		ringData_->vertexData_[ringVertexIndex + 2].position = {innerRadius * -sinNext, innerRadius * cosNext,0.0f, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex + 3].position = ringData_->vertexData_[ringVertexIndex].position; // outer[i]
		ringData_->vertexData_[ringVertexIndex + 4].position = ringData_->vertexData_[ringVertexIndex + 2].position; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex + 5].position = { outerRadius * -sinNext, outerRadius * cosNext, 0.0f, 1.0f }; // outer[i+1]

		ringData_->vertexData_[ringVertexIndex].texcoord = { uv, 0.0f };     // outer[i]
		ringData_->vertexData_[ringVertexIndex + 1].texcoord = { uv, 1.0f };     // inner[i]
		ringData_->vertexData_[ringVertexIndex + 2].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex + 3].texcoord = { uv, 0.0f };     // outer[i]
		ringData_->vertexData_[ringVertexIndex + 4].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex + 5].texcoord = { uvNext, 0.0f }; // outer[i+1]

		ringData_->vertexData_[ringVertexIndex].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex + 1].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex + 2].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex + 3].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex + 4].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex + 5].normal = { 0.0f, 0.0f, 1.0f };

		ringVertexIndex += 6;
	}

	ringInstanceIndex_ += 1;
}

void PrimitiveDrawer::DrawParticle(PSO* pso,UINT instanceCount) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//--------------------------------------------------
	//		ringの描画
	//--------------------------------------------------

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VertexBufferView
	commandList->IASetVertexBuffers(0, 1, &ringData_->primitiveData_.vertexBufferView_);
	// materialResource
	commandList->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gMaterial"), ringData_->material_->GetMaterialResource()->GetGPUVirtualAddress());
	// texture
	srvManager_->SetGraphicsRootDescriptorTable(pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(ringData_->material_->GetTextureFilePath()));
	
	//描画
	commandList->DrawInstanced(ringVertexCount_, instanceCount, 0, 0);
}

void PrimitiveDrawer::CreateRingVertexData() {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData_->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	//bufferview設定
	ringData_->primitiveData_.vertexBufferView_.BufferLocation = ringData_->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	ringData_->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData_->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData_->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&ringData_->vertexData_));

	GenerateRing(1.0f, 0.1f);

	ringData_->material_ = new Material();
	ringData_->material_->Initialize(dxCommon_, "Resources/images/gradationLine.png");
	ringData_->material_->SetEnableLighting(false);
	ringData_->material_->SetEnvCoefficient(0.0f);

	ringData_->material_->SetMaterialColor({1.0f,1.0f,1.0f,1.0f});
}