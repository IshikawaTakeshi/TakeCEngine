#include "Ring.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================
void Ring::Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// リングデータの生成
//============================================================================
uint32_t Ring::Generate(float outerRadius, float innerRadius, const std::string& textureFilePath) {

	auto ring = std::make_unique<RingData>();
	ring->innerRadius = innerRadius;
	ring->outerRadius = outerRadius;
	// 頂点データ作成
	CreateVertexData(ring.get());
	// マテリアル作成
	CreateMaterial(textureFilePath, ring.get());
	// ハンドルを発行して登録
	uint32_t handle = nextHandle_++;
	datas_[handle] = std::move(ring);
	return handle;
}

//============================================================================
// 頂点データ作成
//============================================================================
void Ring::CreateVertexData(RingData* ringData) {
	// この Ring 固有の頂点数を計算
	uint32_t vertexCount = ringData->subDivision * kVerticesPerSegment;
	ringData->vertexCount = vertexCount;  // ★個別に保存
	UINT size = sizeof(VertexData) * vertexCount;
	// バッファ確保
	ringData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	ringData->mesh.vertexBuffer_->SetName(L"PrimitiveRing::vertexBuffer");
	// BufferView 設定
	ringData->mesh.vertexBufferView_.BufferLocation = ringData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	ringData->mesh.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData->mesh.vertexBufferView_.SizeInBytes = size;
	// マッピング
	ringData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&ringData->vertexData));
	// 頂点データ生成
	const float radianPerDivide = (2.0f * kPi) / static_cast<float>(ringData->subDivision);
	uint32_t vertexIndex = 0;
	for (uint32_t index = 0; index < ringData->subDivision; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringData->subDivision);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringData->subDivision);
		// 頂点座標
		ringData->vertexData[vertexIndex].position = { ringData->outerRadius * -sin, ringData->outerRadius * cos, 0.0f, 1.0f };
		ringData->vertexData[vertexIndex + 1].position = { ringData->innerRadius * -sin, ringData->innerRadius * cos, 0.0f, 1.0f };
		ringData->vertexData[vertexIndex + 2].position = { ringData->innerRadius * -sinNext, ringData->innerRadius * cosNext, 0.0f, 1.0f };
		ringData->vertexData[vertexIndex + 3].position = ringData->vertexData[vertexIndex].position;
		ringData->vertexData[vertexIndex + 4].position = ringData->vertexData[vertexIndex + 2].position;
		ringData->vertexData[vertexIndex + 5].position = { ringData->outerRadius * -sinNext, ringData->outerRadius * cosNext, 0.0f, 1.0f };
		// UV座標
		ringData->vertexData[vertexIndex].texcoord = { uv, 0.0f };
		ringData->vertexData[vertexIndex + 1].texcoord = { uv, 1.0f };
		ringData->vertexData[vertexIndex + 2].texcoord = { uvNext, 1.0f };
		ringData->vertexData[vertexIndex + 3].texcoord = { uv, 0.0f };
		ringData->vertexData[vertexIndex + 4].texcoord = { uvNext, 1.0f };
		ringData->vertexData[vertexIndex + 5].texcoord = { uvNext, 0.0f };
		// 法線
		for (int i = 0; i < 6; ++i) {
			ringData->vertexData[vertexIndex + i].normal = { 0.0f, 0.0f, 1.0f };
		}
		vertexIndex += kVerticesPerSegment;
	}
}
//============================================================================
// マテリアル作成
//============================================================================
void Ring::CreateMaterial(const std::string& textureFilePath, RingData* ringData) {
	ringData->material = std::make_unique<Material>();
	ringData->material->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	ringData->material->SetEnableLighting(false);
	ringData->material->SetEnvCoefficient(0.0f);
	ringData->material->SetMaterialColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	ringData->material->SetUvScale({ 2.5f, 0.5f, 1.0f });
}

//============================================================================
// 描画処理(パーティクル用)
//============================================================================
void Ring::DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle) {

	// インスタンス数が0の場合は早期リターン
	if (instanceCount == 0) {
		return;
	}
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& ringData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &ringData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		ringData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(ringData->material->GetTextureFilePath()));

	//このRing固有の頂点数を使用
	commandList->DrawInstanced(ringData->vertexCount, instanceCount, 0, 0);
}

//============================================================================
// 描画処理（オブジェクト用）
//============================================================================
void Ring::DrawObject(PSO* pso, uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;
	}
	auto& ringData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &ringData->mesh.vertexBufferView_);
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		ringData->material->GetMaterialResource()->GetGPUVirtualAddress());
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(ringData->material->GetTextureFilePath()));
	// ★この Ring 固有の頂点数を使用（インスタンス数は1）
	commandList->DrawInstanced(ringData->vertexCount, 1, 0, 0);
}
//============================================================================
// データ取得
//============================================================================
Ring::RingData* Ring::GetData(uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return nullptr;
	}
	return it->second.get();
}
//============================================================================
// マテリアル色設定
//============================================================================
void Ring::SetMaterialColor(uint32_t handle, const Vector4& color) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;
	}
	it->second->material->SetMaterialColor(color);
}