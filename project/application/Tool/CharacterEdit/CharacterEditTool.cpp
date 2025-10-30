#include "CharacterEditTool.h"
#include "engine/base/TakeCFrameWork.h"

//========================================================================
// 初期化
//========================================================================
void CharacterEditTool::Initialize() {
	
	//1.武器、キャラクターデータの全読み込み

	LoadAllWeaponData();
	LoadAllCharacterData();

	//2.編集中のキャラクターデータ初期化
	editCharacterData_ = CharacterData{};
	

}

void CharacterEditTool::Update() {
	// 更新処理
}

void CharacterEditTool::UpdateImGui() {
	// ImGui更新処理
}

void CharacterEditTool::UpdateCharacterEdit() {
	// キャラクター編集の更新処理
}

void CharacterEditTool::UpdateWeaponEdit() {
	// 武器編集の更新処理
}

void CharacterEditTool::Draw() {
	// 描画処理
}

void CharacterEditTool::DrawUI() {
	// UI描画処理
}

void CharacterEditTool::CharacterDataSelect() {
	// キャラクターデータ選択処理
}

WeaponData CharacterEditTool::WeaponSelect(WeaponUnit unit) {
	// 武器データ選択処理
	return WeaponData{};
}

void CharacterEditTool::SaveCharacterData(std::string characterName) {

	// キャラクターデータ保存処理
	// プリセット名が空でないか、既に存在しないか確認
	if (characterName.empty() || characterContextMap_.find(characterName) != characterContextMap_.end()) {
		//ImGui::Text("Preset already exists or name is empty: %s", presetName.c_str());
		return;
	}

	// プリセットが既に存在する場合は上書き確認
	if (characterContextMap_.find(characterName) != characterContextMap_.end()) {
		// 上書き確認フラグを立てる（次のフレームで確認ダイアログを表示）
		showOverwriteConfirm_ = true;
		pendingCharacterName_ = characterName;
		return;
	}

	// 現在のキャラクター名をcharacterDataとして保存
	currentCharacterData_.characterName = characterName;
	//TakeCFrameWork::GetJsonLoader()->SaveJsonData<GameCharacterContext>(characterName + ".json", currentPreset_);
	// プリセット名を更新
	//characterNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<GameCharacterContext>();

}
