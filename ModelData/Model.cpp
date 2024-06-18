#include "../ModelData/Model.h"
#include "../Include/DirectXCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>

void Model::Initialize(DirectXCommon* dxCommon, const std::string& directoryPath, const std::string& filename) {

	//モデル読み込み
	ModelData modelData = LoadObjFile(directoryPath,filename);

	InitializeVertexData(dxCommon);
}

void Model::DrawCall(DirectXCommon* dxCommon) {

	dxCommon->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

void Model::InitializeVertexData(DirectXCommon* dxCommon) {

	//頂点リソースを作る
	vertexResource_ = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	//頂点バッファビューを作る
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//リソースにデータを書き込む
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; //構築するModelData
	std::vector<Vector4> positions; //位置
	std::vector<Vector3> normals; //法線
	std::vector<Vector2> texcoords; //テクスチャ座標
	std::string line; //ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); //ファイルを開く
	assert(file.is_open()); //開けれなかったら止める

	while (std::getline(file, line)) {
		std::string identifier; 
		std::istringstream s(line);
		s >> identifier; //先頭の識別子を読む

		//頂点情報を読む
		if (identifier == "v") { //"v" 頂点情報

			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") { //"vt" 頂点テクスチャ座標

			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if(identifier == "vn"){
			 
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (identifier == "f") {

			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (uint32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); //区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}

				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);
			}
		}

	}

	return modelData;
}
