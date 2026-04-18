#include "Plane.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/ImGuiManager.h"

using namespace TakeC;

//============================================================================
// planeデータの生成
//============================================================================

uint32_t Plane::Generate(float width, float height, const std::string& textureFilePath) {
	auto planeData = std::make_unique<PlaneData>();
	planeData->width = width;
	planeData->height = height;
	// 頂点データ作成
	CreateVertexData(planeData.get());
	// マテリアル作成
	CreateMaterial(planeData.get(), textureFilePath);
	// ハンドルを発行して登録
	uint32_t handle = RegisterData(std::move(planeData));
	return handle;
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

void TakeC::Plane::EditPrimitiveData(PlaneData* planeData) {

	ImGui::Text("Plane Parameters");
	ImGui::DragFloat("Width", &planeData->width, 0.1f);
	ImGui::DragFloat("Height", &planeData->height, 0.1f);
}
