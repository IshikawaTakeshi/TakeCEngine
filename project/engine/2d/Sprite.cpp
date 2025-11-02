#include "Sprite.h"
#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "Material.h"
#include "MatrixMath.h"
#include "TextureManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"

//==================================================================================
// 初期化
//==================================================================================
void Sprite::Initialize(SpriteCommon* spriteCommon, const std::string& filePath) {

	//SpriteCommonの設定
	spriteCommon_ = spriteCommon;
	
	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(spriteCommon_->GetDirectXCommon(),filePath);
	//vertexResource初期化
	mesh_->InitializeVertexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice(),anchorPoint_);
	//IndexResource初期化
	mesh_->InitializeIndexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice());

	//テクスチャ番号の検索と記録
	filePath_ = filePath;

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(spriteCommon->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

	//単位行列を書き込んでおく
	wvpData_->WVP = MatrixMath::MakeIdentity4x4();

	//======================= Transform・各行列の初期化 ===========================//

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//SpriteAnimatorの初期化(targetの設定)
	spriteAnimator_ = std::make_unique<SpriteAnimator>();
	spriteAnimator_->Initialize(this);

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::MakeIdentity4x4();
	projectionMatrix_ = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, WinApp::kScreenWidth, WinApp::kScreenHeight, 0.1f, 1000.0f);
	worldViewProjectionMatrix_ = worldMatrix_ * viewMatrix_ * projectionMatrix_;
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;
}

//=============================================================================================
// 更新処理
//=============================================================================================
void Sprite::Update() {

	if(firstUpdate_) {
		//初回更新時にサイズを相対サイズにする
		SetSizeRelative();
		position_.x *= WinApp::widthPercent_;
		position_.y *= WinApp::heightPercent_;
		firstUpdate_ = false;
	}

	//SpriteAnimatorの更新
	spriteAnimator_->Update(TakeCFrameWork::GetDeltaTime());

	//Transformの更新
	transform_.translate = Vector3{ position_.x,position_.y,0.0f };
	transform_.rotate = Vector3{ 0.0f,0.0f,rotation_ };
	transform_.scale = Vector3{ size_.x,size_.y,1.0f };

	//頂点データ更新
	UpdateVertexData();

	//テクスチャサイズをイメージに合わせる
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
}

//=============================================================================================
// ImGuiの更新
//=============================================================================================
void Sprite::UpdateImGui([[maybe_unused]]const std::string& name) {
#ifdef _DEBUG
	//ImGuiの更新
	std::string windowName = "Sprite" + name;
	ImGui::Begin("Sprite");
	if (ImGui::TreeNode(windowName.c_str())) {
		ImGui::DragFloat2("SpriteTranslate", &position_.x, 1);
		ImGui::DragFloat("SpriteRotation", &rotation_, 0.01f);
		ImGui::DragFloat2("TextureLeftTop", &textureLeftTop_.x, 0.01f);
		ImGui::DragFloat2("TextureSize", &textureSize_.x, 0.01f);
		ImGui::DragFloat2("SpriteSize", &size_.x, 1);
		ImGui::SliderFloat2("AnchorPoint", &anchorPoint_.x, -1.0f, 1.0f);
		ImGui::Checkbox("isFlipX", &isFlipX_);
		ImGui::Checkbox("isFlipY", &isFlipY_);
		ImGui::Checkbox("adjustSwitch", &adjustSwitch_);
		mesh_->GetMaterial()->UpdateMaterialImGui();
		ImGui::TreePop();
	}
	ImGui::End();
#endif // DEBUG
}

//=============================================================================================
// 頂点データ更新
//=============================================================================================
void Sprite::UpdateVertexData() {
	//頂点データ
	VertexData* vertexData;
	mesh_->GetInputVertexResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//anchorPoint
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;
	//テクスチャ範囲指定
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(filePath_);
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

//=============================================================================================
// サイズを画面サイズに対する相対サイズにする
//=============================================================================================
void Sprite::SetSizeRelative() {
	size_.x *= WinApp::widthPercent_;
	size_.y *= WinApp::heightPercent_;
}

//=============================================================================================
// テクスチャサイズをイメージに合わせる
//=============================================================================================
void Sprite::AdjustTextureSize() {

	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(filePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	//画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}


//=============================================================================================
// 描画処理
//=============================================================================================
void Sprite::Draw() {
	//spriteの描画
	mesh_->SetVertexBuffers(spriteCommon_->GetDirectXCommon()->GetCommandList(), 0);
	//TransformationMatrixCBufferの場所の設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());
	//gMaterial
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		1, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//gTexureの設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath_));
	//IBVの設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}