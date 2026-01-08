#include "Ring.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Base/TextureManager.h"

using namespace TakeC;

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
	CreateMaterial(ring.get(),textureFilePath);
	// ハンドルを発行して登録
	uint32_t handle = RegisterData(std::move(ring));
	return handle;
}

//============================================================================
// 頂点データ作成
//============================================================================
void Ring::CreateVertexData(RingData* ringData) {
	// この Ring 固有の頂点数を計算
	uint32_t vertexCount = ringData->subDivision * kVerticesPerSegment;
	ringData->vertexCount = vertexCount;  // 個別に保存
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
// プリミティブデータ編集
//============================================================================
void TakeC::Ring::EditPrimitiveData(RingData* data) {

	ImGui::Text("Ring Parameters");
	ImGui::DragFloat("Outer Radius", &data->outerRadius, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Inner Radius", &data->innerRadius, 0.1f, 0.01f, data->outerRadius - 0.01f);

}