#include "SpriteSheetSttings.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

using namespace TakeC;

//=============================================================================
// ImGuiで設定編集
//=============================================================================
void SpriteSheetSettings::EditConfigImGui(const std::string& label) {
	//ImGuiで編集
	if (ImGui::TreeNode(label.c_str())) {
		ImGui::DragScalar("Columns", ImGuiDataType_U32, &columns, uint32_t(1));
		ImGui::DragScalar("Rows", ImGuiDataType_U32, &rows, uint32_t(1));
		ImGui::DragScalar("Total Frames", ImGuiDataType_U32, &totalFrames);
		ImGui::DragFloat("Frame Duration", &frameDuration, 0.01f, 0.1f);
		ImGui::Checkbox("Loop", &loop);

		// 保存用ポップアップ
		ImGuiManager::ShowSavePopup<SpriteSheetSettings>(
			TakeCFrameWork::GetJsonLoader(),
			"Save SpriteSheet Settings",
			"SpriteSheetSettings.json",
			*this,
			filePath
		);

		ImGui::TreePop();
	}
}