#include "Cylinder.h"
#include "engine/Base/ImGuiManager.h"


uint32_t TakeC::Cylinder::Generate(float radius, float height, uint32_t subDivision, const std::string& textureFilePath) {
	auto cylinder = std::make_unique<CylinderData>();
	cylinder->radius = radius;
	cylinder->height = height;
	cylinder->subDivision = subDivision;
	// 頂点データ作成
	CreateVertexData(cylinder.get());
	// マテリアル作成
	CreateMaterial(cylinder.get(), textureFilePath);
	// ハンドルを発行して登録
	uint32_t handle = RegisterData(std::move(cylinder));
	return handle;
}

//============================================================================
// 頂点データ作成
//============================================================================
void TakeC::Cylinder::CreateVertexData(CylinderData* cylinderData) {

	auto CreateCylinderVertex = [&](const Vector3& pos, float u, float v, const Vector3& normal) {
		VertexData vertex{};
		vertex.position = { pos.x, pos.y, pos.z, 1.0f };
		vertex.normal = normal;
		vertex.texcoord = { u, v };
		return vertex;
		};

	const uint32_t kSubdivision = cylinderData->subDivision;  // 円の分割数
	const float kAngleEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);

	// 頂点数 = 側面(kSubdivision * 6) + 上面(kSubdivision * 3) + 底面(kSubdivision * 3)
	uint32_t vertexCount = kSubdivision * 6 + kSubdivision * 3 + kSubdivision * 3;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	cylinderData->mesh.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	cylinderData->mesh. vertexBuffer_->SetName(L"Cylinder:: vertexResource_");

	// bufferview設定
	cylinderData->mesh.vertexBufferView_. BufferLocation = cylinderData->mesh.vertexBuffer_->GetGPUVirtualAddress();
	cylinderData->mesh.vertexBufferView_. StrideInBytes = sizeof(VertexData);
	cylinderData->mesh.vertexBufferView_.SizeInBytes = size;

	// mapping
	cylinderData->mesh.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cylinderData->vertexData));

	VertexData* vertexData = cylinderData->vertexData;
	uint32_t vertexIndex = 0;

	// 円柱の中心位置
	float halfHeight = cylinderData->height * 0.5f;
	Vector3 topCenter = { 0.0f, halfHeight, 0.0f };
	Vector3 bottomCenter = { 0.0f, -halfHeight, 0.0f };

	// ===== 側面の生成 =====
	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 上面の円周上の点
		Vector3 topPos = {
			cylinderData->radius * cosf(angle),
			halfHeight,
			cylinderData->radius * sinf(angle)
		};

		Vector3 topPosNext = {
			cylinderData->radius * cosf(angleNext),
			halfHeight,
			cylinderData->radius * sinf(angleNext)
		};

		// 底面の円周上の点
		Vector3 bottomPos = {
			cylinderData->radius * cosf(angle),
			-halfHeight,
			cylinderData->radius * sinf(angle)
		};

		Vector3 bottomPosNext = {
			cylinderData->radius * cosf(angleNext),
			-halfHeight,
			cylinderData->radius * sinf(angleNext)
		};

		// 側面の法線（外向き）
		Vector3 normal = {
			cosf(angle),
			0.0f,
			sinf(angle)
		};

		Vector3 normalNext = {
			cosf(angleNext),
			0.0f,
			sinf(angleNext)
		};

		// UV座標
		float u = static_cast<float>(i) / static_cast<float>(kSubdivision);
		float uNext = static_cast<float>(i + 1) / static_cast<float>(kSubdivision);

		// 側面の四角形を2つの三角形で構成（反時計回り）
		// 第1三角形:  底面現在 -> 上面現在 -> 底面次
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomPos, u, 1.0f, normal);
		vertexData[vertexIndex++] = CreateCylinderVertex(topPos, u, 0.0f, normal);
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomPosNext, uNext, 1.0f, normalNext);

		// 第2三角形: 底面次 -> 上面現在 -> 上面次
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomPosNext, uNext, 1.0f, normalNext);
		vertexData[vertexIndex++] = CreateCylinderVertex(topPos, u, 0.0f, normal);
		vertexData[vertexIndex++] = CreateCylinderVertex(topPosNext, uNext, 0.0f, normalNext);
	}

	// ===== 上面の生成 =====
	Vector3 topNormal = { 0.0f, 1.0f, 0.0f };  // 上面の法線は上向き

	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 上面の円周上の点
		Vector3 topPos = {
			cylinderData->radius * cosf(angle),
			halfHeight,
			cylinderData->radius * sinf(angle)
		};

		Vector3 topPosNext = {
			cylinderData->radius * cosf(angleNext),
			halfHeight,
			cylinderData->radius * sinf(angleNext)
		};

		// テクスチャ座標（0.5を中心とした円）
		float u = 0.5f + 0.5f * cosf(angle);
		float v = 0.5f + 0.5f * sinf(angle);
		float uNext = 0.5f + 0.5f * cosf(angleNext);
		float vNext = 0.5f + 0.5f * sinf(angleNext);

		// 上面の三角形（反時計回り）:  中心 -> 現在 -> 次
		vertexData[vertexIndex++] = CreateCylinderVertex(topCenter, 0.5f, 0.5f, topNormal);
		vertexData[vertexIndex++] = CreateCylinderVertex(topPos, u, v, topNormal);
		vertexData[vertexIndex++] = CreateCylinderVertex(topPosNext, uNext, vNext, topNormal);
	}

	// ===== 底面の生成 =====
	Vector3 bottomNormal = { 0.0f, -1.0f, 0.0f };  // 底面の法線は下向き

	for (uint32_t i = 0; i < kSubdivision; ++i) {
		float angle = i * kAngleEvery;
		float angleNext = (i + 1) * kAngleEvery;

		// 底面の円周上の点
		Vector3 bottomPos = {
			cylinderData->radius * cosf(angle),
			-halfHeight,
			cylinderData->radius * sinf(angle)
		};

		Vector3 bottomPosNext = {
			cylinderData->radius * cosf(angleNext),
			-halfHeight,
			cylinderData->radius * sinf(angleNext)
		};

		// テクスチャ座標（0.5を中心とした円）
		float u = 0.5f + 0.5f * cosf(angle);
		float v = 0.5f + 0.5f * sinf(angle);
		float uNext = 0.5f + 0.5f * cosf(angleNext);
		float vNext = 0.5f + 0.5f * sinf(angleNext);

		// 底面の三角形（反時計回り）: 中心 -> 次 -> 現在
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomCenter, 0.5f, 0.5f, bottomNormal);
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomPosNext, uNext, vNext, bottomNormal);
		vertexData[vertexIndex++] = CreateCylinderVertex(bottomPos, u, v, bottomNormal);
	}

	cylinderData->vertexCount += vertexIndex;
}

//============================================================================
// プリミティブデータ編集
//============================================================================
void TakeC::Cylinder::EditPrimitiveData(CylinderData* data) {
	ImGui::DragFloat("Radius", &data->radius, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Height", &data->height, 0.1f, 0.1f, 100.0f);
	ImGui::DragInt("Subdivision", reinterpret_cast<int*>(&data->subDivision), 1, 3, 128);
}
