#include "Cube.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================

void Cube::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// cube生成
//============================================================================

uint32_t TakeC::Cube::Generate(const AABB& size, const std::string& textureFilePath) {
	
	auto cube = std::make_unique<CubeData>();
	cube->size = size;
	// 頂点データ作成
	CreateVertexData(cube.get());
	// マテリアル作成
	CreateMaterial(textureFilePath, cube.get());
	// ハンドルを発行して登録
	uint32_t handle = nextHandle_++;
	datas_[handle] = std::move(cube);
	return handle;
}

//============================================================================
// 描画処理(パーティクル用)
//============================================================================

void TakeC::Cube::DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle) {
	// インスタンス数が0の場合は早期リターン
	if (instanceCount == 0) {
		return;
	}
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& cubeData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &cubeData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		cubeData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(cubeData->material->GetTextureFilePath()));
	// 描画コマンド
	commandList->DrawInstanced(cubeData->vertexCount, instanceCount, 0, 0);
}

//============================================================================
// 描画処理（オブジェクト用）
//============================================================================

void TakeC::Cube::DrawObject(PSO* pso, uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& cubeData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &cubeData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		cubeData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(cubeData->material->GetTextureFilePath()));
	// 描画コマンド
	commandList->DrawInstanced(cubeData->vertexCount, 1, 0, 0);
}

//============================================================================
// データ取得
//============================================================================

Cube::CubeData* Cube::GetData(uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return nullptr;  // ハンドルが無効
	}
	return it->second.get();
}

//============================================================================
// マテリアル色設定
//============================================================================

void Cube::SetMaterialColor(uint32_t handle, const Vector4& color) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& cubeData = it->second;
	cubeData->material->GetMaterialData()->color = color;
}

//============================================================================
// 頂点データ作成
//============================================================================

void Cube::CreateVertexData(CubeData* cubeData) {
	UINT size = sizeof(VertexData) * 36;

	//bufferをカウント分確保
	cubeData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	cubeData->mesh.vertexBuffer_->SetName(L"Cube::vertexBuffer");
	//bufferview設定
	cubeData->mesh.vertexBufferView_.BufferLocation = cubeData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	cubeData->mesh.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	cubeData->mesh.vertexBufferView_.SizeInBytes = size;
	//mapping
	cubeData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cubeData->vertexData));

	//Cubeの頂点データを生成(36頂点分)
	Vector3 max = cubeData->size.max;
	Vector3 min = cubeData->size.min;

	// 各頂点の座標を決める
	Vector3 p000 = {min.x, min.y, min.z};
	Vector3 p001 = {min.x, min.y, max.z};
	Vector3 p010 = {min.x, max.y, min.z};
	Vector3 p011 = {min.x, max.y, max.z};
	Vector3 p100 = {max.x, min.y, min.z};
	Vector3 p101 = {max.x, min.y, max.z};
	Vector3 p110 = {max.x, max.y, min.z};
	Vector3 p111 = {max.x, max.y, max.z};

	VertexData* v = cubeData->vertexData;
	int idx = 0;

	auto SetVertex = [&](int i, const Vector3& pos, const Vector2& uv, const Vector3& normal) {
		v[i].position = {pos.x, pos.y, pos.z, 1.0f};
		v[i].texcoord = uv;
		v[i].normal   = normal;
		};

	// +X面 (右側)
	{
		Vector3 n = { 1.0f, 0.0f, 0.0f };
		// 2 Triangles: (p110, p111, p101), (p110, p101, p100)
		SetVertex(idx + 0, p110, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p111, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p101, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p110, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p101, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p100, {0.0f, 1.0f}, n);
		idx += 6;
	}

	// -X面 (左側)
	{
		Vector3 n = { -1.0f, 0.0f, 0.0f };
		// (p011, p010, p000), (p011, p000, p001)
		SetVertex(idx + 0, p011, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p010, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p000, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p011, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p000, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p001, {0.0f, 1.0f}, n);
		idx += 6;
	}

	// +Y面 (上)
	{
		Vector3 n = {0.0f, 1.0f, 0.0f};
		// (p010, p110, p111), (p010, p111, p011)
		SetVertex(idx + 0, p010, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p110, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p111, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p010, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p111, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p011, {0.0f, 1.0f}, n);
		idx += 6;
	}

	// -Y面 (下)
	{
		Vector3 n = {0.0f, -1.0f, 0.0f};
		// (p001, p101, p100), (p001, p100, p000)
		SetVertex(idx + 0, p001, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p101, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p100, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p001, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p100, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p000, {0.0f, 1.0f}, n);
		idx += 6;
	}

	// +Z面 (前)
	{
		Vector3 n = {0.0f, 0.0f, 1.0f};
		// (p111, p101, p001), (p111, p001, p011)
		SetVertex(idx + 0, p111, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p101, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p001, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p111, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p001, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p011, {0.0f, 1.0f}, n);
		idx += 6;
	}

	// -Z面 (後ろ)
	{
		Vector3 n = {0.0f, 0.0f, -1.0f};
		// (p010, p000, p100), (p010, p100, p110)
		SetVertex(idx + 0, p010, {0.0f, 0.0f}, n);
		SetVertex(idx + 1, p000, {1.0f, 0.0f}, n);
		SetVertex(idx + 2, p100, {1.0f, 1.0f}, n);

		SetVertex(idx + 3, p010, {0.0f, 0.0f}, n);
		SetVertex(idx + 4, p100, {1.0f, 1.0f}, n);
		SetVertex(idx + 5, p110, {0.0f, 1.0f}, n);
		idx += 6;
	}	

	cubeData->vertexCount = 36;
}

//============================================================================
// マテリアル作成
//============================================================================

void TakeC::Cube::CreateMaterial(const std::string& textureFilePath, CubeData* planeData) {

	planeData->material = std::make_unique<Material>();
	planeData->material->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
}
