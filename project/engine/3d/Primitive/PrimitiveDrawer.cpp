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
	planeData_ = new PlaneData();
}

void PrimitiveDrawer::Finalize() {

	if (ringData_) {
		ringData_->primitiveData_.vertexResource_.Reset();
		delete ringData_;
		ringData_ = nullptr;
	}

	if (planeData_) {
		planeData_->primitiveData_.vertexResource_.Reset();
		delete planeData_;
		planeData_ = nullptr;
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
	if (ringInstanceIndex_ != 0) {
		ImGui::Text("Ring Instance Count : %d", ringInstanceIndex_);
	} else {
		ImGui::Text("Ring Instance Count : 0");
	}
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

void PrimitiveDrawer::GeneratePlane(const float width, const float height) {
	
	//Planeの頂点データを生成(6頂点分)
	planeData_->vertexData_[0].position = { -width,  height,0.0f, 1.0f }; //左下
	planeData_->vertexData_[1].position = { -width, -height,0.0f, 1.0f }; //左上
	planeData_->vertexData_[2].position = {  width,  height,0.0f, 1.0f }; //右下
	planeData_->vertexData_[3].position = { -width, -height,0.0f, 1.0f }; //左上
	planeData_->vertexData_[4].position = {  width, -height,0.0f, 1.0f }; //右上
	planeData_->vertexData_[5].position = {  width,  height,0.0f, 1.0f }; //右下
	//PlaneのUV座標を生成
	planeData_->vertexData_[0].texcoord = { 0.0f, 1.0f };
	planeData_->vertexData_[1].texcoord = { 0.0f, 0.0f };
	planeData_->vertexData_[2].texcoord = { 1.0f, 1.0f };
	planeData_->vertexData_[3].texcoord = { 0.0f, 0.0f };
	planeData_->vertexData_[4].texcoord = { 1.0f, 0.0f };
	planeData_->vertexData_[5].texcoord = { 1.0f, 1.0f };
	//Planeの法線を生成
	planeData_->vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	planeData_->vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	planeData_->vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	planeData_->vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };
	planeData_->vertexData_[4].normal = { 0.0f, 0.0f, -1.0f };
	planeData_->vertexData_[5].normal = { 0.0f, 0.0f, -1.0f };

	//Planeの頂点の
	planeVertexCount_ += 6;
}

void PrimitiveDrawer::DrawParticle(PSO* pso,UINT instanceCount) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//--------------------------------------------------
	//		ringの描画
	//--------------------------------------------------

	if (ringInstanceIndex_ != 0) {

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

	//--------------------------------------------------
	//		planeの描画
	//--------------------------------------------------

	if (planeVertexCount_ != 0) {

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &planeData_->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gMaterial"), planeData_->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData_->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(planeVertexCount_, instanceCount, 0, 0);
	}
}

void PrimitiveDrawer::CreateVertexData(PrimitiveType type) {

	switch (type) {
	case PrimitiveType::PRIMITIVE_RING:
		CreateRingVertexData();
		break;
	case PrimitiveType::PRIMITIVE_PLANE:
		CreatePlaneVertexData();
		break;
	default:
		break;
	}
}


void PrimitiveDrawer::CreateRingVertexData() {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData_->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	ringData_->primitiveData_.vertexResource_->SetName(L"Ring::vertexResource_");
	//bufferview設定
	ringData_->primitiveData_.vertexBufferView_.BufferLocation = ringData_->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	ringData_->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData_->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData_->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&ringData_->vertexData_));

	GenerateRing(1.0f, 0.1f);
}

void PrimitiveDrawer::CreatePlaneVertexData() {

	UINT size = sizeof(VertexData) * 6;
	//bufferをカウント分確保
	planeData_->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	planeData_->primitiveData_.vertexResource_->SetName(L"Plane::vertexResource_");
	//bufferview設定
	planeData_->primitiveData_.vertexBufferView_.BufferLocation = planeData_->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	planeData_->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	planeData_->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	planeData_->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&planeData_->vertexData_));

	GeneratePlane(1.0f, 1.0f);
}

void PrimitiveDrawer::CreateMatrialData(PrimitiveType type, const std::string& textureFilePath) {

	switch (type) {
	case PrimitiveType::PRIMITIVE_RING:
		CreateRingMaterial(textureFilePath);
		break;
	case PrimitiveType::PRIMITIVE_PLANE:
		CreatePlaneMaterial(textureFilePath);
		break;
	default:
		break;
	}
}

void PrimitiveDrawer::CreateRingMaterial(const std::string& textureFilePath) {

	ringData_->material_ = new Material();
	ringData_->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	ringData_->material_->SetEnableLighting(false);
	ringData_->material_->SetEnvCoefficient(0.0f);
	ringData_->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::CreatePlaneMaterial(const std::string& textureFilePath) {

	planeData_->material_ = new Material();
	planeData_->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	planeData_->material_->SetEnableLighting(false);
	planeData_->material_->SetEnvCoefficient(0.0f);
	planeData_->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}
