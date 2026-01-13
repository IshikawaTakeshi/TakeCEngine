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
}

//============================================================================
// ImGui更新処理（パラメータ付き）
//============================================================================
void TakeC::PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3&) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		cube_->UpdateImGui(handle, "Edit Cube");
	}
	case PRIMITIVE_PLANE:
	{
		//Planeのパラメータ更新と表示
		plane_->UpdateImGui(handle, "Edit Plane");
		
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		//Sphereのパラメータ更新と表示
		sphere_->UpdateImGui(handle, "Edit Sphere");
		
		break;
	}
	case PRIMITIVE_CONE:
	{
		//Coneのパラメータ更新と表示
		cone_->UpdateImGui(handle, "Edit Cone");
		
		break;
	}
	case PRIMITIVE_CUBE:
	{
		//Cubeのパラメータ更新と表示
		cube_->UpdateImGui(handle, "Edit Cube");
		break;
	}
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
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
//	描画処理
//=================================================================================

void TakeC::PrimitiveDrawer::DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle) {

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
		
		cone_->DrawParticle(pso, instanceCount, handle);
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
		
		sphere_->DrawObject(pso, handle);
		break;
	}
	case PRIMITIVE_CONE:
	{
		//--------------------------------------------------
		//		coneの描画
		//--------------------------------------------------
		
		cone_->DrawObject(pso, handle);
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
		cone_->SetMaterialColor(handle, color);
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
