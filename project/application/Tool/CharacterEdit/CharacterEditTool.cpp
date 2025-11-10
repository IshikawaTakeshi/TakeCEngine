#include "CharacterEditTool.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/3d/Object3d.h"
#include "engine/io/Input.h"

//========================================================================
// 初期化
//========================================================================
void CharacterEditTool::Initialize() {

	//武器、キャラクターデータの全読み込み
	LoadAllWeaponData();
	LoadAllCharacterData();

	//武器、キャラクターデータのサイズを使ってresize,初期化
	maxWeaponMenuItems_ = static_cast<uint32_t>(weaponDataMap_.size());
	weaponItemSprites_.resize(maxWeaponMenuItems_);
	for(size_t i =0;i<weaponItemSprites_.size();++i){
		weaponItemSprites_[i] = std::make_unique<Sprite>();
		weaponItemSprites_[i]->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
		weaponItemSprites_[i]->SetSize(menuBarSpriteSize_);
		weaponItemSprites_[i]->SetTranslate({
			0.0f,
			menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(i)
			});
		weaponItemSprites_[i]->SetMaterialColor({ 0.2f,0.2f,0.2f,0.0f });
	}


	//編集中のキャラクターデータ初期化
	currentCharacterData_ = CharacterData{};

	//カーソルスプライト初期化
	cursorSprite_ = std::make_unique<Sprite>();
	cursorSprite_->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
	cursorSprite_->SetSize(menuBarSpriteSize_);
	cursorSprite_->SetTranslate(menuBarLeftTop_);

	//メニューバー用スプライト群初期化
	for (size_t i = 0; i < WeaponUnit::Size; ++i) {
		weaponMenuBarSprites_[i] = std::make_unique<Sprite>();
		weaponMenuBarSprites_[i]->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
		weaponMenuBarSprites_[i]->SetSize(menuBarSpriteSize_);
		weaponMenuBarSprites_[i]->SetTranslate({
			menuBarLeftTop_.x,
			menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(i)
			});
		weaponMenuBarSprites_[i]->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
	}

	//編集モード初期化
	editModeRequest_ = EditMode::EDIT_MENU;

	
}

//========================================================================
// 更新処理
//========================================================================
void CharacterEditTool::Update() {

	// 編集モードリクエストがある場合、編集モードを変更
	if (editModeRequest_) {

		//EditMode previousMode = editMode_;
		editMode_ = *editModeRequest_;

		// モード変更時の初期化処理
		switch (editMode_) {
		case EditMode::EDIT_MENU:

			for(size_t i =0;i<weaponMenuBarSprites_.size();++i){
				// メニューバースプライトを元の位置に戻すアニメーション再生
				weaponMenuBarSprites_[i]->Animation()->PlayTranslate(
					weaponMenuBarSprites_[i]->GetTranslate(),
					{ menuBarLeftTop_.x,weaponMenuBarSprites_[i]->GetTranslate().y },
					0.2f,
					0.0f,
					Easing::EasingType::OUT_BACK,
					SpriteAnimator::PlayMode::ONCE
				);
				// メニューバースプライトのフェードインアニメーション再生
				weaponMenuBarSprites_[i]->Animation()->PlayFade(0.0f,1.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
			}

			// カーソルスプライトの移動アニメーション再生
			cursorSprite_->Animation()->PlayTranslate(
				cursorSprite_->GetTranslate(),
				{ menuBarLeftTop_.x,menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(editingItemIndex_) },
				0.5f,
				0.0f,
				Easing::EasingType::OUT_BACK,
				SpriteAnimator::PlayMode::PINGPONG_LOOP
			);

			break;
		case EditMode::CHARACTER_EDIT:


			break;
		case EditMode::WEAPON_EDIT:

			// 武器編集モード初期化処理
			//武器項目スプライトの移動アニメーション再生
			for(auto& sprite:weaponItemSprites_){
				sprite->Animation()->PlayTranslate(
					sprite->GetTranslate(),
					{ menuBarLeftTop_.x,sprite->GetTranslate().y },
					0.2f,
					0.0f,
					Easing::EasingType::OUT_BACK,
					SpriteAnimator::PlayMode::ONCE
				);
				// 武器項目スプライトのフェードインアニメーション再生
				sprite->Animation()->PlayFade(0.0f,1.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
			}

			break;
		default:
			break;
		}

		// カーソルスプライトのフェードインアニメーション再生
		cursorSprite_->Animation()->PlayFade(0.0f, 0.6f, 0.5f, 0.0f, Easing::EasingType::LINEAR, SpriteAnimator::PlayMode::PINGPONG_LOOP);
		
		editModeRequest_ = std::nullopt;
	}

	switch (editMode_) {
	case EditMode::EDIT_MENU:

		// 編集項目選択処理
		SelectEditItem();
		break;
	case EditMode::CHARACTER_EDIT:

		// キャラクター編集更新処理
		break;
	case EditMode::WEAPON_EDIT:

		// 武器編集更新処理
		UpdateWeaponEdit();

		break;
	default:
		break;
	}

	// カーソルスプライトの更新
	cursorSprite_->Update();
	// メニューバー用スプライト群の更新
	for (auto& sprite : weaponMenuBarSprites_) {
		sprite->Update();
	}
	// 武器項目スプライト群の更新
	for(auto& sprite:weaponItemSprites_){
		sprite->Update();
	}
}

//========================================================================
// ImGui更新処理
//========================================================================
void CharacterEditTool::UpdateImGui() {
	// ImGui更新処理
	ImGui::Begin("Character Edit Tool");

	//menu bar用スプライトのImGui更新
	ImGui::SeparatorText("Character Edit Tool");
	cursorSprite_->UpdateImGui("Cursor Sprite");
	for (size_t i = 0; i < WeaponUnit::Size; ++i) {
		weaponMenuBarSprites_[i]->UpdateImGui("Weapon Menu Bar Sprite " + std::to_string(i));
	}
	ImGui::End();
}

//========================================================================
// 編集項目選択処理
//========================================================================
void CharacterEditTool::SelectEditItem() {


	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadDOWN)) {
		//下ボタンが押されたときの処理
		editingItemIndex_ = (editingItemIndex_ + 1) % maxMenuItems_;

		//オフセット分を越えたら0に戻す
		if (editingItemIndex_ >= maxMenuItems_) {
			editingItemIndex_ = 0;
		}

	} else if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadUP)) {
		//上ボタンが押されたときの処理
		editingItemIndex_ = (editingItemIndex_ - 1 + maxMenuItems_) % maxMenuItems_;

		//オフセット分を越えたら最後の項目に戻す
		if (editingItemIndex_ >= maxMenuItems_) {
			editingItemIndex_ = maxMenuItems_ - 1;
		}
	}

	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::A)) {
		// 決定ボタンが押されたときの処理

		//インデックスが武器項目内なら武器編集モードへ
		if (editingItemIndex_ < WeaponUnit::Size) {
			editModeRequest_ = EditMode::WEAPON_EDIT;
		} else {
			editModeRequest_ = EditMode::CHARACTER_EDIT;
		}

		//spriteのアニメーション再生
		for(auto& sprite:weaponMenuBarSprites_){
			// メニューバースプライトを画面外に移動させるアニメーション再生
			sprite->Animation()->PlayTranslate(
				sprite->GetTranslate(),
				{ 0.0f,sprite->GetTranslate().y },
				0.2f,
				0.0f,
				Easing::EasingType::OUT_BACK,
				SpriteAnimator::PlayMode::ONCE
			);
			// メニューバースプライトのフェードアウトアニメーション再生
			sprite->Animation()->PlayFade(
				1.0f,0.0f,
				0.2f,
				0.0f,
				Easing::EasingType::LINEAR,
				SpriteAnimator::PlayMode::ONCE
			);
		}
	}

	// カーソルスプライトの位置更新
	cursorSprite_->SetTranslate({
		menuBarLeftTop_.x,
		menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(editingItemIndex_)
		});
}

//========================================================================
// 描画処理
//========================================================================
void CharacterEditTool::Draw() {

}

void CharacterEditTool::DrawUI() {
	// UI描画処理

	// メニューバー用スプライト群の描画
	for (auto& sprite : weaponMenuBarSprites_) {
		sprite->Draw();
	}
	// 武器項目スプライト群の描画
	for(auto& sprite:weaponItemSprites_){
		sprite->Draw();
	}
	// カーソルスプライトの描画
	cursorSprite_->Draw();
}

//========================================================================
// キャラクター編集更新処理
//========================================================================
void CharacterEditTool::UpdateCharacterEdit() {
	// キャラクター編集の更新処理


}

//========================================================================
// 武器編集更新処理
//========================================================================
void CharacterEditTool::UpdateWeaponEdit() {

	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadDOWN)) {
		//下ボタンが押されたときの処理
		editingWeaponUnitIndex_ = (editingWeaponUnitIndex_ + 1) % maxWeaponMenuItems_;

		//オフセット分を越えたら0に戻す
		if (editingWeaponUnitIndex_ >= maxWeaponMenuItems_) {
			editingWeaponUnitIndex_ = 0;
		}

	} else if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadUP)) {
		//上ボタンが押されたときの処理
		editingWeaponUnitIndex_ = (editingWeaponUnitIndex_ - 1 + maxWeaponMenuItems_) % maxWeaponMenuItems_;

		//オフセット分を越えたら最後の項目に戻す
		if (editingWeaponUnitIndex_ >= maxWeaponMenuItems_) {
			editingWeaponUnitIndex_ = maxWeaponMenuItems_ - 1;
		}
	}

	//決定ボタンが押されたときの処理
	if(Input::GetInstance()->TriggerButton(0,GamepadButtonType::A)){
		
		// 武器の適用
		currentCharacterData_.weaponData[editingItemIndex_] = *AttachWeapon(weaponNames_[editingWeaponUnitIndex_]);
		// 編集モードをメニューに変更
		editModeRequest_ = EditMode::EDIT_MENU;

		// 武器項目スプライトの移動アニメーション再生
		for(auto& sprite:weaponItemSprites_){
			sprite->Animation()->PlayTranslate(
				sprite->GetTranslate(),
				{ 0.0f,sprite->GetTranslate().y },
				0.2f,
				0.0f,
				Easing::EasingType::OUT_BACK,
				SpriteAnimator::PlayMode::ONCE
			);
			// 武器項目スプライトのフェードアウトアニメーション再生
			sprite->Animation()->PlayFade(1.0f,0.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
		}
	}

	//戻るボタンが押されたときの処理
	if(Input::GetInstance()->TriggerButton(0,GamepadButtonType::B)){

		// 編集モードをメニューに変更
		editModeRequest_ = EditMode::EDIT_MENU;

		// 武器項目スプライトの移動アニメーション再生
		for(auto& sprite:weaponItemSprites_){
			sprite->Animation()->PlayTranslate(
				sprite->GetTranslate(),
				{ 0.0f,sprite->GetTranslate().y },
				0.2f,
				0.0f,
				Easing::EasingType::OUT_BACK,
				SpriteAnimator::PlayMode::ONCE
			);
			// 武器項目スプライトのフェードアウトアニメーション再生
			sprite->Animation()->PlayFade(1.0f,0.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
		}
	}

	// カーソルスプライトの位置更新
	cursorSprite_->SetTranslate({
		menuBarLeftTop_.x,
		menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(editingWeaponUnitIndex_)
		});
}

//========================================================================
// キャラクターデータ適用処理
//========================================================================

std::optional<PlayableCharacterInfo> CharacterEditTool::AttachPlayableCharacterInfo(const std::string& characterName) {

	auto it = playableCharacterInfoMap_.find(characterName);
	if (it != playableCharacterInfoMap_.end()) {
		// 見つかった場合、そのデータを返す
		return it->second;
	}
	return std::nullopt;
}

//========================================================================
// 武器データ適用処理
//========================================================================
std::optional<WeaponData> CharacterEditTool::AttachWeapon(const std::string& weaponName) {

	// 武器データマップから指定された武器名のデータを取得
	auto it = weaponDataMap_.find(weaponName);
	if (it != weaponDataMap_.end()) {
		// 見つかった場合、そのデータを返す
		return it->second;
	}
	return std::nullopt;
}

void CharacterEditTool::SavePlayableCharacterInfo(std::string characterName) {

	// キャラクターデータ保存処理
	// プリセット名が空でないか、既に存在しないか確認
	if (characterName.empty() || playableCharacterInfoMap_.find(characterName) != playableCharacterInfoMap_.end()) {
		return;
	}

	// プリセットが既に存在する場合は上書き確認
	if (playableCharacterInfoMap_.find(characterName) != playableCharacterInfoMap_.end()) {
		// 上書き確認フラグを立てる（次のフレームで確認ダイアログを表示）
		showOverwriteConfirm_ = true;
		pendingCharacterName_ = characterName;
		return;
	}

	// 現在のキャラクター名をcharacterDataとして保存
	currentCharacterData_.playableCharacterInfo.characterName = characterName;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<PlayableCharacterInfo>(characterName + ".json", currentCharacterData_.playableCharacterInfo);
	// キャラクター名リストを更新
	characterNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<PlayableCharacterInfo>();

}

void CharacterEditTool::SaveWeaponData(std::string WeaponName, WeaponUnit unit) {

	// プリセット名が空でないか、既に存在しないか確認
	if (WeaponName.empty() || weaponDataMap_.find(WeaponName) != weaponDataMap_.end()) {
		return;
	}

	// プリセットが既に存在する場合は上書き確認
	if (weaponDataMap_.find(WeaponName) != weaponDataMap_.end()) {
		// 上書き確認フラグを立てる（次のフレームで確認ダイアログを表示）
		showOverwriteConfirm_ = true;
		pendingWeaponName_ = WeaponName;
		return;
	}

	// 現在のキャラクター名をcharacterDataとして保存
	currentCharacterData_.weaponData[unit].weaponName = WeaponName;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<WeaponData>(WeaponName + ".json", currentCharacterData_.weaponData[unit]);


	// 武器名リストを更新
	weaponNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<WeaponConfig>();
}

void CharacterEditTool::LoadPlayableCharacterInfo(std::string characterName) {

	if (characterName.empty() || playableCharacterInfoMap_.find(characterName) == playableCharacterInfoMap_.end()) {
		ImGui::Text("Preset not found: %s", characterName.c_str());
		return;
	}

	//JSONからプリセットを読み込む
	PlayableCharacterInfo playableInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PlayableCharacterInfo>(characterName + ".json");

	// コンテナに存在しなければ追加
	if (!playableCharacterInfoMap_.contains(characterName)) {
		playableCharacterInfoMap_[characterName] = playableInfo;
		characterNames_.push_back(characterName);
	}

	// 編集中のキャラクターデータに反映
	currentCharacterData_.playableCharacterInfo = playableInfo;
}

void CharacterEditTool::LoadWeaponData(std::string WeaponName, WeaponUnit unit) {

	if (WeaponName.empty() || weaponDataMap_.find(WeaponName) == weaponDataMap_.end()) {
		ImGui::Text("Preset not found: %s", WeaponName.c_str());
		return;
	}
	//JSONからプリセットを読み込む
	WeaponData loadedWeaponData = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>(WeaponName + ".json");

	switch (loadedWeaponData.weaponType) {
	case WeaponType::NONE:
		// NONEの場合の処理
		break;
	case WeaponType::WEAPON_TYPE_RIFLE:
	{


		// ライフルの場合の処理
		RifleInfo rifleInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<RifleInfo>(WeaponName + "_extraInfo.json");
		loadedWeaponData.actionData = rifleInfo;
		break;
	}
	case WeaponType::WEAPON_TYPE_BAZOOKA:
		// バズーカの場合の処理
		break;
	case WeaponType::WEAPON_TYPE_VERTICAL_MISSILE:
	{

		// 垂直ミサイルの場合の処理
		VerticalMissileLauncherInfo vmLauncherInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<VerticalMissileLauncherInfo>(WeaponName + "_extraInfo.json");
		loadedWeaponData.actionData = vmLauncherInfo;
		break;
	}
	default:
		break;
	}

	// コンテナに存在しなければ追加
	if (!weaponDataMap_.contains(WeaponName)) {
		weaponDataMap_[WeaponName] = loadedWeaponData;
		weaponNames_.push_back(WeaponName);
	}

	// 編集中のキャラクターデータに反映
	currentCharacterData_.weaponData[unit] = loadedWeaponData;
}

void CharacterEditTool::LoadAllCharacterData() {
}

void CharacterEditTool::LoadAllWeaponData() {

	// 全武器データの読み込み
	weaponNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<WeaponData>();
	for (const auto& weaponName : weaponNames_) {
		WeaponData weaponData = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>(weaponName + ".json");
		switch (weaponData.weaponType) {
		case WeaponType::NONE:
			// NONEの場合の処理
			break;
		case WeaponType::WEAPON_TYPE_RIFLE:
		{
			// ライフルの場合の処理
			RifleInfo rifleInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<RifleInfo>(weaponName + "_extraInfo.json");
			weaponData.actionData = rifleInfo;
			break;
		}
		case WeaponType::WEAPON_TYPE_BAZOOKA:
			// バズーカの場合の処理
			break;
		case WeaponType::WEAPON_TYPE_VERTICAL_MISSILE:
		{
			// 垂直ミサイルの場合の処理
			VerticalMissileLauncherInfo vmLauncherInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<VerticalMissileLauncherInfo>(weaponName + "_extraInfo.json");
			weaponData.actionData = vmLauncherInfo;
			break;
		}
		default:
			break;
		}
		weaponDataMap_[weaponName] = weaponData;
	}
}
