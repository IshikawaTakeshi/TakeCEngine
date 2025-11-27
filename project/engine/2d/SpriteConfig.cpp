#include "SpriteConfig.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

//==========================================================================================
// ImGui更新
//==========================================================================================
void SpriteConfig::UpdateImGui() {
	ImGui::InputFloat2("Position", &position_.x);
	ImGui::InputFloat2("Size", &size_.x);
	ImGui::InputFloat2("Anchor Point", &anchorPoint_.x);
	ImGui::InputFloat2("Texture Left Top", &textureLeftTop_.x);
	ImGui::InputFloat2("Texture Size", &textureSize_.x);
	ImGui::InputFloat("Rotation", &rotation_);
}

void to_json(nlohmann::json& j, const SpriteConfig& spriteConfig) {

	j["jsonFilePath"] = spriteConfig.jsonFilePath;
	j["textureFilePath"] = spriteConfig.textureFilePath_;
	j["position"] = spriteConfig.position_;
	j["size"] = spriteConfig.size_;
	j["anchorPoint"] = spriteConfig.anchorPoint_;
	j["textureLeftTop"] = spriteConfig.textureLeftTop_;
	j["textureSize"] = spriteConfig.textureSize_;
	j["rotation"] = spriteConfig.rotation_;
}

void from_json(const nlohmann::json& j, SpriteConfig& spriteConfig) {

	j.at("jsonFilePath").get_to(spriteConfig.jsonFilePath);
	j.at("textureFilePath").get_to(spriteConfig.textureFilePath_);
	j.at("position").get_to(spriteConfig.position_);
	j.at("size").get_to(spriteConfig.size_);
	j.at("anchorPoint").get_to(spriteConfig.anchorPoint_);
	j.at("textureLeftTop").get_to(spriteConfig.textureLeftTop_);
	j.at("textureSize").get_to(spriteConfig.textureSize_);
	j.at("rotation").get_to(spriteConfig.rotation_);
}
