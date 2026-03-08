#include "SpriteConfig.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

//==========================================================================================
// ImGui更新
//==========================================================================================
void SpriteConfig::UpdateImGui() {

	ImGui::DragFloat2("Position", &position_.x);
	ImGui::DragFloat2("Size", &size_.x);
	ImGui::DragFloat2("Anchor Point", &anchorPoint_.x);
	ImGui::DragFloat2("Texture Left Top", &textureLeftTop_.x);
	ImGui::DragFloat2("Texture Size", &textureSize_.x);
	ImGui::DragFloat("Rotation", &rotation_);
}

void to_json(nlohmann::json& j, const SpriteConfig& spriteConfig) {

	j["name"] = spriteConfig.name;
	j["textureFilePath"] = spriteConfig.textureFilePath_;
	j["position"] = spriteConfig.position_;
	j["size"] = spriteConfig.size_;
	j["anchorPoint"] = spriteConfig.anchorPoint_;
	j["textureLeftTop"] = spriteConfig.textureLeftTop_;
	j["textureSize"] = spriteConfig.textureSize_;
	j["rotation"] = spriteConfig.rotation_;
	j["color"] = spriteConfig.color_;
}

void from_json(const nlohmann::json& j, SpriteConfig& spriteConfig) {

	spriteConfig.name = j.value("name", "");
	spriteConfig.textureFilePath_ = j.value("textureFilePath", "white1x1.png");
	spriteConfig.position_ = j.value("position", Vector2{ 0.0f,0.0f });
	spriteConfig.size_ = j.value("size", Vector2{ 400.0f,200.0f });
	spriteConfig.anchorPoint_ = j.value("anchorPoint", Vector2{ 0.0f,0.0f });
	spriteConfig.textureLeftTop_ = j.value("textureLeftTop", Vector2{ 0.0f,0.0f });
	spriteConfig.textureSize_ = j.value("textureSize", Vector2{ 100.0f,100.0f });
	spriteConfig.rotation_ = j.value("rotation", 0.0f);
	spriteConfig.color_ = j.value("color", Vector4{ 1.0f,1.0f,1.0f,1.0f });
}
