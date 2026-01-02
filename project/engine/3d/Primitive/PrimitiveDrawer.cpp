#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ImGuiManager.h"
#include "MatrixMath.h"
#include "Particle/Particle3d.h"

#include <numbers>

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================

void TakeC::PrimitiveDrawer::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// 各プリミティブデータの初期化
	cube_ = std::make_unique<TakeC::Cube>();
	cube_->Initialize(dxCommon_, srvManager_);

	ring_ = std::make_unique<TakeC::Ring>();
	ring_->Initialize(dxCommon_, srvManager_);

	plane_ = std::make_unique<TakeC::Plane>();
	plane_->Initialize(dxCommon_, srvManager_);

	sphere_ = std::make_unique<TakeC::Sphere>();
	sphere_->Initialize(dxCommon_, srvManager_);

}

//============================================================================
// 終了処理
//============================================================================
void TakeC::PrimitiveDrawer::Finalize() {

	// 各プリミティブデータの解放
	cube_.reset();
	plane_.reset();
	ring_.reset();
	sphere_.reset();

	for (auto& pair : coneDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
}

//============================================================================
// ImGui更新処理（パラメータ付き）
//============================================================================
void TakeC::PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param) {

	switch (type) {
	case PRIMITIVE_RING:
	{

	}
	case PRIMITIVE_PLANE:
	{
		//Planeのパラメータ更新と表示
		
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		//Sphereのパラメータ更新と表示
		
		break;
	}
	case PRIMITIVE_CONE:
	{
		//Coneのパラメータ更新と表示
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

//=================================================================================
//	プリミティブデータ生成処理(リング)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateRing(float outerRadius, float innerRadius, const std::string& textureFilePath) {
	return ring_->Generate(outerRadius, innerRadius, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(平面)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GeneratePlane(float width, float height, const std::string& textureFilePath) {

	return plane_->Generate(width, height, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(球)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateSphere(float radius, const std::string& textureFilePath) {

	return sphere_->Generate(radius, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(円錐)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateCone(float radius,float height, uint32_t subDivision, const std::string& textureFilePath) {

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
//	プリミティブデータ生成処理(cube)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateCube(const AABB& size, const std::string& textureFilePath) {
	return cube_->Generate(size, textureFilePath);
}

//=================================================================================
//	描画処理
//=================================================================================

void TakeC::PrimitiveDrawer::DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();


	switch (type) {
	case PRIMITIVE_RING:
	{

		//--------------------------------------------------
		//		ringの描画
		//--------------------------------------------------

		ring_->DrawParticle(pso, instanceCount, handle);

		break;
	}
	case PRIMITIVE_PLANE:
	{

		//--------------------------------------------------
		//		planeの描画
		//--------------------------------------------------

		plane_->DrawParticle(pso, instanceCount, handle);

		break;
	}
	case PRIMITIVE_SPHERE:
	{
		//--------------------------------------------------
		//		sphereの描画
		//--------------------------------------------------

		sphere_->DrawParticle(pso, instanceCount, handle);

		break;
	}
	case PRIMITIVE_CONE:
	{
		//--------------------------------------------------
		//		coneの描画
		//--------------------------------------------------
		auto itCone = coneDatas_.find(handle);
		if (itCone == coneDatas_.end()) return;

		// coneデータ取得
		auto& coneData = itCone->second;
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &coneData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), coneData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance().GetSrvIndex(coneData->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(coneVertexCount_, instanceCount, 0, 0);
		break;
	}
	case PRIMITIVE_CUBE:
	{
		//--------------------------------------------------
		//		cubeの描画
		//--------------------------------------------------

		cube_->DrawParticle(pso, instanceCount, handle);
		break;
	}

	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

//=================================================================================
//	描画処理(オブジェクト用)
//=================================================================================
void TakeC::PrimitiveDrawer::DrawObject(PSO* pso, PrimitiveType type, uint32_t handle) {


	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	switch (type) {
	case PRIMITIVE_RING:
	{
		//--------------------------------------------------
		//		ringの描画
		//--------------------------------------------------

		ring_->DrawObject(pso, handle);
		
		break;
	}
	case PRIMITIVE_PLANE:
	{
		//--------------------------------------------------
		//		planeの描画
		//--------------------------------------------------
		
		plane_->DrawObject(pso, handle);
		break;

	}
	case PRIMITIVE_SPHERE:
	{
		//--------------------------------------------------
		//		sphereの描画
		//--------------------------------------------------
		
		ring_->DrawObject(pso, handle);
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
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance().GetSrvIndex(coneData->material_->GetTextureFilePath()));
		//IBVの設定
		//commandList->IASetIndexBuffer(&coneData->primitiveData_.indexBufferView_);
		//描画
		commandList->DrawInstanced(coneVertexCount_, 1, 0, 0);
		break;
	}
	case PRIMITIVE_CUBE:
	{
		//--------------------------------------------------
		//		cubeの描画
		//--------------------------------------------------
		
		cube_->DrawObject(pso, handle);
		break;
	}
	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

//====================================================================
// 頂点データの作成関数(円錐)
//====================================================================

void TakeC::PrimitiveDrawer::CreateConeVertexData(ConeData* coneData) {

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

void TakeC::PrimitiveDrawer::CreateConeMaterial(const std::string& textureFilePath, ConeData* coneData) {

	coneData->material_ = new Material();
	coneData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	coneData->material_->SetEnableLighting(false);
	coneData->material_->SetEnvCoefficient(0.0f);
	coneData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}
//====================================================================
//	データ取得関数
//====================================================================

Plane::PlaneData* TakeC::PrimitiveDrawer::GetPlaneData(uint32_t handle) {
	// planeDataを返す
	return plane_->GetData(handle);
}

Sphere::SphereData* TakeC::PrimitiveDrawer::GetSphereData(uint32_t handle) {
	// sphereDataを返す
	return sphere_->GetData(handle);
}

Ring::RingData* TakeC::PrimitiveDrawer::GetRingData(uint32_t handle) {
	return ring_->GetData(handle);
}


TakeC::PrimitiveDrawer::ConeData* TakeC::PrimitiveDrawer::GetConeData(uint32_t handle) {
	// coneDataを返す
	return coneDatas_[handle].get();
}

void TakeC::PrimitiveDrawer::SetMaterialColor(uint32_t handle, PrimitiveType type, const Vector4& color) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		ring_->SetMaterialColor(handle, color);
		break;
	}
	case PRIMITIVE_PLANE:
	{
		plane_->SetMaterialColor(handle, color);		
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		sphere_->SetMaterialColor(handle, color);
		break;
	}
	case PRIMITIVE_CONE:
	{
		auto itCone = coneDatas_.find(handle);
		if (itCone == coneDatas_.end()) return;
		itCone->second->material_->SetMaterialColor(color);
		break;
	}
	case PRIMITIVE_CUBE:
	{
		cube_->SetMaterialColor(handle, color);
		break;
	}
	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}
