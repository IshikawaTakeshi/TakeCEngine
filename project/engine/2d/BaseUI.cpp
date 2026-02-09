#include "BaseUI.h"
#include "engine/Base/SpriteManager.h"
#include "engine/2d/Sprite.h"
#include "engine/base/ImGuiManager.h"

using namespace TakeC;

void BaseUI::Initialize(SpriteManager* spriteManager) {
	// 外部から渡されたマネージャーを保持
	spriteManager_ = spriteManager;
}

void BaseUI::Update() {
	if (!isActive_) return;

	// ここではspriteManager_->Update()は呼ばない。
	// 理由は、Managerが共有されている場合、多重更新になるため。
	// BaseUI継承先で独自のロジック（タイマー処理など）のみ記述する。
}

void BaseUI::Draw() {
	if (!isActive_) return;

	// ここではspriteManager_->Draw()は呼ばない。
	// 一括描画はManagerの持ち主が行う。
}

void BaseUI::UpdateImGui(const std::string& name) {
#ifdef _DEBUG
	if (ImGui::TreeNode(name.c_str())) {
		ImGui::Checkbox("Is Active", &isActive_);
		ImGui::DragFloat2("Position", &position_.x);
		ImGui::DragFloat("Alpha", &alpha_, 0.01f, 0.0f, 1.0f);

		// マネージャーのデバッグ表示はマネージャーの持ち主が行うべきだが、
		// ここで確認したい場合はコメントアウトを外すか検討する。
		// 基本的にはBaseUIごとのパラメータのみ表示する。

		ImGui::TreePop();
	}
#endif
}

Sprite* BaseUI::CreateAndRegisterSprite(const std::string& textureName, const Vector2& position) {
	if (!spriteManager_) return nullptr;
	Sprite* s = spriteManager_->Create(textureName, position);
	sprites_.push_back(s); // 自分のリストにも記録
	return s;
}

Sprite* BaseUI::CreateAndRegisterSpriteFromJson(const std::string& jsonFilePath) {
	if (!spriteManager_) return nullptr;
	Sprite* s = spriteManager_->CreateFromJson(jsonFilePath);
	sprites_.push_back(s); // 自分のリストにも記録
	return s;
}

Sprite* BaseUI::GetSprite(const std::string& name) {
	if (!spriteManager_) return nullptr;
	Sprite* s = spriteManager_->GetSprite(name);
	return s;
}

void BaseUI::SetAlpha(float alpha) {
	if (!spriteManager_) return;

	alpha_ = alpha;
	
	// 管理している全スプライトに適用
	for (Sprite* sprite : sprites_) {
		if (sprite->GetMesh() && sprite->GetMesh()->GetMaterial()) {
			Vector4 color = sprite->GetMesh()->GetMaterial()->GetMaterialData()->color;
			color.w = alpha_;
			sprite->SetMaterialColor(color);
		}
	}
}