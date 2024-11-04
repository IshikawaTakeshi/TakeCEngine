#include "Mesh.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include <numbers>

Mesh::~Mesh() {
	delete material_;
	material_ = nullptr;
}

void Mesh::InitializeMesh(DirectXCommon* dxCommon,const std::string& filePath) {

	material_ = new Material();
	material_->InitializeTexture(dxCommon, filePath);
}

void Mesh::InitializeVertexResourceSphere(ID3D12Device* device) {

	// 頂点数の設定
	uint32_t vertexCount = (kSubdivision * kSubdivision) * 6;

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度

	//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; //現在の緯度(シータ)

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; //現在の経度(ファイ)
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			//1枚目の三角形
			//頂点データの入力。基準点a(左下)
			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;

			//基準点b1(左上)
			vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + 1].position.y = sin(lat + kLatEvery);
			vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start + 1].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;

			//基準点c1(右下)
			vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + 2].position.y = sin(lat);
			vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 2].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;

			//基準点d(右上)
			vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData[start + 3].position.y = sin(lat + kLatEvery);
			vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 3].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;

		}
	}
}

void Mesh::InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) {

	//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 4);
	//頂点バッファビューの作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//anchorPoint
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;
	//1枚目の三角形
	vertexData[0].position = { left,bottom,0.0f,1.0f }; //左下
	vertexData[1].position = { left,top,0.0f,1.0f }; //左上
	vertexData[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData[3].position = { right,top,0.0f,1.0f }; //右上
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
}

void Mesh::InitializeVertexResourceTriangle(ID3D12Device* device) {

	//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 3);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

}

void Mesh::InitializeVertexResourceObjModel(ID3D12Device* device, ModelData modelData) {

	//頂点リソースを作る
	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作る
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//リソースにデータを書き込む

	//頂点データ
	VertexData* vertexData;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void Mesh::InitializeVertexResourceAABB(ID3D12Device* device) {

	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 24);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 24;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData;
	Vector3 min_ = { 0.0f,1.0f,1.0f };
	Vector3 max_ = { 2.0f,2.0f,2.0f };
	min_.x = (std::min)(min_.x, max_.x);
	max_.x = (std::max)(min_.x, max_.x);
	min_.y = (std::min)(min_.y, max_.y);
	max_.y = (std::max)(min_.y, max_.y);
	min_.z = (std::min)(min_.z, max_.z);
	max_.z = (std::max)(min_.z, max_.z);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// Front face
	vertexData[0] = { Vector4(min_.x, min_.y, min_.z), Vector2(0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f) };
	vertexData[1] = { Vector4(max_.x, min_.y, min_.z), Vector2(1.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f) };
	vertexData[2] = { Vector4(min_.x, max_.y, min_.z), Vector2(0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f) };
	vertexData[3] = { Vector4(max_.x, max_.y, min_.z), Vector2(1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f) };

	// Back face
	vertexData[4] = { Vector4(min_.x, min_.y, max_.z), Vector2(0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) };
	vertexData[5] = { Vector4(max_.x, min_.y, max_.z), Vector2(1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) };
	vertexData[6] = { Vector4(min_.x, max_.y, max_.z), Vector2(0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) };
	vertexData[7] = { Vector4(max_.x, max_.y, max_.z), Vector2(1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) };

	// 左側面 (左方向に法線)
	vertexData[8] = { Vector4(min_.x, min_.y, min_.z), Vector2(0.0f, 1.0f), Vector3(-1.0f, 0.0f, 0.0f) };
	vertexData[9] = { Vector4(min_.x, max_.y, min_.z), Vector2(0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f) };
	vertexData[10] = { Vector4(min_.x, min_.y, max_.z), Vector2(1.0f, 1.0f), Vector3(-1.0f, 0.0f, 0.0f) };
	vertexData[11] = { Vector4(min_.x, max_.y, max_.z), Vector2(1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f) };

	// 右側面 (右方向に法線)
	vertexData[12] = { Vector4(max_.x, min_.y, min_.z), Vector2(0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f) };
	vertexData[13] = { Vector4(max_.x, max_.y, min_.z), Vector2(0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f) };
	vertexData[14] = { Vector4(max_.x, min_.y, max_.z), Vector2(1.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f) };
	vertexData[15] = { Vector4(max_.x, max_.y, max_.z), Vector2(1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f) };

	// 上面 (上方向に法線)
	vertexData[16] = { Vector4(min_.x, max_.y, min_.z), Vector2(0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f) };
	vertexData[17] = { Vector4(max_.x, max_.y, min_.z), Vector2(1.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f) };
	vertexData[18] = { Vector4(min_.x, max_.y, max_.z), Vector2(0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f) };
	vertexData[19] = { Vector4(max_.x, max_.y, max_.z), Vector2(1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f) };

	// 底面 (下方向に法線)
	vertexData[20] = { Vector4(min_.x, min_.y, min_.z), Vector2(0.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f) };
	vertexData[21] = { Vector4(max_.x, min_.y, min_.z), Vector2(1.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f) };
	vertexData[22] = { Vector4(min_.x, min_.y, max_.z), Vector2(0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f) };
	vertexData[23] = { Vector4(max_.x, min_.y, max_.z), Vector2(1.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f) };
}

void Mesh::InitializeIndexResourceAABB(ID3D12Device* device) {

	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * 8);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 8;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2; indexData[3] = 3;
	indexData[4] = 4; indexData[5] = 5; indexData[6] = 6; indexData[7] = 7;

}


void Mesh::InitializeIndexResourceSphere(ID3D12Device* device) {

	// インデックスバッファのサイズを設定
	uint32_t indexCount = (kSubdivision * kSubdivision) * 6;
	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * indexCount);

	// インデックスバッファビューの設定
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 最初の三角形のインデックス
			indexData[start] = start;         //左下
			indexData[start + 1] = start + 1; //左上
			indexData[start + 2] = start + 2; //右下

			// 二つ目の三角形のインデックス
			indexData[start + 3] = start + 1; //左上
			indexData[start + 4] = start + 3; //右上
			indexData[start + 5] = start + 2; //右下
		}
	}
}

void Mesh::InitializeIndexResourceSprite(ID3D12Device* device) {

	//リソースの作成
	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}


