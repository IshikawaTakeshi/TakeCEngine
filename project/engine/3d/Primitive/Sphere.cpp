#include "Sphere.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

//============================================================================
// 初期化
//============================================================================

void Sphere::Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// sphereデータの生成
//============================================================================

uint32_t Sphere::Generate(float radius, const std::string& textureFilePath) {
	auto sphere = std::make_unique<SphereData>();
	sphere->radius = radius;
	// 頂点データ作成
	CreateVertexData(sphere.get());
	// マテリアル作成
	CreateMaterial(textureFilePath, sphere.get());
	// ハンドルを発行して登録
	uint32_t handle = nextHandle_++;
	datas_[handle] = std::move(sphere);
	return handle;
}

//============================================================================
// 描画処理(パーティクル用)
//============================================================================

void Sphere::DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle) {
	// インスタンス数が0の場合は早期リターン
	if (instanceCount == 0) {
		return;
	}
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& sphereData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &sphereData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		sphereData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(sphereData->material->GetTextureFilePath()));

	//このsphereData固有の頂点数を使用
	commandList->DrawInstanced(sphereData->vertexCount, instanceCount, 0, 0);
}

//============================================================================
// 描画処理（オブジェクト用）
//============================================================================

void Sphere::DrawObject(PSO* pso, uint32_t handle) {

	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	auto& sphereData = it->second;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VertexBufferView 設定
	commandList->IASetVertexBuffers(0, 1, &sphereData->mesh.vertexBufferView_);
	// マテリアル CBuffer 設定
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gMaterial"),
		sphereData->material->GetMaterialResource()->GetGPUVirtualAddress());
	// テクスチャ設定
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance().GetSrvIndex(sphereData->material->GetTextureFilePath()));
	//このSphere固有の頂点数を使用
	commandList->DrawInstanced(sphereData->vertexCount, 1, 0, 0);
}

//============================================================================
// データ取得
//============================================================================

Sphere::SphereData* Sphere::GetData(uint32_t handle) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return nullptr;  // ハンドルが無効
	}
	return it->second.get();
}

//============================================================================
// マテリアル色設定
//============================================================================

void Sphere::SetMaterialColor(uint32_t handle, const Vector4& color) {
	auto it = datas_.find(handle);
	if (it == datas_.end()) {
		return;  // ハンドルが無効
	}
	it->second->material->GetMaterialData()->color = color;
}

void TakeC::Sphere::CreateVertexData(SphereData* sphereData) {
	
	auto CreateSphereVertex = [&](const Vector3& pos, uint32_t lonIndex, uint32_t latIndex, uint32_t kSubdivision, float radius) {
		VertexData v{};
		v.position = { pos.x, pos.y, pos.z, 1.0f };
		v.normal = { pos.x / radius, pos.y / radius, pos.z / radius };
		v.texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
		v.texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
		return v;
		};

	const uint32_t kSubdivision = sphereData->subDivision; // 分割数
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision);         // 緯度刻み
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);  // 経度刻み

	// 頂点数 = 1マス6頂点 × マス数
	uint32_t vertexCount = kSubdivision * kSubdivision * 6;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	sphereData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	sphereData->mesh.vertexBuffer_->SetName(L"Sphere::vertexResource_");

	// bufferview設定
	sphereData->mesh.vertexBufferView_.BufferLocation = sphereData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	sphereData->mesh.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	sphereData->mesh.vertexBufferView_.SizeInBytes = size;

	// mapping
	sphereData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&sphereData->vertexData));

	VertexData* vertexData = sphereData->vertexData;
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
				sphereData->radius * cosf(lat) * cosf(lon),
				sphereData->radius * sinf(lat),
				sphereData->radius * cosf(lat) * sinf(lon)
			};

			// b: 左上
			Vector3 bPos = {
				sphereData->radius * cosf(latNext) * cosf(lon),
				sphereData->radius * sinf(latNext),
				sphereData->radius * cosf(latNext) * sinf(lon)
			};

			// c: 右下
			Vector3 cPos = {
				sphereData->radius * cosf(lat) * cosf(lonNext),
				sphereData->radius * sinf(lat),
				sphereData->radius * cosf(lat) * sinf(lonNext)
			};

			// d: 右上
			Vector3 dPos = {
				sphereData->radius * cosf(latNext) * cosf(lonNext),
				sphereData->radius * sinf(latNext),
				sphereData->radius * cosf(latNext) * sinf(lonNext)
			};

			// 三角形1 (a, b, c)
			vertexData[vertexIndex++] = CreateSphereVertex(aPos, lonIndex, latIndex, kSubdivision, sphereData->radius);
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex, latIndex + 1, kSubdivision, sphereData->radius);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex, kSubdivision, sphereData->radius);

			// 三角形2 (b, d, c)
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex, latIndex + 1, kSubdivision, sphereData->radius);
			vertexData[vertexIndex++] = CreateSphereVertex(dPos, lonIndex + 1, latIndex + 1, kSubdivision, sphereData->radius);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex, kSubdivision, sphereData->radius);
		}
	}

}

void TakeC::Sphere::CreateMaterial(const std::string& textureFilePath, SphereData* sphereData) {

	sphereData->material = std::make_unique<Material>();
	sphereData->material->Initialize(dxCommon_, textureFilePath, "");
	sphereData->material->InitializeMaterialResource(dxCommon_->GetDevice());
}
