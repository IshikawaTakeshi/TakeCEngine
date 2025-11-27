#include "CharacterEditTool.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
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

	maxCharacterMenuItems_ = static_cast<uint32_t>(playableCharacterInfoMap_.size());
	characterItemSprites_.resize(maxCharacterMenuItems_);
	for(size_t i =0;i<characterItemSprites_.size();++i){
		characterItemSprites_[i] = std::make_unique<Sprite>();
		characterItemSprites_[i]->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
		characterItemSprites_[i]->SetSize(menuBarSpriteSize_);
		characterItemSprites_[i]->SetTranslate({
			0.0f,
			menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(i)
			});
		characterItemSprites_[i]->SetMaterialColor({ 0.2f,0.2f,0.2f,0.0f });
	}

	//カーソルスプライト初期化
	cursorSprite_ = std::make_unique<Sprite>();
	cursorSprite_->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
	cursorSprite_->SetSize(menuBarSpriteSize_);
	cursorSprite_->SetTranslate(menuBarLeftTop_);

	//メニューバー用スプライト群初期化
	for (size_t i = 0; i < CharacterEditMenuEnum::MENU_SIZE; ++i) {
		menuBarSprites_[i] = std::make_unique<Sprite>();
		menuBarSprites_[i]->Initialize(SpriteCommon::GetInstance(), "white1x1.png");
		menuBarSprites_[i]->SetSize(menuBarSpriteSize_);
		menuBarSprites_[i]->SetTranslate({
			menuBarLeftTop_.x,
			menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(i)
			});
		menuBarSprites_[i]->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
	}

	//編集中のキャラクターデータ初期化
	currentCharacterData_ = CharacterData{};
	currentCharacterData_.characterInfo = *AttachPlayableCharacterInfo("Player");
	currentCharacterData_.weaponData[static_cast<size_t>(WeaponUnit::L_ARMS)] = *AttachWeapon("Rifle");
	currentCharacterData_.weaponData[static_cast<size_t>(WeaponUnit::R_ARMS)] = *AttachWeapon("Bazooka");
	currentCharacterData_.weaponData[static_cast<size_t>(WeaponUnit::L_BACK)] = *AttachWeapon("VMLauncher");
	currentCharacterData_.weaponData[static_cast<size_t>(WeaponUnit::R_BACK)] = *AttachWeapon("VMLauncher");

	//プレビュー用キャラクターモデル初期化
	previewCharacterModel_ = std::make_unique<Object3d>();
	previewCharacterModel_->Initialize(Object3dCommon::GetInstance(),currentCharacterData_.characterInfo.modelFilePath);

	//プレビュー用武器モデル群初期化
	previewWeaponModels_.resize(WeaponUnit::Size);
	for(size_t i =0;i<previewWeaponModels_.size();++i){
		previewWeaponModels_[i] = std::make_unique<Object3d>();
		previewWeaponModels_[i]->Initialize(Object3dCommon::GetInstance(),currentCharacterData_.weaponData[i].modelFilePath);
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

		editMode_ = *editModeRequest_;

		// モード変更時の初期化処理
		switch (editMode_) {
		case EditMode::EDIT_MENU:

			//キャラクター情報をセーブするときはアニメーション再生をスキップする
			if(editingItemIndex_ == CharacterEditMenuEnum::SAVE_CONFIG){
				break;
			}

			for(size_t i =0;i<menuBarSprites_.size();++i){
				// メニューバースプライトを元の位置に戻すアニメーション再生
				menuBarSprites_[i]->Animation()->PlayTranslate(
					menuBarSprites_[i]->GetTranslate(),
					{ menuBarLeftTop_.x,menuBarSprites_[i]->GetTranslate().y },
					0.2f,
					0.0f,
					Easing::EasingType::OUT_BACK,
					SpriteAnimator::PlayMode::ONCE
				);
				// メニューバースプライトのフェードインアニメーション再生
				menuBarSprites_[i]->Animation()->PlayFade(0.0f,1.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
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

			// キャラクター編集モード初期化処理
			//キャラクター項目スプライトの移動アニメーション再生
			for(auto& sprite:characterItemSprites_){
				sprite->Animation()->PlayTranslate(
					sprite->GetTranslate(),
					{ menuBarLeftTop_.x,sprite->GetTranslate().y },
					0.2f,
					0.0f,
					Easing::EasingType::OUT_BACK,
					SpriteAnimator::PlayMode::ONCE
				);
				// キャラクター項目スプライトのフェードインアニメーション再生
				sprite->Animation()->PlayFade(0.0f,1.0f,0.2f,0.0f,Easing::EasingType::LINEAR,SpriteAnimator::PlayMode::ONCE);
			}

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
		case EditMode::NEXT_MENU:

			// キャラクターデータ保存処理
			SaveCharacterData();
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
		UpdateCharacterEdit();
		break;
	case EditMode::WEAPON_EDIT:

		// 武器編集更新処理
		UpdateWeaponEdit();

		break;
	case EditMode::NEXT_MENU:

		for (auto& menuSprite : menuBarSprites_) {
			// メニューバースプライトを画面外に移動させるアニメーション再生
			menuSprite->Animation()->PlayTranslate(
				menuSprite->GetTranslate(),
				{ -menuBarSpriteSize_.x,menuSprite->GetTranslate().y },
				0.2f,
				0.0f,
				Easing::EasingType::IN_BACK,
				SpriteAnimator::PlayMode::ONCE
			);
			// メニューバースプライトのフェードアウトアニメーション再生
			menuSprite->Animation()->PlayFade(1.0f, 0.0f, 0.2f, 0.0f, Easing::EasingType::LINEAR, SpriteAnimator::PlayMode::ONCE);
		}

		// 次のメニューへ移動リクエストフラグを立てる
		nextMenuRequested_ = true;
		break;
	default:
		break;
	}

	// プレビュー用キャラクターモデルの更新
	previewCharacterModel_->Update();

	//武器オブジェクトをキャラクターモデルに装備させる
	Matrix4x4 characterWorldMatrix = previewCharacterModel_->GetWorldMatrix();
	auto RightHandWorldMat = previewCharacterModel_->GetModel()->GetSkeleton()->GetJointWorldMatrix("RightHand", characterWorldMatrix);
	auto LeftHandWorldMat = previewCharacterModel_->GetModel()->GetSkeleton()->GetJointWorldMatrix("LeftHand", characterWorldMatrix);
	auto backLeftWorldMat = previewCharacterModel_->GetModel()->GetSkeleton()->GetJointWorldMatrix("backpack.Left.Tip", characterWorldMatrix);
	auto backRightWorldMat = previewCharacterModel_->GetModel()->GetSkeleton()->GetJointWorldMatrix("backpack.Right.Tip", characterWorldMatrix);

	previewWeaponModels_[static_cast<size_t>(WeaponUnit::R_ARMS)]->SetParent(*RightHandWorldMat);
	previewWeaponModels_[static_cast<size_t>(WeaponUnit::L_ARMS)]->SetParent(*LeftHandWorldMat);
	previewWeaponModels_[static_cast<size_t>(WeaponUnit::L_BACK)]->SetParent(*backLeftWorldMat);
	previewWeaponModels_[static_cast<size_t>(WeaponUnit::R_BACK)]->SetParent(*backRightWorldMat);

	// プレビュー用武器モデル群の更新
	for(auto& weaponModel:previewWeaponModels_){
		weaponModel->Update();
	}

	// カーソルスプライトの更新
	cursorSprite_->Update();
	// メニューバー用スプライト群の更新
	for (auto& sprite : menuBarSprites_) {
		sprite->Update();
	}
	// 武器項目スプライト群の更新
	for(auto& sprite:weaponItemSprites_){
		sprite->Update();
	}
	// キャラクター項目スプライト群の更新
	for(auto& sprite:characterItemSprites_){
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
		menuBarSprites_[i]->UpdateImGui("Weapon Menu Bar Sprite " + std::to_string(i));
	}
	previewCharacterModel_->UpdateImGui("Preview Character Model");
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

		if (editingItemIndex_ < CharacterEditMenuEnum::CHARACTER_CONFIG) {
			//インデックスが武器項目内なら武器編集モードへ
			editModeRequest_ = EditMode::WEAPON_EDIT;

		} else if(editingItemIndex_ == CharacterEditMenuEnum::CHARACTER_CONFIG){
			//インデックスがキャラクター項目ならキャラクター編集モードへ
			editModeRequest_ = EditMode::CHARACTER_EDIT;

		} else if (editingItemIndex_ == CharacterEditMenuEnum::SAVE_CONFIG) {
			//インデックスが設定保存項目なら設定保存処理を実行
			SaveCharacterData();

		} else if (editingItemIndex_ == CharacterEditMenuEnum::NEXT_MENU) {

			//インデックスが次のメニュー項目なら次のメニューへ移動
			editModeRequest_ = EditMode::NEXT_MENU;
		}

		//spriteのアニメーション再生
		for(auto& sprite:menuBarSprites_){
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
// 描画処理 (オブジェクト)
//========================================================================
void CharacterEditTool::DrawObject() {

	// プレビュー用キャラクターモデルの描画
	previewCharacterModel_->Draw();

	// プレビュー用武器モデル群の描画
	for(auto& weaponModel:previewWeaponModels_){
		weaponModel->Draw();
	}
}

//========================================================================
// 描画処理 (UI)
//========================================================================
void CharacterEditTool::DrawUI() {
	// UI描画処理

	// メニューバー用スプライト群の描画
	for (auto& sprite : menuBarSprites_) {
		sprite->Draw();
	}
	// 武器項目スプライト群の描画
	for(auto& sprite:weaponItemSprites_){
		sprite->Draw();
	}
	// キャラクター項目スプライト群の描画
	for(auto& sprite:characterItemSprites_){
		sprite->Draw();
	}
	// カーソルスプライトの描画
	cursorSprite_->Draw();
}

//========================================================================
// キャラクター編集更新処理
//========================================================================
void CharacterEditTool::UpdateCharacterEdit() {
	//　下ボタンが押されたときの処理
	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadDOWN)) {
		editingCharacterIndex_ = (editingCharacterIndex_ + 1) % maxCharacterMenuItems_;
		//オフセット分を越えたら0に戻す
		if (editingCharacterIndex_ >= maxCharacterMenuItems_) {
			editingCharacterIndex_ = 0;
		}
	} else if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::DPadUP)) {
		//上ボタンが押されたときの処理
		editingCharacterIndex_ = (editingCharacterIndex_ - 1 + maxCharacterMenuItems_) % maxCharacterMenuItems_;
		//オフセット分を越えたら最後の項目に戻す
		if (editingCharacterIndex_ >= maxCharacterMenuItems_) {
			editingCharacterIndex_ = maxCharacterMenuItems_ - 1;
		}
	}

	//決定ボタンが押されたときの処理
	if(Input::GetInstance()->TriggerButton(0,GamepadButtonType::A)){

		// キャラクターの適用
		currentCharacterData_.characterInfo = *AttachPlayableCharacterInfo(characterNames_[editingCharacterIndex_]);

		// 武器項目スプライトの移動アニメーション再生
		for(auto& sprite:characterItemSprites_){
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
		for(auto& sprite:characterItemSprites_){
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
		menuBarLeftTop_.y + (menuBarSpriteSize_.y + menuBarSpacing_) * static_cast<float>(editingCharacterIndex_)
		});
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
		//武器モデルの再初期化
		previewWeaponModels_[editingItemIndex_]->Initialize(Object3dCommon::GetInstance(), currentCharacterData_.weaponData[editingItemIndex_].modelFilePath);
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
// オブジェクト反映処理
//========================================================================

void CharacterEditTool::DispatchObject() {

	previewCharacterModel_->Dispatch();
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

//========================================================================
// キャラクターデータ保存処理
//========================================================================
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
	currentCharacterData_.characterInfo.characterName = characterName;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<PlayableCharacterInfo>(characterName + ".json", currentCharacterData_.characterInfo);
	// キャラクター名リストを更新
	characterNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<PlayableCharacterInfo>();

}

//========================================================================
// 武器データ保存処理
//========================================================================
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

void CharacterEditTool::SaveCharacterData() {

	TakeCFrameWork::GetJsonLoader()->SaveJsonData<CharacterData>("PlayerData.json", currentCharacterData_);
}

//========================================================================
// キャラクターデータ読み込み処理
//========================================================================
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
	currentCharacterData_.characterInfo = playableInfo;
}

//========================================================================
// 武器データ読み込み処理
//========================================================================
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
	case WeaponType::WEAPON_TYPE_MACHINE_GUN:
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

//========================================================================
// 全キャラクターデータ読み込み処理
//========================================================================
void CharacterEditTool::LoadAllCharacterData() {

	// 全キャラクターデータの読み込み
	characterNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<PlayableCharacterInfo>();
	for (const auto& characterName : characterNames_) {
		PlayableCharacterInfo playableInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PlayableCharacterInfo>(characterName + ".json");
		playableCharacterInfoMap_[characterName] = playableInfo;
	}
}

//========================================================================
// 全武器データ読み込み処理
//========================================================================
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
		case WeaponType::WEAPON_TYPE_MACHINE_GUN:
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