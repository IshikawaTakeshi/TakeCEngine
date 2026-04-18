#include "Cube.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

//============================================================================
// cube生成
//============================================================================

uint32_t TakeC::Cube::Generate(const AABB& size, const std::string& textureFilePath) {
	
	auto cube = std::make_unique<CubeData>();
	cube->size = size;
	// 頂点データ作成
	CreateVertexData(cube.get());
	// マテリアル作成
	CreateMaterial(cube.get(),textureFilePath);
	// ハンドルを発行して登録
	uint32_t handle = RegisterData(std::move(cube));
	return handle;
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

void TakeC::Cube::EditPrimitiveData(CubeData* data) {

	ImGui::Text("Cube Parameters");
	ImGui::DragFloat3("Min", &data->size.min.x, 0.1f);
	ImGui::DragFloat3("Max", &data->size.max.x, 0.1f);
}
