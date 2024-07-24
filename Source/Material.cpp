#include "../Include/Material.h"
#include "../Include/DirectXCommon.h"
#include "../MyMath/MatrixMath.h"

#pragma region imgui
#ifdef _DEBUG

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif // DEBUG

#pragma endregion


Material::~Material() {}

void Material::InitializeTexture(uint32_t index, DirectXCommon* dxCommon, bool enableLight, const std::string& filePath) {

	//マテリアルリソース初期化
	InitializeMaterialResource(dxCommon->GetDevice(), enableLight);

	//テクスチャ初期化
	texture_ = new Texture();
	texture_->Initialize(index, dxCommon, filePath);

	//uvTransform
	uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
}

void Material::UpdateMaterial() {


#ifdef _DEBUG
	//ImGuiの更新
	ImGui::Begin("Material");
	ImGui::DragFloat2("UVTranslate", &uvTransform_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransform_.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransform_.rotate.z);
	Matrix4x4 uvTransformMatrix = MatrixMath::MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;
	ImGui::End();

#endif // DEBUG

}

void Material::InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device, bool enableLight) {

	//マテリアル用リソース作成
	materialResource_ = DirectXCommon::CreateBufferResource(device.Get(), sizeof(MaterialData));
	//materialにデータを書き込む
	materialData_ = nullptr;
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//色を書き込む
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->uvTransform = MatrixMath::MakeIdentity4x4();
	materialData_->enableLighting = enableLight;
}
