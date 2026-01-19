#include "RingUI.h"
#include "engine/2d/SpriteCommon.h"

void RingUI::Initialize() {
	SpriteCommon* spriteCommon = &SpriteCommon::GetInstance();
	// 画像はテクスチャの右上の象限にある（または全体を占める）「1/4リング」であると仮定し、
	// テクスチャの左下隅がリングの中心と一致するようにします。
	std::string texturePath = "Resources/ui/ring_quarter.png";

	for (int i = 0; i < kPartCount; ++i) {
		parts_[i] = std::make_unique<Sprite>();
		parts_[i]->Initialize(spriteCommon, texturePath);

		// アンカーポイントを左下（0, 1）に設定してピボット（回転中心）にする
		parts_[i]->SetAnchorPoint({ 0.0f, 1.0f });

		// 初期位置を中心位置に設定
		parts_[i]->SetTranslate(centerPos_);

		// 各パーツを90度ずつ回転させる
		// 0: 0度 (右上)
		// 1: 90度 (右下) 
		// 2: 180度 (左下)
		// 3: 270度 (左上)
		float rotation = static_cast<float>(i) * (3.14159265f / 2.0f); // 90度(ラジアン)
		parts_[i]->SetRotate(rotation);
	}
}

void RingUI::Update() {
	if (!isActive_) return;

	for (auto& part : parts_) {
		// 中心位置が移動した場合に備えて位置を更新
		// メモ: 個別のオフセットをサポートしたい場合は、ロジックを追加する必要があります。
		// 現状はすべて中心に固定されています。
		part->SetTranslate(centerPos_);
		part->Update();
	}
}

void RingUI::Draw() {
	if (!isActive_) return;

	for (auto& part : parts_) {
		part->Draw();
	}
}

void RingUI::SetColor(int index, const Vector4& color) {
	if (index >= 0 && index < kPartCount) {
		parts_[index]->SetMaterialColor(color);
	}
}

void RingUI::SetAllColor(const Vector4& color) {
	for (auto& part : parts_) {
		part->SetMaterialColor(color);
	}
}

void RingUI::SetScale(int index, const Vector2& scale) {
	if (index >= 0 && index < kPartCount) {
		parts_[index]->SetSize(scale);
	}
}

void RingUI::SetCenterPosition(const Vector2& position) {
	centerPos_ = position;
}

void RingUI::SetActive(bool isActive) {
	isActive_ = isActive;
}
