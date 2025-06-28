#include "Sprite.h"
#include "2d/SpriteCommon.h"
#include "Material.h"
#include "math/MatrixMath.h"
#include "base/DirectXCommon.h"
#include "base/TextureManager.h"
#include "base/TakeCFrameWork.h"

Sprite::~Sprite() {
	spriteCommon_ = nullptr;
}

#pragma region 初期化処理

void Sprite::Initialize(SpriteCommon* spriteCommon, const std::string& filePath) {

	//SpriteCommonの設定
	spriteCommon_ = spriteCommon;
	
	//メッシュ初期化
	//Todo: primitiveDrawerからのメッシュ生成に変更
	
	//vertexResource初期化
	primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(1.0f, 1.0f, filePath);
	vertexData_ = new VertexData();

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

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::MakeIdentity4x4();
	projectionMatrix_ = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kScreenWidth), float(WinApp::kScreenHeight), 0.0f, 100.0f);
	worldViewProjectionMatrix_ = worldMatrix_ * viewMatrix_ * projectionMatrix_;
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update() {

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

	//TODO: primitiveDrawerからvertexResourceをMappingするように変更
	TakeCFrameWork::GetPrimitiveDrawer()->MappingPlaneVertexData(vertexData_, primitiveHandle_);
}

#ifdef _DEBUG
void Sprite::UpdateImGui(const std::string& name) {
	//ImGuiの更新
	std::string windowName = "Sprite" + name;
	ImGui::Begin("Sprite");
	if (ImGui::TreeNode(windowName.c_str())) {
		ImGui::DragFloat2("SpriteTranslate", &position_.x, 1);
		ImGui::DragFloat("SpriteRotation", &rotation_, 0.01f);
		ImGui::DragFloat2("SpriteScale", &size_.x, 1);
		ImGui::SliderFloat2("AnchorPoint", &anchorPoint_.x, -1.0f, 1.0f);
		ImGui::Checkbox("isFlipX", &isFlipX_);
		ImGui::Checkbox("isFlipY", &isFlipY_);
		ImGui::Checkbox("adjustSwitch", &adjustSwitch_);
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // DEBUG

void Sprite::UpdateVertexData() {

	
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
	vertexData_[0].position = { left,bottom,0.0f,1.0f }; //左下
	vertexData_[1].position = { left,top,0.0f,1.0f }; //左上
	vertexData_[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData_[3].position = { right,top,0.0f,1.0f }; //右上
	vertexData_[0].texcoord = { tex_left,tex_bottom };
	vertexData_[1].texcoord = { tex_left,tex_top };
	vertexData_[2].texcoord = { tex_right,tex_bottom };
	vertexData_[3].texcoord = { tex_right,tex_top };
}

void Sprite::AdjustTextureSize() {

	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(filePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	//画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}

#pragma endregion


#pragma region 描画処理
void Sprite::Draw() {

	ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDirectXCommon()->GetCommandList();

	//TransformationMatrix
	commandList->SetGraphicsRootConstantBufferView(
		spriteCommon_->GetPSO()->GetGraphicBindResourceIndex("gTransformationMatrix"), wvpResource_->GetGPUVirtualAddress());

	TakeCFrameWork::GetPrimitiveDrawer()->DrawSprite(spriteCommon_->GetPSO(), primitiveHandle_);
}
#pragma endregion