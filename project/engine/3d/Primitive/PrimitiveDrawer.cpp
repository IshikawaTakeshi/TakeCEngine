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

	planeDatas_.clear();
	sphereDatas_.clear();
	ringDatas_.clear();
	boxDatas_.clear();
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

uint32_t PrimitiveDrawer::GenerateSphere(const float radius, const std::string& textureFilePath) {
	
	auto sphere = std::make_unique<SphereData>();
	sphere->radius_ = radius;
	CreateSphereVertexData(sphere.get());
	CreateSphereMaterial(textureFilePath, sphere.get());
	uint32_t useHandle = sphereHandle_++;
	sphereDatas_[useHandle] = std::move(sphere);
	return useHandle;
}

uint32_t PrimitiveDrawer::GenerateBox(const Vector3& size, const std::string& textureFilePath) {
	
	auto box = std::make_unique<BoxData>();
	box->size_ = size;
	CreateBoxVertexData(box.get());
	CreateBoxMaterial(textureFilePath, box.get());
	uint32_t useHandle = boxHandle_++;
	boxDatas_[useHandle] = std::move(box);
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

	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::DrawObject(PSO* pso, PrimitiveType type, uint32_t handle) {

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
		//envMap
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gEnvMapTexture"), TextureManager::GetInstance()->GetSrvIndex(ringData->material_->GetEnvMapFilePath()));
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
		//envMap
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gEnvMapTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData->material_->GetEnvMapFilePath()));
		
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
		//envMap
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gEnvMapTexture"), TextureManager::GetInstance()->GetSrvIndex(sphereData->material_->GetEnvMapFilePath()));
		
		//描画
		commandList->DrawInstanced(sphereVertexCount_, 1, 0, 0);

		break;
	}

	case PRIMITIVE_BOX:
	{
		//--------------------------------------------------
		//		boxの描画
		//--------------------------------------------------
		auto itBox = boxDatas_.find(handle);
		if (itBox == boxDatas_.end()) return;
		auto& boxData = itBox->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &boxData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), boxData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(boxData->material_->GetTextureFilePath()));
		commandList->DrawInstanced(boxVertexCount_, 1, 0, 0);
		//envMap
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gEnvMapTexture"), TextureManager::GetInstance()->GetSrvIndex(boxData->material_->GetEnvMapFilePath()));
		//描画
		commandList->DrawInstanced(boxVertexCount_, 1, 0, 0);

		break;
	}

	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::DrawSprite(PSO* pso, uint32_t handle) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

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
}

void PrimitiveDrawer::DrawSkyBox(PSO* pso, uint32_t handle) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();


	//--------------------------------------------------
	//		boxの描画
	//--------------------------------------------------
	auto itBox = boxDatas_.find(handle);
	if (itBox == boxDatas_.end()) return;
	auto& boxData = itBox->second;
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VertexBufferView
	commandList->IASetVertexBuffers(0, 1, &boxData->primitiveData_.vertexBufferView_);
	// materialResource
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"), boxData->material_->GetMaterialResource()->GetGPUVirtualAddress());
	// texture
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(boxData->material_->GetTextureFilePath()));
	commandList->DrawInstanced(boxVertexCount_, 1, 0, 0);
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

void PrimitiveDrawer::CreateSphereVertexData(SphereData* sphereData) {

	auto CreateSphereVertex = [&](const Vector3& pos, uint32_t lonIndex, uint32_t latIndex, uint32_t kSubdivision, float radius) {
		VertexData v{};
		v.position = { pos.x, pos.y, pos.z, 1.0f };
		v.normal = { pos.x / radius, pos.y / radius, pos.z / radius };
		v.texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
		v.texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
		return v;
	};

	const uint32_t kSubdivision = sphereDivide_;
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision);         // 緯度刻み
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);  // 経度刻み

	// 頂点数 = 1マス6頂点 × マス数
	uint32_t vertexCount = kSubdivision * kSubdivision * 6;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	sphereData->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	sphereData->primitiveData_.vertexResource_->SetName(L"Sphere::vertexResource_");

	// bufferview設定
	sphereData->primitiveData_.vertexBufferView_.BufferLocation = sphereData->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	sphereData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	sphereData->primitiveData_.vertexBufferView_.SizeInBytes = size;

	// mapping
	sphereData->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&sphereData->vertexData_));

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
			vertexData[vertexIndex++] = CreateSphereVertex(aPos, lonIndex,     latIndex,     kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex,     latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex,     kSubdivision, sphereData->radius_);

			// 三角形2 (b, d, c)
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex,     latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(dPos, lonIndex + 1, latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex,     kSubdivision, sphereData->radius_);
		}
	}

	sphereVertexCount_ += vertexIndex;
}

void PrimitiveDrawer::CreateBoxVertexData(BoxData* boxData) {

	// 立方体の8頂点
		// 中心が原点、サイズはboxData->size_ (x, y, z)
		float hx = boxData->size_.x * 0.5f;
	float hy = boxData->size_.y * 0.5f;
	float hz = boxData->size_.z * 0.5f;

	// 8 corners
	Vector3 p[8] = {
		{-hx, -hy, -hz}, // 0: 左下手前
		{-hx, +hy, -hz}, // 1: 左上手前
		{+hx, +hy, -hz}, // 2: 右上手前
		{+hx, -hy, -hz}, // 3: 右下手前
		{-hx, -hy, +hz}, // 4: 左下奥
		{-hx, +hy, +hz}, // 5: 左上奥
		{+hx, +hy, +hz}, // 6: 右上奥
		{+hx, -hy, +hz}  // 7: 右下奥
	};

	// 各面2三角形×6面=12三角形=36頂点
	// 頂点データ
	static const struct {
		int i0, i1, i2, i3;
		Vector3 normal;
	} faces[6] = {
		// 前面(-Z)
		{ 0, 1, 2, 3, { 0.0f,  0.0f, -1.0f } },
		// 背面(+Z)
		{ 7, 6, 5, 4, { 0.0f,  0.0f,  1.0f } },
		// 左面(-X)
		{ 4, 5, 1, 0, { -1.0f, 0.0f,  0.0f } },
		// 右面(+X)
		{ 3, 2, 6, 7, { 1.0f,  0.0f,  0.0f } },
		// 上面(+Y)
		{ 1, 5, 6, 2, { 0.0f,  1.0f,  0.0f } },
		// 下面(-Y)
		{ 4, 0, 3, 7, { 0.0f, -1.0f,  0.0f } }
	};

	// UV: (左下, 左上, 右上, 右下)
	Vector2 uvs[4] = {
		{0.0f, 1.0f}, // 左下
		{0.0f, 0.0f}, // 左上
		{1.0f, 0.0f}, // 右上
		{1.0f, 1.0f}  // 右下
	};

	const UINT kVertexCount = 36;
	UINT size = sizeof(VertexData) * kVertexCount * kMaxVertexCount_;
	boxData->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	boxData->primitiveData_.vertexResource_->SetName(L"Box::vertexResource_");
	boxData->primitiveData_.vertexBufferView_.BufferLocation = boxData->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	boxData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	boxData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	boxData->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&boxData->vertexData_));

	VertexData* vtx = boxData->vertexData_;
	uint32_t idx = 0;
	for (int f = 0; f < 6; ++f) {
		// 2 triangles per face (0,1,2) (0,2,3)
		int i0 = faces[f].i0, i1 = faces[f].i1, i2 = faces[f].i2, i3 = faces[f].i3;
		Vector3 normal = faces[f].normal;

		// 1st triangle
		vtx[idx].position = { p[i0].x, p[i0].y, p[i0].z, 1.0f };
		vtx[idx].texcoord = uvs[0];
		vtx[idx].normal = normal;
		++idx;

		vtx[idx].position = { p[i1].x, p[i1].y, p[i1].z, 1.0f };
		vtx[idx].texcoord = uvs[1];
		vtx[idx].normal = normal;
		++idx;

		vtx[idx].position = { p[i2].x, p[i2].y, p[i2].z, 1.0f };
		vtx[idx].texcoord = uvs[2];
		vtx[idx].normal = normal;
		++idx;

		// 2nd triangle
		vtx[idx].position = { p[i0].x, p[i0].y, p[i0].z, 1.0f };
		vtx[idx].texcoord = uvs[0];
		vtx[idx].normal = normal;
		++idx;

		vtx[idx].position = { p[i2].x, p[i2].y, p[i2].z, 1.0f };
		vtx[idx].texcoord = uvs[2];
		vtx[idx].normal = normal;
		++idx;

		vtx[idx].position = { p[i3].x, p[i3].y, p[i3].z, 1.0f };
		vtx[idx].texcoord = uvs[3];
		vtx[idx].normal = normal;
		++idx;
	}

	boxVertexCount_ += idx; // = 36
}


void PrimitiveDrawer::CreateRingMaterial(const std::string& textureFilePath, RingData* ringData) {

	ringData->material_ = std::make_unique<Material>();
	ringData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	ringData->material_->SetEnableLighting(false);
	ringData->material_->SetEnvCoefficient(0.0f);
	ringData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
	ringData->material_->SetUvScale({ 2.5f, 0.5f,1.0f }); // UVスケールを設定
}

void PrimitiveDrawer::CreatePlaneMaterial(const std::string& textureFilePath, PlaneData* planeData) {

	planeData->material_ = std::make_unique<Material>();
	planeData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	planeData->material_->SetEnableLighting(false);
	planeData->material_->SetEnvCoefficient(0.0f);
	planeData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::CreateSphereMaterial(const std::string& textureFilePath, SphereData* sphereData) {

	sphereData->material_ = std::make_unique<Material>();
	sphereData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	sphereData->material_->SetEnableLighting(false);
	sphereData->material_->SetEnvCoefficient(0.0f);
	sphereData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::CreateBoxMaterial(const std::string& textureFilePath, BoxData* boxData) {

	boxData->material_ = std::make_unique<Material>();
	boxData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	boxData->material_->SetEnableLighting(false);
	boxData->material_->SetEnvCoefficient(0.0f);
	boxData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::MappingPlaneVertexData(VertexData* vertexData, uint32_t primitiveHandle) {

	planeDatas_[primitiveHandle]->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
}