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

	// 各プリミティブデータの解放
	for (auto& pair : ringDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	for (auto& pair : planeDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	for (auto& pair : sphereDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	for (auto& pair : coneDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	ringDatas_.clear();
	planeDatas_.clear();
	sphereDatas_.clear();
}

void PrimitiveDrawer::Update() {

}

void PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		auto it = ringDatas_.find(handle);
		if (it != ringDatas_.end()) {
			auto& ringData = it->second;
			ImGui::Begin("Ring Data");
			ImGui::Text("Outer Radius: %.2f", ringData->outerRadius_);
			ImGui::Text("Inner Radius: %.2f", ringData->innerRadius_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_PLANE:
	{
		auto it = planeDatas_.find(handle);
		if (it != planeDatas_.end()) {
			auto& planeData = it->second;
			ImGui::Begin("Plane Data");
			ImGui::Text("Width: %.2f", planeData->width_);
			ImGui::Text("Height: %.2f", planeData->height_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		auto it = sphereDatas_.find(handle);
		if (it != sphereDatas_.end()) {
			auto& sphereData = it->second;
			ImGui::Begin("Sphere Data");
			ImGui::Text("Radius: %.2f", sphereData->radius_);
			ImGui::End();
		}
		break;
	}
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		auto it = ringDatas_.find(handle);
		if (it != ringDatas_.end()) {
			auto& ringData = it->second;
			ringData->outerRadius_ = param.x;
			ringData->innerRadius_ = param.y;
			ImGui::Begin("Ring Data");
			ImGui::Text("Outer Radius: %.2f", ringData->outerRadius_);
			ImGui::Text("Inner Radius: %.2f", ringData->innerRadius_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_PLANE:
	{
		auto it = planeDatas_.find(handle);
		if (it != planeDatas_.end()) {
			auto& planeData = it->second;
			planeData->width_ = param.x;
			planeData->height_ = param.y;
			ImGui::Begin("Plane Data");
			ImGui::Text("Width: %.2f", planeData->width_);
			ImGui::Text("Height: %.2f", planeData->height_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		auto it = sphereDatas_.find(handle);
		if (it != sphereDatas_.end()) {
			auto& sphereData = it->second;
			sphereData->radius_ = param.x;
			ImGui::Begin("Sphere Data");
			ImGui::Text("Radius: %.2f", sphereData->radius_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_CONE:
	{
		auto it = coneDatas_.find(handle);
		if (it != coneDatas_.end()) {
			auto& coneData = it->second;
			coneData->radius_ = param.x;
			coneData->height_ = param.y;
			ImGui::Begin("Cone Data");
			ImGui::Text("Radius: %.2f", coneData->radius_);
			ImGui::Text("Height: %.2f", coneData->height_);
			ImGui::End();
		}
		break;
	}
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
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

uint32_t PrimitiveDrawer::GenerateSphere(const float radius, const std::string& textureFilePath) {

	auto sphere = std::make_unique<SphereData>();
	sphere->radius_ = radius;
	CreateSphereVertexData(sphere.get());
	CreateSphereMaterial(textureFilePath, sphere.get());
	uint32_t useHandle = sphereHandle_++;
	sphereDatas_[useHandle] = std::move(sphere);
	return useHandle;
}

uint32_t PrimitiveDrawer::GenerateCone(const float radius, const float height, uint32_t subDivision, const std::string& textureFilePath) {

	auto cone = std::make_unique<ConeData>();
	cone->radius_ = radius;
	cone->height_ = height;
	cone->subDivision_ = subDivision;
	CreateConeVertexData(cone.get());
	CreateConeMaterial(textureFilePath, cone.get());
	uint32_t useHandle = coneHandle_++;
	coneDatas_[useHandle] = std::move(cone);
	return useHandle;
}

//=================================================================================
//	描画処理
//=================================================================================

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

		auto& planeData = itPlane->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &planeData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), planeData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(planeVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_SPHERE:
	{
		//--------------------------------------------------
		//		sphereの描画
		//--------------------------------------------------

		auto itSphere = sphereDatas_.find(handle);
		if (itSphere == sphereDatas_.end()) return;

		auto& sphereData = itSphere->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &sphereData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), sphereData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(sphereData->material_->GetTextureFilePath()));

		//IBVの設定
		//commandList->IASetIndexBuffer(&sphereData->primitiveData_.indexBufferView_);

		//描画
		commandList->DrawInstanced(sphereVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_CONE:
	{
		//--------------------------------------------------
		//		coneの描画
		//--------------------------------------------------
		auto itCone = coneDatas_.find(handle);
		if (itCone == coneDatas_.end()) return;
		auto& coneData = itCone->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &coneData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), coneData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(coneData->material_->GetTextureFilePath()));
		//IBVの設定
		//commandList->IASetIndexBuffer(&coneData->primitiveData_.indexBufferView_);
		//描画
		commandList->DrawInstanced(coneVertexCount_, instanceCount, 0, 0);
		break;
	}

	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::DrawAllObject(PSO* pso, PrimitiveType type, uint32_t handle) {


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
		commandList->DrawInstanced(ringVertexCount_, 1, 0, 0);
		break;
	}
	case PRIMITIVE_PLANE:
	{
		//--------------------------------------------------
		//		planeの描画
		//--------------------------------------------------
		auto itPlane = planeDatas_.find(handle);
		if (itPlane == planeDatas_.end()) return;
		auto& planeData = itPlane->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &planeData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), planeData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(planeVertexCount_, 1, 0, 0);
		break;

	}
	case PRIMITIVE_SPHERE:
	{
		//--------------------------------------------------
		//		sphereの描画
		//--------------------------------------------------
		auto itSphere = sphereDatas_.find(handle);
		if (itSphere == sphereDatas_.end()) return;
		auto& sphereData = itSphere->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &sphereData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), sphereData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(sphereData->material_->GetTextureFilePath()));
		//IBVの設定
		//commandList->IASetIndexBuffer(&sphereData->primitiveData_.indexBufferView_);
		//描画
		commandList->DrawInstanced(sphereVertexCount_, 1, 0, 0);
		break;
	}
	case PRIMITIVE_CONE:
	{
		//--------------------------------------------------
		//		coneの描画
		//--------------------------------------------------
		auto itCone = coneDatas_.find(handle);
		if (itCone == coneDatas_.end()) return;
		auto& coneData = itCone->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &coneData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), coneData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(coneData->material_->GetTextureFilePath()));
		//IBVの設定
		//commandList->IASetIndexBuffer(&coneData->primitiveData_.indexBufferView_);
		//描画
		commandList->DrawInstanced(coneVertexCount_, 1, 0, 0);
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

	UINT size = sizeof(VertexData) * ringData->subDivision_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	ringData->primitiveData_.vertexBuffer_->SetName(L"Ring::vertexResource_");
	//bufferview設定
	ringData->primitiveData_.vertexBufferView_.BufferLocation = ringData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	ringData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&ringData->vertexData_));

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringData->subDivision_);
	uint32_t ringVertexIndex = 0;

	for (uint32_t index = 0; index < ringData->subDivision_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringData->subDivision_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringData->subDivision_);

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

	ringVertexCount_ += ringData->subDivision_ * 6;
}

void PrimitiveDrawer::CreatePlaneVertexData(PlaneData* planeData) {

	UINT size = sizeof(VertexData) * 6 * kMaxVertexCount_;
	//bufferをカウント分確保
	planeData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	planeData->primitiveData_.vertexBuffer_->SetName(L"Plane::vertexResource_");
	//bufferview設定
	planeData->primitiveData_.vertexBufferView_.BufferLocation = planeData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	planeData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	planeData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	planeData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&planeData->vertexData_));

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

void PrimitiveDrawer::CreateSphereVertexData(SphereData* sphereData) {

	auto CreateSphereVertex = [&](const Vector3& pos, uint32_t lonIndex, uint32_t latIndex, uint32_t kSubdivision, float radius) {
		VertexData v{};
		v.position = { pos.x, pos.y, pos.z, 1.0f };
		v.normal = { pos.x / radius, pos.y / radius, pos.z / radius };
		v.texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
		v.texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
		return v;
		};

	const uint32_t kSubdivision = sphereData->subDivision_; // 分割数
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision);         // 緯度刻み
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);  // 経度刻み

	// 頂点数 = 1マス6頂点 × マス数
	uint32_t vertexCount = kSubdivision * kSubdivision * 6;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	sphereData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	sphereData->primitiveData_.vertexBuffer_->SetName(L"Sphere::vertexResource_");

	// bufferview設定
	sphereData->primitiveData_.vertexBufferView_.BufferLocation = sphereData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	sphereData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	sphereData->primitiveData_.vertexBufferView_.SizeInBytes = size;

	// mapping
	sphereData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&sphereData->vertexData_));

	VertexData* vertexData = sphereData->vertexData_;
	uint32_t vertexIndex = 0;

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			// 各点の緯度・経度
			float latNext = lat + kLatEvery;
			float lonNext = lon + kLonEvery;

			// a: 左下
			Vector3 aPos = {
				sphereData->radius_ * cosf(lat) * cosf(lon),
				sphereData->radius_ * sinf(lat),
				sphereData->radius_ * cosf(lat) * sinf(lon)
			};

			// b: 左上
			Vector3 bPos = {
				sphereData->radius_ * cosf(latNext) * cosf(lon),
				sphereData->radius_ * sinf(latNext),
				sphereData->radius_ * cosf(latNext) * sinf(lon)
			};

			// c: 右下
			Vector3 cPos = {
				sphereData->radius_ * cosf(lat) * cosf(lonNext),
				sphereData->radius_ * sinf(lat),
				sphereData->radius_ * cosf(lat) * sinf(lonNext)
			};

			// d: 右上
			Vector3 dPos = {
				sphereData->radius_ * cosf(latNext) * cosf(lonNext),
				sphereData->radius_ * sinf(latNext),
				sphereData->radius_ * cosf(latNext) * sinf(lonNext)
			};

			// 三角形1 (a, b, c)
			vertexData[vertexIndex++] = CreateSphereVertex(aPos, lonIndex, latIndex, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex, latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex, kSubdivision, sphereData->radius_);

			// 三角形2 (b, d, c)
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex, latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(dPos, lonIndex + 1, latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex, kSubdivision, sphereData->radius_);
		}
	}

	sphereVertexCount_ += vertexIndex;
}

//====================================================================
// Coneの頂点データの作成関数
//====================================================================

void PrimitiveDrawer::CreateConeVertexData(ConeData* coneData) {

	auto CreateConeVertex = [&](const Vector3& pos, float u, float v, const Vector3& normal) {
		VertexData vertex{};
		vertex.position = { pos.x, pos.y, pos.z, 1.0f };
		vertex.normal = normal;
		vertex.texcoord = { u, v };
		return vertex;
		};

	const uint32_t kSubdivision = coneData->subDivision_;  // 円の分割数
	const float kAngleEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);

	// 頂点数 = 側面(kSubdivision * 3) + 底面(kSubdivision * 3)
	uint32_t vertexCount = kSubdivision * 3 + kSubdivision * 3;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	coneData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	coneData->primitiveData_.vertexBuffer_->SetName(L"Cone::vertexResource_");

	// bufferview設定
	coneData->primitiveData_.vertexBufferView_.BufferLocation = coneData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	coneData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	coneData->primitiveData_.vertexBufferView_.SizeInBytes = size;

	// mapping
	coneData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&coneData->vertexData_));

	VertexData* vertexData = coneData->vertexData_;
	uint32_t vertexIndex = 0;

	// 円錐の頂点位置（上部）
	Vector3 apex = { 0.0f, coneData->height_, 0.0f };

	// 底面の中心位置
	Vector3 baseCenter = { 0.0f, 0.0f, 0.0f };

	// ===== 側面の生成 =====
	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 底面の円周上の点
		Vector3 basePos = {
			coneData->radius_ * cosf(angle),
			0.0f,
			coneData->radius_ * sinf(angle)
		};

		Vector3 basePosNext = {
			coneData->radius_ * cosf(angleNext),
			0.0f,
			coneData->radius_ * sinf(angleNext)
		};

		// 法線の計算（側面）
		Vector3 edge1 = { basePosNext.x - basePos.x, basePosNext.y - basePos.y, basePosNext.z - basePos.z };
		Vector3 edge2 = { apex.x - basePos.x, apex.y - basePos.y, apex.z - basePos.z };
		Vector3 normal = {
			edge1.y * edge2.z - edge1.z * edge2.y,
			edge1.z * edge2.x - edge1.x * edge2.z,
			edge1.x * edge2.y - edge1.y * edge2.x
		};
		float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		normal = { normal.x / length, normal.y / length, normal.z / length };

		// 側面の三角形 (basePos, apex, basePosNext)
		float u = static_cast<float>(i) / static_cast<float>(kSubdivision);
		float uNext = static_cast<float>(i + 1) / static_cast<float>(kSubdivision);

		vertexData[vertexIndex++] = CreateConeVertex(basePos, u, 1.0f, normal);
		vertexData[vertexIndex++] = CreateConeVertex(apex, (u + uNext) * 0.5f, 0.0f, normal);
		vertexData[vertexIndex++] = CreateConeVertex(basePosNext, uNext, 1.0f, normal);
	}

	// ===== 底面の生成 =====
	Vector3 bottomNormal = { 0.0f, -1.0f, 0.0f };  // 底面の法線は下向き

	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 底面の円周上の点
		Vector3 basePos = {
			coneData->radius_ * cosf(angle),
			0.0f,
			coneData->radius_ * sinf(angle)
		};

		Vector3 basePosNext = {
			coneData->radius_ * cosf(angleNext),
			0.0f,
			coneData->radius_ * sinf(angleNext)
		};

		// テクスチャ座標（0.5を中心とした円）
		float u = 0.5f + 0.5f * cosf(angle);
		float v = 0.5f + 0.5f * sinf(angle);
		float uNext = 0.5f + 0.5f * cosf(angleNext);
		float vNext = 0.5f + 0.5f * sinf(angleNext);

		// 底面の三角形 (baseCenter, basePosNext, basePos) - 時計回り
		vertexData[vertexIndex++] = CreateConeVertex(baseCenter, 0.5f, 0.5f, bottomNormal);
		vertexData[vertexIndex++] = CreateConeVertex(basePosNext, uNext, vNext, bottomNormal);
		vertexData[vertexIndex++] = CreateConeVertex(basePos, u, v, bottomNormal);
	}

	coneVertexCount_ += vertexIndex;
}

//====================================================================
// マテリアルの作成関数
//====================================================================

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

void PrimitiveDrawer::CreateSphereMaterial(const std::string& textureFilePath, SphereData* sphereData) {

	sphereData->material_ = new Material();
	sphereData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	sphereData->material_->SetEnableLighting(false);
	sphereData->material_->SetEnvCoefficient(0.0f);
	sphereData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::CreateConeMaterial(const std::string& textureFilePath, ConeData* coneData) {

	coneData->material_ = new Material();
	coneData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	coneData->material_->SetEnableLighting(false);
	coneData->material_->SetEnvCoefficient(0.0f);
	coneData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

PrimitiveDrawer::PlaneData* PrimitiveDrawer::GetPlaneData(uint32_t handle) {
	return planeDatas_[handle].get();
}

PrimitiveDrawer::SphereData* PrimitiveDrawer::GetSphereData(uint32_t handle) {
	return sphereDatas_[handle].get();
}

PrimitiveDrawer::RingData* PrimitiveDrawer::GetRingData(uint32_t handle) {
	return ringDatas_[handle].get();
}

PrimitiveDrawer::ConeData* PrimitiveDrawer::GetConeData(uint32_t handle) {
	return coneDatas_[handle].get();
}
