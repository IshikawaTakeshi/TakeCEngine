#include "Object3d.h"
#include "Object3dCommon.h"
#include "MyMath/MatrixMath.h"
#include "Include/DirectXCommon.h"
#include "Include/Mesh.h"
#include "TextureManager.h"
#include "ModelData/Model.h"
#include <fstream>
#include <sstream>
#include <cassert>

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif 
#pragma endregion

Object3d::~Object3d() {
	
	directionalLightResource_.Reset();
	wvpResource_.Reset();

}

void Object3d::Initialize(Object3dCommon* object3dCommon, Matrix4x4 cameraView, Model* model) {
	object3dCommon_ = object3dCommon;
	model_ = model;
	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));

	//単位行列を書き込んでおく
	transformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(DirectionalLightData));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;
	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::Inverse(cameraView);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f
	);
}

void Object3d::Update(int num) {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;
	//ImGuiの更新
	std::string windowName = "Object3d" + std::to_string(num);
	ImGui::Begin(windowName.c_str());
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::End();
}

void Object3d::Draw() {

	//wvp用のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	//Lighting用のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	if(model_ != nullptr) {
		model_->Draw();
	}
}

ModelData Object3d::LoadObjFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename) {
	ModelData modelData; //構築するModelData
	std::vector<Vector4> positions; //位置
	std::vector<Vector3> normals; //法線
	std::vector<Vector2> texcoords; //テクスチャ座標
	std::string line; //ファイルから読んだ1行を格納するもの
	std::ifstream file(resourceDirectoryPath + "/" + modelDirectoryPath + "/" + filename); //ファイルを開く
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
		} else if (identifier == "vn") {

			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (identifier == "f") {

			VertexData triangle[3];
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
				position.x *= -1.0f;
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				texcoord.y = 1.0f - texcoord.y;
				Vector3 normal = normals[elementIndices[2] - 1];
				normal.x *= -1.0f;
				VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}

			//頂点を逆順で登録することで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {

			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFileName;
			s >> materialFileName;

			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMtlFile(resourceDirectoryPath, modelDirectoryPath, materialFileName);
			modelData.material.textureFilePath = modelData.material.textureFilePath;
		}

	}

	return modelData;
}

ModelMaterialData Object3d::LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename) {

	ModelMaterialData materialData; //構築するMaterialData
	std::string line; //ファイルから読んだ1行を核のするもの
	std::ifstream file(resourceDirectoryPath + "/" + modelDirectoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFileName;
			s >> textureFileName;

			//連結してファイルパスにする
			materialData.textureFilePath = resourceDirectoryPath + "/" + textureFileName;
		}
	}

	return materialData;
}
