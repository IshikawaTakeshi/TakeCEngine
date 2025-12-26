#include "Plane.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================

void Plane::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// plane生成
//============================================================================

uint32_t Plane::Generate(float width, float height, const std::string& textureFilePath) {
	auto plane = std::make_unique<PlaneData>();
	plane->width = width;
	plane->height = height;
	// 頂点データ作成
	CreateVertexData(plane.get());
	// マテリアル作成
	CreateMaterial(textureFilePath, plane.get());
	// ハンドルを発行して登録
	uint32_t handle = nextHandle_++;
	datas_[handle] = std::move(plane);
	return handle;
}

//============================================================================
// 描画処理(パーティクル用)
//============================================================================

void Plane::DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle) {
	// インスタンス数が0の場合は早期リターン
	if (instanceCount == 0) {
		return;
	}
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& planeData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &planeData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		planeData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(planeData->material->GetTextureFilePath()));

	//このRing固有の頂点数を使用
	commandList->DrawInstanced(planeData->vertexCount, instanceCount, 0, 0);
}

//============================================================================
// 描画処理（オブジェクト用）
//============================================================================

void Plane::DrawObject(PSO* pso, uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& planeData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &planeData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		planeData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(planeData->material->GetTextureFilePath()));
	//このRing固有の頂点数を使用
	commandList->DrawInstanced(planeData->vertexCount, 1, 0, 0);
}

//============================================================================
// planeデータ取得
//============================================================================

Plane::PlaneData* Plane::GetData(uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return nullptr;  // ハンドルが無効
	}
	return it->second.get();
}

//============================================================================
// マテリアル色設定
//============================================================================

void Plane::SetMaterialColor(uint32_t handle, const Vector4& color) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	it->second->material->SetMaterialColor(color);
}

//============================================================================
// 頂点データ作成
//============================================================================

void Plane::CreateVertexData(PlaneData* planeData) {
	// Planeは固定で6頂点
	const uint32_t kVertexCount = 6;
	planeData->vertexCount = kVertexCount;
	UINT size = sizeof(VertexData) * kVertexCount;
	planeData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	planeData->mesh.vertexBuffer_->SetName(L"Plane::vertexBuffer");

	planeData->mesh.vertexBufferView_.BufferLocation = planeData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	planeData->mesh.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	planeData->mesh.vertexBufferView_.SizeInBytes = size;
	//mapping
	planeData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&planeData->vertexData));

	//Planeの頂点データを生成(6頂点分)
	planeData->vertexData[0].position = { -planeData->width,  planeData->height,0.0f, 1.0f }; //左下
	planeData->vertexData[1].position = { -planeData->width, -planeData->height,0.0f, 1.0f }; //左上
	planeData->vertexData[2].position = {  planeData->width,  planeData->height,0.0f, 1.0f }; //右下
	planeData->vertexData[3].position = { -planeData->width, -planeData->height,0.0f, 1.0f }; //左上
	planeData->vertexData[4].position = {  planeData->width, -planeData->height,0.0f, 1.0f }; //右上
	planeData->vertexData[5].position = {  planeData->width,  planeData->height,0.0f, 1.0f }; //右下
	//PlaneのUV座標を生成
	planeData->vertexData[0].texcoord = { 0.0f, 1.0f };
	planeData->vertexData[1].texcoord = { 0.0f, 0.0f };
	planeData->vertexData[2].texcoord = { 1.0f, 1.0f };
	planeData->vertexData[3].texcoord = { 0.0f, 0.0f };
	planeData->vertexData[4].texcoord = { 1.0f, 0.0f };
	planeData->vertexData[5].texcoord = { 1.0f, 1.0f };
	//Planeの法線を生成
	planeData->vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData[4].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData[5].normal = { 0.0f, 0.0f, -1.0f };

}

//============================================================================
// マテリアル作成
//============================================================================

void Plane::CreateMaterial(const std::string& textureFilePath, PlaneData* planeData) {
	planeData->material = std::make_unique<Material>();
	planeData->material->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
}