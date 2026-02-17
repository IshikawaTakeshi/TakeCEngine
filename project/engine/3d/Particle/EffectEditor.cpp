#include "EffectEditor.h"
#include "base/TakeCFrameWork.h"
#include "base/ImGuiManager.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>

using namespace TakeC;

//==================================================================================
// 初期化
//==================================================================================
void EffectEditor::Initialize(ParticleCommon* particleCommon) {
	particleCommon_ = particleCommon;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	// デフォルトのエフェクト設定を作成
	currentConfig_.effectName = "NewEffect";
	currentConfig_.isLooping = false;
	currentConfig_.totalDuration = -1.0f;
	currentConfig_.defaultScale = {1.0f, 1.0f, 1.0f};

	// プレビュー用エフェクトグループを作成
	previewEffect_ = std::make_unique<EffectGroup>();

	//利用可能なエフェクトを読み込み
	LoadSavedEffects();

	// 利用可能なプリセットを読み込み
	LoadAvailablePresets();
}

//==================================================================================
// 終了処理
//==================================================================================
void EffectEditor::Finalize() {
	previewEffect_.reset();
	savedEffects_.clear();
	availablePresets_.clear();
}

//==================================================================================
// 更新処理
//==================================================================================
void EffectEditor::Update() {

	// パーティクルマネージャーの更新
	TakeCFrameWork::GetParticleManager()->Update();

	// プレビューエフェクトの更新
	if (previewEffect_ && editorState_ == EditorState::Playing) {
		previewEffect_->Update();
	}
}

//==================================================================================
// ImGui更新処理
//==================================================================================
void EffectEditor::UpdateImGui() {
	DrawMainWindow();
}

//==================================================================================
// 描画処理
//==================================================================================
void EffectEditor::Draw() {
	TakeCFrameWork::GetParticleManager()->Draw();
}

//==================================================================================
// メインウィンドウ
//==================================================================================
void EffectEditor::DrawMainWindow() {
	ImGui::Begin("Effect Editor");
	//プレビューコントロールタブ
	DrawPreviewControlTab();

	//エディタータブ群
	if (ImGui::BeginTabBar("EffectEditorTabs")) {

		//エフェクトリストタブ
		if (ImGui::BeginTabItem("Effect List")) {
			DrawEffectListTab();
			ImGui::NewLine();
			ImGui::NewLine();
			DrawFileOperationTab();
			ImGui::EndTabItem();
		}
		//エフェクト設定タブ
		if (ImGui::BeginTabItem("Effect Settings")) {
			DrawEffectSettingsTab();
			ImGui::EndTabItem();
		}
		//エミッターリストタブ
		if (ImGui::BeginTabItem("Emitter List")) {
			DrawEmitterListTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

//==================================================================================
// エフェクトリストタブ
//==================================================================================
void EffectEditor::DrawEffectListTab() {
	// 保存済みエフェクトリスト
	for (int i = 0; i < savedEffects_.size(); ++i) {
		bool isSelected = (selectedEffectIndex_ == i);

		if (ImGui::Selectable(savedEffects_[i].effectName.c_str(), isSelected)) {
			// エフェクトを選択
			SelectEffect(i);
		}

		// 右クリックメニュー
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Duplicate")) {
				DuplicateEffect(i);
			}
			if (ImGui::MenuItem("Delete")) {
				DeleteEffect(i);
			}
			ImGui::EndPopup();
		}
	}
}

//==================================================================================
// エフェクト設定タブ
//==================================================================================
void EffectEditor::DrawEffectSettingsTab() {
	// エフェクト名
	ImGui::InputText("Effect Name", effectNameBuffer_, sizeof(effectNameBuffer_));
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		currentConfig_.effectName = effectNameBuffer_;
		RebuildEffectGroup();
	}

	ImGui::Separator();

	// ループ設定
	bool isLooping = currentConfig_.isLooping;
	if (ImGui::Checkbox("Looping", &isLooping)) {
		currentConfig_.isLooping = isLooping;
	}

	// 持続時間
	float totalDuration = currentConfig_.totalDuration;
	if (ImGui::DragFloat("Total Duration", &totalDuration, 0.1f, -1.0f, 100.0f)) {
		currentConfig_.totalDuration = totalDuration;
	}
	ImGui::SameLine();
	// 無限に再生させるボタン
	if (ImGui::Button("Infinite")) {
		currentConfig_.totalDuration = -1.0f;
		RebuildEffectGroup();
	}

	// デフォルトスケール操作
	ImGui::DragFloat3("Default Scale", &currentConfig_.defaultScale.x, 0.01f, 0.01f, 10.0f);

	ImGui::Separator();

	// エミッター数表示
	ImGui::Text("Emitters: %d", currentConfig_.emitters.size());
}

//==================================================================================
// エミッターリストタブ
//==================================================================================
void EffectEditor::DrawEmitterListTab() {
	// エミッター追加ボタン
	if (ImGui::Button("Add Emitter", ImVec2(-1, 0))) {
		AddEmitter();
	}
	// エミッター削除ボタン
	if(ImGui::Button("Remove Emitter", ImVec2(-1, 0))){
		if(selectedEmitterIndex_ >= 0){
			RemoveEmitter(selectedEmitterIndex_);
		}
	}

	//余白+区切り線
	ImGui::Separator();
	ImGui::NewLine();

	// エミッターリスト
	for (int i = 0; i < currentConfig_.emitters.size(); ++i) {
		auto& emitter = currentConfig_.emitters[i];
		bool isSelected = (selectedEmitterIndex_ == i);

		// 選択可能なアイテム
		if (ImGui::Selectable(emitter.emitterName.c_str(), isSelected)) {
			SelectEmitter(i);
		}

		// 右クリックメニュー
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Duplicate")) {
				DuplicateEmitter(i);
			}
			if (ImGui::MenuItem("Move Up", nullptr, false, i > 0)) {
				MoveEmitterUp(i);
			}
			if (ImGui::MenuItem("Move Down", nullptr, false, i < currentConfig_.emitters.size() - 1)) {
				MoveEmitterDown(i);
			}
			if (ImGui::MenuItem("Delete")) {
				RemoveEmitter(i);
			}
			ImGui::EndPopup();
		}
	}

	//余白+区切り線
	ImGui::NewLine();
	ImGui::Separator();
	// エミッター設定タブの表示
	DrawEmitterSettingsTab();
}

//==================================================================================
// エミッター設定タブ
//==================================================================================
void EffectEditor::DrawEmitterSettingsTab() {
	if (selectedEmitterIndex_ >= 0 && selectedEmitterIndex_ < currentConfig_.emitters.size()) {
		auto& emitter = currentConfig_.emitters[selectedEmitterIndex_];

		// 変更フラグ（Rebuild不要な、軽量な更新用）
		bool isChanged = false;

		// ---------------------------------------------------------
		// Rebuildが必要な項目（構造が変わるもの）
		// ---------------------------------------------------------

		// エミッター名
		char nameBuffer[256];
		strcpy_s(nameBuffer, emitter.emitterName.c_str());
		if (ImGui::InputText("Emitter Name", nameBuffer, sizeof(nameBuffer))) {
			emitter.emitterName = nameBuffer;
			RebuildEffectGroup(); // 名前が変わると検索等に影響するためRebuild推奨
		}

		ImGui::Separator();

		// プリセット選択
		if (ImGui::BeginCombo("Particle Preset", emitter.presetFilePath.c_str())) {
			for (int i = 0; i < availablePresets_.size(); ++i) {
				bool isSelected = (emitter.presetFilePath == availablePresets_[i]);
				if (ImGui::Selectable(availablePresets_[i].c_str(), isSelected)) {
					emitter.presetFilePath = availablePresets_[i];
					RebuildEffectGroup(); // 読み込むファイルが変わるのでRebuild必須
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		// ---------------------------------------------------------
		// 即時反映可能な項目（パラメータのみ）
		// ---------------------------------------------------------

		// 位置オフセット
		if (ImGui::DragFloat3("Position Offset", &emitter.positionOffset.x, 0.1f)) {
			isChanged = true;
		}
		// 回転オフセット
		if (ImGui::DragFloat3("Rotation Offset", &emitter.rotationOffset.x, 0.01f)) {
			isChanged = true;
		}

		ImGui::Separator();

		// 遅延時間
		if (ImGui::DragFloat("Delay Time", &emitter.delayTime, 0.01f, 0.0f, 10.0f)) {
			isChanged = true;
		}

		// 持続時間
		if (ImGui::DragFloat("Duration", &emitter.duration, 0.1f, -1.0f, 100.0f)) {
			isChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Infinite##Duration")) {
			emitter.duration = -1.0f;
			isChanged = true;
		}

		ImGui::Separator();

		// 自動開始（動作フラグなので即時反映でも良いが、Rebuildの方が安全な場合もある。今回は即時反映に含める）
		bool autoStart = emitter.autoStart;
		if (ImGui::Checkbox("Auto Start", &autoStart)) {
			emitter.autoStart = autoStart;
			RebuildEffectGroup(); // Start状態のリセットを含むためRebuildにしておくのが無難
		}

		// 発生回数
		int emitCount = static_cast<int>(emitter.emitCount);
		if (ImGui::DragInt("Emit Count", &emitCount, 1, 0, 1000)) {
			emitter.emitCount = static_cast<uint32_t>(emitCount);
			isChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Continuous##EmitCount")) {
			emitter.emitCount = 0;
			isChanged = true;
		}

		//変更があった場合、プレビューへ即時反映
		if (isChanged && previewEffect_) {
			previewEffect_->UpdateEmitterConfig(selectedEmitterIndex_, emitter);
		}

	} else {
		ImGui::Text("No emitter selected");
	}
}

//==================================================================================
// プレビューコントロールタブ
//==================================================================================
void EffectEditor::DrawPreviewControlTab() {

	// 再生コントロール
	if (ImGui::Button("Play", ImVec2(60, 30))) {
		PlayPreview();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop", ImVec2(60, 30))) {
		StopPreview();
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause", ImVec2(60, 30))) {
		PausePreview();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(60, 30))) {
		ResetPreview();
	}

	ImGui::Separator();

	// プレビュー位置
	if (ImGui::DragFloat3("Preview Position", &previewPosition_.x, 0.1f)) {
		if (previewEffect_) {
			previewEffect_->SetPosition(previewPosition_);
		}
	}

	// プレビュー回転
	if (ImGui::DragFloat3("Preview Rotation", &previewRotation_.x, 0.01f)) {
		if (previewEffect_) {
			previewEffect_->SetRotation(previewRotation_);
		}
	}

	// プレビュースケール
	if (ImGui::DragFloat3("Preview Scale", &previewScale_.x, 0.01f, 0.01f, 10.0f)) {
		if (previewEffect_) {
			previewEffect_->SetScale(previewScale_);
		}
	}

	ImGui::Separator();

	// エフェクト情報
	if (previewEffect_) {
		ImGui::Text("Playing: %s", previewEffect_->IsPlaying() ? "Yes" : "No");
		ImGui::Text("Finished: %s", previewEffect_->IsFinished() ? "Yes" : "No");
		ImGui::Text("Elapsed: %.2f", previewEffect_->GetElapsedTime());
	}
}

//==================================================================================
// ファイル操作タブ
//==================================================================================
void EffectEditor::DrawFileOperationTab() {
	// 保存
	ImGui::Text("Effect Name: %s", currentConfig_.effectName.c_str());

	if (ImGui::Button("Save Effect", ImVec2(-1, 0))) {
		std::string fullPath = currentConfig_.effectName + ".json";
		SaveEffectToJson(fullPath);
	}

	ImGui::Separator();

	// 読み込み
	if (ImGui::Button("Load Effect", ImVec2(-1, 0))) {
		// ファイル選択ダイアログを開く（実装省略）
		// LoadEffectFromJson("path/to/file.json");
	}

	ImGui::Separator();

	//エフェクトのリロード
	if (ImGui::Button("Reload Effects", ImVec2(-1, 0))) {
		LoadSavedEffects();
	}

	// プリセットのリロード
	if (ImGui::Button("Reload Presets", ImVec2(-1, 0))) {
		LoadAvailablePresets();
	}
}

//==================================================================================
// 新しいエフェクトを作成
//==================================================================================
void EffectEditor::CreateNewEffect() {
	currentConfig_ = EffectGroupConfig();
	currentConfig_.effectName = "NewEffect";
	currentConfig_.isLooping = false;
	currentConfig_.totalDuration = -1.0f;
	currentConfig_.defaultScale = {1.0f, 1.0f, 1.0f};

	selectedEmitterIndex_ = -1;
	RebuildEffectGroup();
}

//==================================================================================
// エフェクトを複製
//==================================================================================
void TakeC::EffectEditor::DuplicateEffect(int index) {

	if (index >= 0 && index < savedEffects_.size()) {
		EffectGroupConfig newEffect = savedEffects_[index];
		newEffect.effectName += "_Copy";
		savedEffects_.push_back(newEffect);
	}
}

//==================================================================================
// エフェクトを削除
//==================================================================================
void TakeC::EffectEditor::DeleteEffect(int index) {

	if (index >= 0 && index < savedEffects_.size()) {
		savedEffects_.erase(savedEffects_.begin() + index);
		if (selectedEffectIndex_ == index) {
			selectedEffectIndex_ = -1;
		} else if (selectedEffectIndex_ > index) {
			selectedEffectIndex_--;
		}
	}
}

//==================================================================================
// エフェクトを選択
//==================================================================================
void TakeC::EffectEditor::SelectEffect(int index) {

	if (index >= 0 && index < savedEffects_.size()) {
		currentConfig_ = savedEffects_[index];
		selectedEffectIndex_ = index;
		selectedEmitterIndex_ = -1;
		strcpy_s(effectNameBuffer_, currentConfig_.effectName.c_str());
		RebuildEffectGroup();
	}
}

void TakeC::EffectEditor::LoadSavedEffects() {

	savedEffects_.clear();
	// effects/ディレクトリ内の全JSONファイルを読み込み
	std::string effectsDir = "Resources/JsonLoader/EffectGroup/";
	if (std::filesystem::exists(effectsDir)) {
		for (const auto& entry : std::filesystem::directory_iterator(effectsDir)) {
			if (entry.path().extension() == ".json") {
				EffectGroupConfig effectConfig;
				effectConfig = TakeCFrameWork::GetJsonLoader()->LoadJsonData<EffectGroupConfig>(entry.path().filename().string());
				savedEffects_.push_back(effectConfig);
			}
		}
	}
}

//==================================================================================
// エミッターを追加
//==================================================================================
void EffectEditor::AddEmitter() {
	EmitterConfig newEmitter;
	newEmitter.emitterName = "NewEmitter_" + std::to_string(currentConfig_.emitters.size());
	newEmitter.presetFilePath = availablePresets_.empty() ? "" : availablePresets_[0];
	newEmitter.positionOffset = {0.0f, 0.0f, 0.0f};
	newEmitter.rotationOffset = {0.0f, 0.0f, 0.0f};
	newEmitter.delayTime = 0.0f;
	newEmitter.duration = 1.0f;
	newEmitter.autoStart = true;
	newEmitter.emitCount = 10;

	currentConfig_.emitters.push_back(newEmitter);
	RebuildEffectGroup();
}

//==================================================================================
// エミッターを削除
//==================================================================================
void EffectEditor::RemoveEmitter(int index) {
	if (index >= 0 && index < currentConfig_.emitters.size()) {
		currentConfig_.emitters.erase(currentConfig_.emitters.begin() + index);

		if (selectedEmitterIndex_ == index) {
			selectedEmitterIndex_ = -1;
		} else if (selectedEmitterIndex_ > index) {
			selectedEmitterIndex_--;
		}

		RebuildEffectGroup();
	}
}

//==================================================================================
// エミッターを複製
//==================================================================================
void TakeC::EffectEditor::DuplicateEmitter(int index) {

	if (index >= 0 && index < currentConfig_.emitters.size()) {
		EmitterConfig newEmitter = currentConfig_.emitters[index];
		newEmitter.emitterName += "_Copy";
		currentConfig_.emitters.insert(currentConfig_.emitters.begin() + index + 1, newEmitter);
		RebuildEffectGroup();
	}
}

//==================================================================================
// エミッターを並び替え（上へ）
//==================================================================================
void TakeC::EffectEditor::MoveEmitterUp(int index) {

	if (index > 0 && index < currentConfig_.emitters.size()) {
		std::swap(currentConfig_.emitters[index], currentConfig_.emitters[index - 1]);
		if (selectedEmitterIndex_ == index) {
			selectedEmitterIndex_--;
		} else if (selectedEmitterIndex_ == index - 1) {
			selectedEmitterIndex_++;
		}
		RebuildEffectGroup();
	}
}

//==================================================================================
// エミッターを並び替え（下へ）
//==================================================================================
void TakeC::EffectEditor::MoveEmitterDown(int index) {

	if (index >= 0 && index < currentConfig_.emitters.size() - 1) {
		std::swap(currentConfig_.emitters[index], currentConfig_.emitters[index + 1]);
		if (selectedEmitterIndex_ == index) {
			selectedEmitterIndex_++;
		} else if (selectedEmitterIndex_ == index + 1) {
			selectedEmitterIndex_--;
		}
		RebuildEffectGroup();
	}
}

//==================================================================================
// エミッターを選択
//==================================================================================
void TakeC::EffectEditor::SelectEmitter(int index) {

	if (index >= 0 && index < currentConfig_.emitters.size()) {
		selectedEmitterIndex_ = index;
	}
}

//==================================================================================
// プレビューを再生
//==================================================================================
void EffectEditor::PlayPreview() {
	if (previewEffect_) {
		previewEffect_->Play(previewPosition_);
		editorState_ = EditorState::Playing;
	}
}

//==================================================================================
// プレビューを停止
//==================================================================================
void EffectEditor::StopPreview() {
	if (previewEffect_) {
		previewEffect_->Stop();
		editorState_ = EditorState::Idle;
	}
}

//==================================================================================
// プレビューをリセット
//==================================================================================
void TakeC::EffectEditor::PausePreview() {

	if (previewEffect_) {
		previewEffect_->Pause();
		editorState_ = EditorState::Paused;
	}
}

//==================================================================================
// プレビューをリセット
//==================================================================================
void TakeC::EffectEditor::ResetPreview() {

	if (previewEffect_) {
		previewEffect_->Reset();
		editorState_ = EditorState::Idle;
	}
}

//==================================================================================
// エフェクトグループを再構築
//==================================================================================
void EffectEditor::RebuildEffectGroup() {
	previewEffect_ = std::make_unique<EffectGroup>();
	previewEffect_->Initialize(currentConfig_);
	previewEffect_->SetPosition(previewPosition_);
	previewEffect_->SetRotation(previewRotation_);
	previewEffect_->SetScale(previewScale_);
}

//==================================================================================
// 利用可能なプリセットを読み込み
//==================================================================================
void EffectEditor::LoadAvailablePresets() {
	availablePresets_.clear();

	// presets/ディレクトリ内の全JSONファイルを読み込み
	std::string presetDir = "Resources/JsonLoader/ParticlePresets/";
	if (std::filesystem::exists(presetDir)) {
		for (const auto& entry : std::filesystem::directory_iterator(presetDir)) {
			if (entry.path().extension() == ".json") {
				availablePresets_.push_back(entry.path().filename().string());
			}
		}
	}
}

//==================================================================================
// JSONに保存
//==================================================================================
void EffectEditor::SaveEffectToJson(const std::string& filePath) {
	// JSON保存処理（TakeCFrameWork::GetJsonLoader()を使用）
	TakeCFrameWork::GetJsonLoader()->SaveJsonData(filePath, currentConfig_);
}

//==================================================================================
// JSONから読み込み
//==================================================================================
void EffectEditor::LoadEffectFromJson(const std::string& filePath) {
	// JSON読み込み処理
	currentConfig_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<EffectGroupConfig>(filePath);
	RebuildEffectGroup();
}

//==================================================================================
// プレビューギズモを描画
//==================================================================================
void EffectEditor::DrawPreviewGizmo() {
	// ワイヤーフレームで位置を表示
	TakeCFrameWork::GetWireFrame()->DrawSphere(previewPosition_, 0.5f, {1.0f, 0.0f, 0.0f, 1.0f});
}

//==================================================================================
// エミッターギズモを描画
//==================================================================================
void EffectEditor::DrawEmitterGizmo() {
	if (selectedEmitterIndex_ >= 0 && selectedEmitterIndex_ < currentConfig_.emitters.size()) {
		auto& emitter = currentConfig_.emitters[selectedEmitterIndex_];
		Vector3 emitterPos = previewPosition_ + emitter.positionOffset;
		TakeCFrameWork::GetWireFrame()->DrawSphere(emitterPos, 0.3f, {0.0f, 1.0f, 0.0f, 1.0f});
	}
}

//==================================================================================
// エフェクトグループ設定を更新
//==================================================================================
void TakeC::EffectEditor::UpdateEffectGroupConfig() {


	// エフェクト名を更新
	if (std::string(effectNameBuffer_) != currentConfig_.effectName) {
		currentConfig_.effectName = std::string(effectNameBuffer_);
	}
	// プレビューエフェクトが存在しない場合は更新しない
	if (!previewEffect_) {
		return;
	}
	// ループ設定
	if (previewEffect_->IsLooping() != currentConfig_.isLooping) {
		previewEffect_->SetLooping(currentConfig_.isLooping);
	}
	// デフォルトスケール
	if (previewEffect_->GetScale() != currentConfig_.defaultScale) {
		previewEffect_->SetScale(currentConfig_.defaultScale);
	}

	for (size_t i = 0; i < currentConfig_.emitters.size(); ++i) {
		const auto& emitterConfig = currentConfig_.emitters[i];

		// 位置オフセット
		previewEffect_->SetEmitterOffset(emitterConfig.emitterName, emitterConfig.positionOffset);

		// 回転オフセット
		previewEffect_->SetEmitterRotation(emitterConfig.emitterName, emitterConfig.rotationOffset);

		// 遅延時間
		previewEffect_->SetEmitterDelay(emitterConfig.emitterName, emitterConfig.delayTime);

		// アクティブ状態（持続時間が0以下なら無効化）
		bool shouldBeActive = emitterConfig.duration != 0.0f;
		previewEffect_->SetEmitterActive(emitterConfig.emitterName, shouldBeActive);
	}
}
