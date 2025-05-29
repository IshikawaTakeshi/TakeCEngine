#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ImGuiManager.h"
#include "MatrixMath.h"
#include "Particle/Particle3d.h"
#include <numbers>

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

}

void PrimitiveDrawer::Finalize() {

	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PrimitiveDrawer::Update() {

}

void PrimitiveDrawer::UpdateImGui() {

	ImGui::Begin("PrimitiveDrawer");

	ImGui::End();
}

uint32_t PrimitiveDrawer::GenerateRing(const float outerRadius, const float innerRadius, const std::string& textureFilePath) {

	auto ring = std::make_unique<RingData>();
	ring->innerRadius_ = innerRadius;
	ring->outerRadius_ = outerRadius;
	CreateRingVertexData(ring.get());
	CreateRingMaterial(textureFilePath, ring.get());


	uint32_t useHandle = ringHandle_++;
	ringDatas_[useHandle] = std::move(ring);
	return useHandle;
}

uint32_t PrimitiveDrawer::GeneratePlane(const float width, const float height, const std::string& textureFilePath) {

	auto plane = std::make_unique<PlaneData>();
	plane->width_ = width;
	plane->height_ = height;
	CreatePlaneVertexData(plane.get());
	CreatePlaneMaterial(textureFilePath, plane.get());

	uint32_t useHandle = planeHandle_++;
	planeDatas_[useHandle] = std::move(plane);
	return useHandle;
}

void PrimitiveDrawer::DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();


	switch (type) {
	case PRIMITIVE_RING:
	{

		//--------------------------------------------------
		//		ringの描画
		//--------------------------------------------------

		auto itRing = ringDatas_.find(handle);
		if (itRing == ringDatas_.end()) {
			return; // handleが無効な場合は何もしない
		}

		auto& ringData = itRing->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &ringData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), ringData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(ringData->material_->GetTextureFilePath()));

		//描画
		commandList->DrawInstanced(ringVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_PLANE:
	{

		//--------------------------------------------------
		//		planeの描画
		//--------------------------------------------------

		auto itPlane = planeDatas_.find(handle);
		if (itPlane == planeDatas_.end()) return;

		auto& planeData_ = itPlane->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &planeData_->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), planeData_->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData_->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(planeVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::CreateRingVertexData(RingData* ringData) {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	ringData->primitiveData_.vertexResource_->SetName(L"Ring::vertexResource_");
	//bufferview設定
	ringData->primitiveData_.vertexBufferView_.BufferLocation = ringData->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	ringData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&ringData->vertexData_));

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringDivide_);
	uint32_t ringVertexIndex = 0;

	for (uint32_t index = 0; index < ringDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringDivide_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide_);

		ringData->vertexData_[ringVertexIndex].position = { ringData->outerRadius_ * -sin,ringData->outerRadius_ * cos, 0.0f, 1.0f }; // outer[i]
		ringData->vertexData_[ringVertexIndex + 1].position = { ringData->innerRadius_ * -sin,     ringData->innerRadius_ * cos,    0.0f, 1.0f }; // inner[i]
		ringData->vertexData_[ringVertexIndex + 2].position = { ringData->innerRadius_ * -sinNext, ringData->innerRadius_ * cosNext,0.0f, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 3].position = ringData->vertexData_[ringVertexIndex].position; // outer[i]
		ringData->vertexData_[ringVertexIndex + 4].position = ringData->vertexData_[ringVertexIndex + 2].position; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 5].position = { ringData->outerRadius_ * -sinNext, ringData->outerRadius_ * cosNext, 0.0f, 1.0f }; // outer[i+1]

		ringData->vertexData_[ringVertexIndex].texcoord = { uv, 0.0f };     // outer[i]
		ringData->vertexData_[ringVertexIndex + 1].texcoord = { uv, 1.0f };     // inner[i]
		ringData->vertexData_[ringVertexIndex + 2].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 3].texcoord = { uv, 0.0f };     // outer[i]
		ringData->vertexData_[ringVertexIndex + 4].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 5].texcoord = { uvNext, 0.0f }; // outer[i+1]

		ringData->vertexData_[ringVertexIndex].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 1].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 2].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 3].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 4].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 5].normal = { 0.0f, 0.0f, 1.0f };

		ringVertexIndex += 6;
	}

	ringVertexCount_ += ringDivide_ * 6;
}

void PrimitiveDrawer::CreatePlaneVertexData(PlaneData* planeData) {

	UINT size = sizeof(VertexData) * 6 * kMaxVertexCount_;
	//bufferをカウント分確保
	planeData->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	planeData->primitiveData_.vertexResource_->SetName(L"Plane::vertexResource_");
	//bufferview設定
	planeData->primitiveData_.vertexBufferView_.BufferLocation = planeData->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	planeData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	planeData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	planeData->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&planeData->vertexData_));

	//Planeの頂点データを生成(6頂点分)
	planeData->vertexData_[0].position = { -planeData->width_,  planeData->height_,0.0f, 1.0f }; //左下
	planeData->vertexData_[1].position = { -planeData->width_, -planeData->height_,0.0f, 1.0f }; //左上
	planeData->vertexData_[2].position = { planeData->width_,  planeData->height_,0.0f, 1.0f }; //右下
	planeData->vertexData_[3].position = { -planeData->width_, -planeData->height_,0.0f, 1.0f }; //左上
	planeData->vertexData_[4].position = { planeData->width_, -planeData->height_,0.0f, 1.0f }; //右上
	planeData->vertexData_[5].position = { planeData->width_,  planeData->height_,0.0f, 1.0f }; //右下
	//PlaneのUV座標を生成
	planeData->vertexData_[0].texcoord = { 0.0f, 1.0f };
	planeData->vertexData_[1].texcoord = { 0.0f, 0.0f };
	planeData->vertexData_[2].texcoord = { 1.0f, 1.0f };
	planeData->vertexData_[3].texcoord = { 0.0f, 0.0f };
	planeData->vertexData_[4].texcoord = { 1.0f, 0.0f };
	planeData->vertexData_[5].texcoord = { 1.0f, 1.0f };
	//Planeの法線を生成
	planeData->vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[4].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[5].normal = { 0.0f, 0.0f, -1.0f };

	//Planeの頂点の
	planeVertexCount_ += 6;
}

void PrimitiveDrawer::CreateRingMaterial(const std::string& textureFilePath, RingData* ringData) {

	ringData->material_ = new Material();
	ringData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	ringData->material_->SetEnableLighting(false);
	ringData->material_->SetEnvCoefficient(0.0f);
	ringData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
	ringData->material_->SetUvScale({ 2.5f, 0.5f,1.0f }); // UVスケールを設定
}

void PrimitiveDrawer::CreatePlaneMaterial(const std::string& textureFilePath, PlaneData* planeData) {

	planeData->material_ = new Material();
	planeData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	planeData->material_->SetEnableLighting(false);
	planeData->material_->SetEnvCoefficient(0.0f);
	planeData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}