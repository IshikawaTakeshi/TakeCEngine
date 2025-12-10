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
	mesh_->InitializeVertexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice(),spriteConfig_.anchorPoint_);
	//IndexResource初期化
	mesh_->InitializeIndexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice());

	//テクスチャ番号の検索と記録
	spriteConfig_.textureFilePath_ = filePath;

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
		spriteConfig_.position_.x *= WinApp::widthPercent_;
		spriteConfig_.position_.y *= WinApp::heightPercent_;
		firstUpdate_ = false;
	}

	//SpriteAnimatorの更新
	spriteAnimator_->Update();

	//Transformの更新
	transform_.translate = Vector3{ spriteConfig_.position_.x,spriteConfig_.position_.y,0.0f };
	transform_.rotate = Vector3{ 0.0f,0.0f,spriteConfig_.rotation_ };
	transform_.scale = Vector3{ spriteConfig_.size_.x,spriteConfig_.size_.y,1.0f };

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
		spriteConfig_.UpdateImGui();
		ImGui::Checkbox("isFlipX", &isFlipX_);
		ImGui::Checkbox("isFlipY", &isFlipY_);
		ImGui::Checkbox("adjustSwitch", &adjustSwitch_);
		mesh_->GetMaterial()->UpdateMaterialImGui();

		//設定保存
		if (ImGui::Button("Save Config"))
		{
			ImGui::OpenPopup("Save Camera");
		}

		// 保存ポップアップ
		if (ImGui::BeginPopupModal("Save Camera", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char filenameBuf[256] = "SpriteConfig.json";

			//jsonFilePathがある場合はfileNameBufにセット
			if(!spriteConfig_.jsonFilePath.empty()) {
				strncpy_s(filenameBuf, sizeof(filenameBuf),
					spriteConfig_.jsonFilePath.c_str(),
					_TRUNCATE);
			}
			
			ImGui::Text("Input filename for camera data:");
			ImGui::InputText("Filename", filenameBuf, sizeof(filenameBuf));

			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				// ファイル保存
				spriteConfig_.jsonFilePath = std::string(filenameBuf);
				TakeCFrameWork::GetJsonLoader()->SaveJsonData<SpriteConfig>(spriteConfig_.jsonFilePath,spriteConfig_);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
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
	float left = 0.0f - spriteConfig_.anchorPoint_.x;
	float right = 1.0f - spriteConfig_.anchorPoint_.x;
	float top = 0.0f - spriteConfig_.anchorPoint_.y;
	float bottom = 1.0f - spriteConfig_.anchorPoint_.y;
	//テクスチャ範囲指定
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance().GetMetadata(spriteConfig_.textureFilePath_);
	float tex_left = spriteConfig_.textureLeftTop_.x / metadata.width;
	float tex_top = spriteConfig_.textureLeftTop_.y / metadata.height;
	float tex_right = (spriteConfig_.textureLeftTop_.x + spriteConfig_.textureSize_.x) / metadata.width;
	float tex_bottom = (spriteConfig_.textureLeftTop_.y + spriteConfig_.textureSize_.y) / metadata.height;

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
	spriteConfig_.size_.x *= WinApp::widthPercent_;
	spriteConfig_.size_.y *= WinApp::heightPercent_;
}

void Sprite::SetFilePath(const std::string& filePath) {
	spriteConfig_.textureFilePath_ = filePath;
	mesh_->GetMaterial()->SetTextureFilePath(filePath);

}

//=============================================================================================
// テクスチャサイズをイメージに合わせる
//=============================================================================================
void Sprite::AdjustTextureSize() {

	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance().GetMetadata(spriteConfig_.textureFilePath_);

	spriteConfig_.textureSize_.x = static_cast<float>(metadata.width);
	spriteConfig_.textureSize_.y = static_cast<float>(metadata.height);

	//画像サイズをテクスチャサイズに合わせる
	spriteConfig_.size_ = spriteConfig_.textureSize_;
}

void Sprite::LoadConfig(const std::string& jsonFilePath) {

	//jsonファイルから設定読み込み
	spriteConfig_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<SpriteConfig>(jsonFilePath);
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
	//gTextureの設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance().GetSrvHandleGPU(spriteConfig_.textureFilePath_));
	//IBVの設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}