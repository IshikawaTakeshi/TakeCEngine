#include "../Sprite/Sprite.h"
#include "../Sprite/SpriteCommon.h"
#include "../Include/DirectXCommon.h"
#include "../Include/Mesh.h"
#include "../Include/Material.h"
#include "../MyMath/MatrixMath.h"
#include "../TextureManager.h"

#pragma region imgui
#ifdef _DEBUG

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif // DEBUG

#pragma endregion


Sprite::~Sprite() {

	directionalLightResource_.Reset();
	
}

#pragma region 初期化処理

void Sprite::Initialize(SpriteCommon* spriteCommon, const std::string& filePath) {

	//SpriteCommonの設定
	spriteCommon_ = spriteCommon;
	
	//メッシュ初期化
	mesh_ = new Mesh();
	mesh_->InitializeMesh(spriteCommon_->GetDirectXCommon(),filePath);
	//vertexResource初期化
	mesh_->InitializeVertexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice(),anchorPoint_);
	//IndexResource初期化
	mesh_->InitializeIndexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice());
	//MaterialResource初期化
	mesh_->GetMaterial()->InitializeMaterialResource(spriteCommon->GetDirectXCommon()->GetDevice());

	//テクスチャ番号の検索と記録
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(filePath);

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(spriteCommon->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->
		Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

	//単位行列を書き込んでおく
	wvpData_->WVP = MatrixMath::MakeIdentity4x4();

	
	//======================= DirectionalLightResource ===========================//

	InitializeDirectionalLightData(spriteCommon->GetDirectXCommon());

	//======================= Transform・各行列の初期化 ===========================//

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::MakeIdentity4x4();
	projectionMatrix_ = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update(int num) {

	transform_.translate = Vector3{ position_.x,position_.y,0.0f };
	transform_.rotate = Vector3{ 0.0f,0.0f,rotation_ };
	transform_.scale = Vector3{ size_.x,size_.y,1.0f };

	UpdateVertexData();

	if (adjustSwitch_) {
		AdjustTextureSize();
	}
	

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの処理
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;

#ifdef _DEBUG
	//ImGuiの更新
	std::string windowName = "Sprite" + std::to_string(num);
	ImGui::Begin(windowName.c_str());
	ImGui::DragFloat2("SpriteTranslate", &position_.x, 1);
	ImGui::DragFloat("SpriteRotation", &rotation_, 0.01f);
	ImGui::DragFloat2("SpriteScale", &size_.x, 1);
	ImGui::SliderFloat2("AnchorPoint", &anchorPoint_.x, -1.0f, 1.0f);
	ImGui::Checkbox("isFlipX", &isFlipX_);
	ImGui::Checkbox("isFlipY", &isFlipY_);
	ImGui::Checkbox("adjustSwitch", &adjustSwitch_);
	mesh_->GetMaterial()->UpdateMaterialImGui();
	ImGui::End();
	
#endif // DEBUG

}

void Sprite::UpdateVertexData() {
	//頂点データ
	VertexData* vertexData;
	mesh_->GetVertexResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//anchorPoint
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;
	//テクスチャ範囲指定
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(textureIndex_);
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	//1枚目の三角形
	vertexData[0].position = { left,bottom,0.0f,1.0f }; //左下
	vertexData[1].position = { left,top,0.0f,1.0f }; //左上
	vertexData[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData[3].position = { right,top,0.0f,1.0f }; //右上
	vertexData[0].texcoord = { tex_left,tex_bottom };
	vertexData[1].texcoord = { tex_left,tex_top };
	vertexData[2].texcoord = { tex_right,tex_bottom };
	vertexData[3].texcoord = { tex_right,tex_top };
}

void Sprite::AdjustTextureSize() {

	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(textureIndex_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	//画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}

#pragma endregion

void Sprite::InitializeDirectionalLightData(DirectXCommon* dxCommon) {

	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLightData));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;
}

#pragma region 描画処理
void Sprite::DrawCall() {
	//spriteの描画。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定
	//materialCBufferの場所を指定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//TransformationMatrixCBufferの場所の設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	//Lighting用のCBufferの場所を指定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//IBVの設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
#pragma endregion