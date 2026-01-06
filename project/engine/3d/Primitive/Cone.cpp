#include "Cone.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"


using namespace TakeC;

//============================================================================
// 初期化
//============================================================================

void Cone::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// cone生成
//============================================================================

uint32_t TakeC::Cone::Generate(float radius, float height, uint32_t subDivision, const std::string& textureFilePath) {
	auto coneData = std::make_unique<ConeData>();
	coneData->radius = radius;
	coneData->height = height;
	coneData->subDivision = subDivision;
	// 頂点データ作成
	CreateVertexData(coneData.get());
	// マテリアル作成
	CreateMaterial(textureFilePath, coneData.get());
	// ハンドルを発行して登録
	uint32_t handle = nextHandle_++;
	datas_[handle] = std::move(coneData);
	return handle;
}

//============================================================================
// 頂点データ作成
//============================================================================

void TakeC::Cone::CreateVertexData(ConeData* coneData) {

	auto CreateConeVertex = [&](const Vector3& pos, float u, float v, const Vector3& normal) {
		VertexData vertex{};
		vertex.position = { pos.x, pos.y, pos.z, 1.0f };
		vertex.normal = normal;
		vertex.texcoord = { u, v };
		return vertex;
		};

	const uint32_t kSubdivision = coneData->subDivision;  // 円の分割数
	const float kAngleEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);

	// 頂点数 = 側面(kSubdivision * 3) + 底面(kSubdivision * 3)
	uint32_t vertexCount = kSubdivision * 3 + kSubdivision * 3;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	coneData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	coneData->mesh.vertexBuffer_->SetName(L"Cone::vertexResource_");

	// bufferview設定
	coneData->mesh.vertexBufferView_.BufferLocation = coneData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	coneData->mesh.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	coneData->mesh.vertexBufferView_.SizeInBytes = size;

	// mapping
	coneData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&coneData->vertexData));

	VertexData* vertexData = coneData->vertexData;
	uint32_t vertexIndex = 0;

	// 円錐の頂点位置（上部）
	Vector3 apex = { 0.0f, coneData->height, 0.0f };

	// 底面の中心位置
	Vector3 baseCenter = { 0.0f, 0.0f, 0.0f };

	// ===== 側面の生成 =====
	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 底面の円周上の点
		Vector3 basePos = {
			coneData->radius * cosf(angle),
			0.0f,
			coneData->radius * sinf(angle)
		};

		Vector3 basePosNext = {
			coneData->radius * cosf(angleNext),
			0.0f,
			coneData->radius * sinf(angleNext)
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
			coneData->radius * cosf(angle),
			0.0f,
			coneData->radius * sinf(angle)
		};

		Vector3 basePosNext = {
			coneData->radius * cosf(angleNext),
			0.0f,
			coneData->radius * sinf(angleNext)
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

	coneData->vertexCount += vertexIndex;
}

//============================================================================
// マテリアル作成関数
//============================================================================

void TakeC::Cone::CreateMaterial(const std::string& textureFilePath, ConeData* coneData) {

	coneData->material = std::make_unique<Material>();
	coneData->material->Initialize(dxCommon_, textureFilePath, "");
}

//============================================================================
// 描画処理(パーティクル用)
//============================================================================

void Cone::DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle) {
	// インスタンス数が0の場合は早期リターン
	if (instanceCount == 0) {
		return;
	}
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& data = it->second;
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
	// 描画コマンド
	commandList->DrawInstanced(data->vertexCount, instanceCount, 0, 0);
}

//============================================================================
// 描画処理（オブジェクト用）
//============================================================================

void Cone::DrawObject(PSO* pso, uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& data = it->second;
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
	// 描画コマンド
	commandList->DrawInstanced(data->vertexCount, 1, 0, 0);
}

//============================================================================
// データ取得
//============================================================================

Cone::ConeData* Cone::GetData(uint32_t handle) {
	auto it = datas_.find(handle);
	if (it != datas_.end()) {
		return it->second.get();
	}
	return nullptr;
}

//============================================================================
// マテリアル色設定
//============================================================================

void Cone::SetMaterialColor(uint32_t handle, const Vector4& color) {
	auto it = datas_.find(handle);
	if (it != datas_.end()) {
		it->second->material->SetMaterialColor(color);
	}
}