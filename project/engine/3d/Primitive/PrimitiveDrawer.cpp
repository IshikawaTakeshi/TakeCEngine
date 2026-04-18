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

	cone_ = std::make_unique<TakeC::Cone>();
	cone_->Initialize(dxCommon_, srvManager_);

	cylinder_ = std::make_unique<TakeC::Cylinder>();
	cylinder_->Initialize(dxCommon_, srvManager_);

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
	cone_.reset();
	cylinder_.reset();

	primitiveMap_.clear();
}

//============================================================================
// 解放処理
//============================================================================
void TakeC::PrimitiveDrawer::Release(uint32_t handle) {
	primitiveMap_.erase(handle);
}

//=================================================================================
//	プリミティブベースデータ取得処理
//=================================================================================
PrimitiveBaseData* TakeC::PrimitiveDrawer::GetBaseData(uint32_t handleId) {
	auto it = primitiveMap_.find(handleId);
	if (it == primitiveMap_.end()) {
		return nullptr;
	}
	return it->second.data.get();
}

PrimitiveType TakeC::PrimitiveDrawer::GetPrimitiveType(uint32_t handleId) {
	auto it = primitiveMap_. find(handleId);
	if (it == primitiveMap_.end()) {
		return PRIMITIVE_COUNT;
	}
	return it->second.type;
}

//=================================================================================
//	プリミティブデータ生成処理(リング)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateRing(float outerRadius, float innerRadius,uint32_t subDivision, const std::string& textureFilePath) {
	return Generate<Ring>(outerRadius, innerRadius, subDivision, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(平面)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GeneratePlane(float width, float height, const std::string& textureFilePath) {

	return Generate<Plane>(width, height, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(球)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateSphere(float radius,uint32_t subDivision, const std::string& textureFilePath) {

	return Generate<Sphere>(radius,subDivision, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(円錐)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateCone(float radius,float height, uint32_t subDivision, const std::string& textureFilePath) {

	return Generate<Cone>(radius, height, subDivision, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(cube)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateCube(const AABB& size, const std::string& textureFilePath) {
	return Generate<Cube>(size, textureFilePath);
}

//=================================================================================
//	プリミティブデータ生成処理(cylinder)
//=================================================================================
uint32_t TakeC::PrimitiveDrawer::GenerateCylinder(float radius, float height, uint32_t subDivision, const std::string& textureFilePath) {
	return Generate<Cylinder>(radius, height, subDivision, textureFilePath);
}

//=================================================================================
//	描画処理
//=================================================================================

void TakeC::PrimitiveDrawer::DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle) {
	type;
	if (instanceCount == 0) return;
	auto* baseData = GetBaseData(handle);
	if (!baseData) return;
	baseData->material->Update();
	DrawCommon(pso, baseData);
	dxCommon_->GetCommandList()->DrawInstanced(baseData->vertexCount, instanceCount, 0, 0);
}

//=================================================================================
//	描画処理(オブジェクト用)
//=================================================================================
void TakeC::PrimitiveDrawer::DrawObject(PSO* pso, PrimitiveType type, uint32_t handle) {
	type;
	auto* baseData = GetBaseData(handle);
	if (!baseData) return;
	baseData->material->Update();
	DrawCommon(pso, baseData);
	dxCommon_->GetCommandList()->DrawInstanced(baseData->vertexCount, 1, 0, 0);
}

//====================================================================
//	データ取得関数
//====================================================================

PlaneData* TakeC::PrimitiveDrawer::GetPlaneData(uint32_t handle) {
	// planeDataを返す
	return GetData<Plane>(handle);
}

SphereData* TakeC::PrimitiveDrawer::GetSphereData(uint32_t handle) {
	// sphereDataを返す
	return GetData<Sphere>(handle);
}

RingData* TakeC::PrimitiveDrawer::GetRingData(uint32_t handle) {
	return GetData<Ring>(handle);
}


ConeData* TakeC::PrimitiveDrawer::GetConeData(uint32_t handle) {
	// coneDataを返す
	return GetData<Cone>(handle);
}

CubeData* TakeC::PrimitiveDrawer::GetCubeData(uint32_t handle) {
	return GetData<Cube>(handle);
}

CylinderData* TakeC::PrimitiveDrawer::GetCylinderData(uint32_t handle) {
	return GetData<Cylinder>(handle);
}

void TakeC::PrimitiveDrawer::DrawCommon(PSO* pso, PrimitiveBaseData* data) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &data->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		data->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(data->material->GetTextureFilePath()));
}
