#include "TextureAnimation.h"
#include "engine/3d/Material.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Utility/StringUtility.h"
#include <cmath>

using namespace TakeC;

//=================================================================================
// 初期化
//=================================================================================
void UVTextureAnimation::Initialize(Material* material) {
	material_ = material;
	Reset();
}

//=================================================================================
// 更新処理
//=================================================================================
void UVTextureAnimation::Update(float timeScale) {
	if (material_ == nullptr) {
		return;
	}

	// タイマー更新
	frameTimer_.Update(timeScale);

	switch (animationType_) {
	case TextureAnimationType::UVScroll:
		UpdateUVScroll();
		break;
	case TextureAnimationType::SpriteSheet:
		UpdateSpriteSheet();
		break;
	default:
		break;
	}

	ApplyToMaterial();
}

void UVTextureAnimation::UpdateImGui(const std::string& label) {
	// ImGuiによる編集
	
	if(ImGui::TreeNode(label.c_str())){
		// アニメーションタイプの設定
		std::string animationTypeStr = StringUtility::EnumToString(animationType_);
		if (ImGui::BeginCombo("Animation Type", animationTypeStr.c_str())) {
			for (int i = 0; i <= static_cast<int>(TextureAnimationType::SpriteSheet); ++i) {
				TextureAnimationType type = static_cast<TextureAnimationType>(i);
				std::string typeStr = StringUtility::EnumToString(type);
				bool isSelected = (animationType_ == type);
				if (ImGui::Selectable(typeStr.c_str(), isSelected)) {
					animationType_ = type;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		
		// UVオフセットとスケールの表示
		ImGui::Text("Current UV Offset: (%.3f, %.3f)", currentUVOffset_.x, currentUVOffset_.y);
		ImGui::Text("Current UV Scale: (%.3f, %.3f)", currentUVScale_.x, currentUVScale_.y);

		if(ImGui::Button("Play")){
			Play();
		}
		if(ImGui::Button("Pause")){
			Pause();
		}
		if(ImGui::Button("Stop")){
			Stop();
		}
		if(ImGui::Button("Restart")){
			Restart();
		}


		// uvScrollSettingsの編集
		ImGui::SeparatorText("UV Scroll Settings");
		ImGui::DragFloat2("Scroll Speed", &uvScrollSettings_.scrollSpeed.x, 0.01f, -10.0f, 10.0f);
		ImGui::Checkbox("Wrap U", &uvScrollSettings_.wrapU);
		ImGui::Checkbox("Wrap V", &uvScrollSettings_.wrapV);
		if(ImGui::Button("Apply UV Scroll Settings")){
			SetUVScrollAnimation(uvScrollSettings_);
		}

		//spriteSheetSettingsの編集
		spriteSheetSettings_.EditConfigImGui(label + "Sprite Sheet Settings");
		ImGui::TreePop();
	}

}

//=================================================================================
// リセット
//=================================================================================
void UVTextureAnimation::Reset() {
	currentUVOffset_ = { 0.0f, 0.0f };
	currentUVScale_ = { 1.0f, 1.0f };
	currentFrame_ = 0;
	frameTimer_.Reset();
}

//=================================================================================
// 再生開始
//=================================================================================
void UVTextureAnimation::Play() {
	frameTimer_.Restart();
}

//=================================================================================
// 一時停止
//=================================================================================
void UVTextureAnimation::Pause() {
	frameTimer_.Stop();
}

//=================================================================================
// 停止（リセットも行う）
//=================================================================================
void UVTextureAnimation::Stop() {
	frameTimer_.Stop();
	Reset();
}

//=================================================================================
// 再開
//=================================================================================
void UVTextureAnimation::Restart() {
	Reset();
	frameTimer_.Restart();
}

//=================================================================================
// 再生中かどうか
//=================================================================================
bool UVTextureAnimation::IsPlaying() const {
	// タイマーが終了していない状態を再生中とする
	return !frameTimer_.IsFinished();
}

//=================================================================================
// 終了判定
//=================================================================================
bool UVTextureAnimation::IsFinished() const {
	return frameTimer_.IsFinished();
}

//=================================================================================
// UVスクロールアニメーション設定
//=================================================================================
void UVTextureAnimation::SetUVScrollAnimation(const Vector2& scrollSpeed) {
	UVScrollSettings settings;
	settings.scrollSpeed = scrollSpeed;
	SetUVScrollAnimation(settings);
}

void UVTextureAnimation::SetUVScrollAnimation(const UVScrollSettings& settings) {
	animationType_ = TextureAnimationType::UVScroll;
	uvScrollSettings_ = settings;
	Reset();
}

//=================================================================================
// スプライトシートアニメーション設定
//=================================================================================
void UVTextureAnimation::SetSpriteSheetAnimation(uint32_t columns, uint32_t rows,
	uint32_t totalFrames, float frameDuration, bool loop) {
	SpriteSheetSettings settings;
	settings.columns = columns;
	settings.rows = rows;
	settings.totalFrames = totalFrames;
	settings.frameDuration = frameDuration;
	settings.loop = loop;
	SetSpriteSheetAnimation(settings);
}

void UVTextureAnimation::SetSpriteSheetAnimation(const SpriteSheetSettings& settings) {
	animationType_ = TextureAnimationType::SpriteSheet;
	spriteSheetSettings_ = settings;

	// スケールを計算（1フレーム分のUVサイズ）
	currentUVScale_.x = 1.0f / static_cast<float>(settings.columns);
	currentUVScale_.y = 1.0f / static_cast<float>(settings.rows);

	// タイマーを1フレーム分の時間で初期化
	frameTimer_.Initialize(settings.frameDuration, 0.0f);

	Reset();
}

//=================================================================================
// UVスクロール更新
//=================================================================================
void UVTextureAnimation::UpdateUVScroll() {

	float deltaTime = TakeCFrameWork::GetDeltaTime();

	// UVオフセットを更新
	currentUVOffset_.x += uvScrollSettings_.scrollSpeed.x * deltaTime;
	currentUVOffset_.y += uvScrollSettings_.scrollSpeed.y * deltaTime;

	// ラップ処理
	if (uvScrollSettings_.wrapU) {
		currentUVOffset_.x = std::fmod(currentUVOffset_.x, 1.0f);
		if (currentUVOffset_.x < 0.0f) {
			currentUVOffset_.x += 1.0f;
		}
	}
	if (uvScrollSettings_.wrapV) {
		currentUVOffset_.y = std::fmod(currentUVOffset_.y, 1.0f);
		if (currentUVOffset_.y < 0.0f) {
			currentUVOffset_.y += 1.0f;
		}
	}
}

//=================================================================================
// スプライトシート更新
//=================================================================================
void UVTextureAnimation::UpdateSpriteSheet() {
	// タイマーが終了したらフレームを進める
	if (frameTimer_.IsFinished()) {
		currentFrame_++;

		// 最終フレームに到達
		if (currentFrame_ >= spriteSheetSettings_.totalFrames) {
			if (spriteSheetSettings_.loop) {
				currentFrame_ = 0;
				frameTimer_.Restart();
			} else {
				currentFrame_ = spriteSheetSettings_.totalFrames - 1;
				// ループしない場合はタイマーを終了状態のままにする
				frameTimer_.ToEnd();
			}
		} else {
			// 次のフレームへ
			frameTimer_.Restart();
		}
	}

	// 現在のフレームに対応するUVオフセットを計算
	uint32_t col = currentFrame_ % spriteSheetSettings_.columns;
	uint32_t row = currentFrame_ / spriteSheetSettings_.columns;

	currentUVOffset_.x = static_cast<float>(col) / static_cast<float>(spriteSheetSettings_.columns);
	currentUVOffset_.y = static_cast<float>(row) / static_cast<float>(spriteSheetSettings_.rows);
}

//=================================================================================
// マテリアルにUV変換を適用
//=================================================================================
void UVTextureAnimation::ApplyToMaterial() {
	if (material_ == nullptr) {
		return;
	}

	// UVスケールとオフセットをマテリアルに適用
	material_->SetUvScale({ currentUVScale_.x, currentUVScale_.y, 1.0f });
	material_->SetUvTranslate({ currentUVOffset_.x, currentUVOffset_.y, 0.0f });
}