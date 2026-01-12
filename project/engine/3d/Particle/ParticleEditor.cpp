#include "ParticleEditor.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/TextureManager.h"
#include "engine/Utility/StringUtility.h"
#include <cassert>

using namespace TakeC;

//======================================================================
//			初期化
//======================================================================

void ParticleEditor::Initialize(ParticleCommon* particleCommon) {

	// ParticleManager初期化
	particleCommon_ = particleCommon;
	//デフォルトのパーティクルグループを作成
	currentGroupName_ = "DefaultGroup";
	currentPreset_.textureFilePath = "white1x1.png";
	currentPreset_.presetName = currentGroupName_;
	currentPreset_.primitiveType = PRIMITIVE_PLANE;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(currentGroupName_, "white1x1.png", currentPreset_.primitiveType);

	// エディター専用エミッターの初期化
	previewEmitter_ = std::make_unique<ParticleEmitter>();

	// エミッターの初期設定
	emitterTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	previewEmitter_->Initialize("PreviewEmitter", emitterTransform_, emitCount_, emitFrequency_);
	
	//エミッターに発生させるパーティクルを設定
	previewEmitter_->SetParticleName(currentGroupName_);

	//全プリセットの読み込み
	LoadAllPresets();

	TextureManager::GetInstance().LoadTextureAll();

	//テクスチャ名一覧の取得
	textureFileNames_ = TextureManager::GetInstance().GetLoadedTextureFileNames();
}

//======================================================================
//			更新処理
//======================================================================

void ParticleEditor::Update() {

	if (autoApply_) {
		//属性を自動的に適用する場合、現在の属性をグループに適用
		TakeCFrameWork::GetParticleManager()->SetPreset(currentGroupName_, currentPreset_);
	}

	TakeCFrameWork::GetParticleManager()->Update();

	// プレビューエミッターの更新
	previewEmitter_->SetTranslate(emitterTransform_.translate);
	previewEmitter_->SetRotate(emitterTransform_.rotate);
	previewEmitter_->SetScale(emitterTransform_.scale);
	previewEmitter_->SetIsEmit(autoEmit_);
	previewEmitter_->SetParticleCount(emitCount_);
	previewEmitter_->SetFrequency(emitFrequency_);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup(currentGroupName_)->SetEmitterPosition(emitterTransform_.translate);
	previewEmitter_->Update();

}

//======================================================================
//			ImGui更新処理
//======================================================================

void ParticleEditor::UpdateImGui() {

	// ImGuiのウィンドウを開始
	if (!ImGui::Begin("Particle Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
		// ウィンドウが閉じられた場合は終了
		ImGui::End();
		return;
	}

	// メニューバーの作成
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save Preset")) {
				// プリセットの保存
				SavePreset(selectedPresetName_);
			}
			if (ImGui::MenuItem("Load Preset")) {
				// プリセットの読み込み
				LoadPreset(selectedPresetName_);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//タブ形式のUIの作成
	if (ImGui::BeginTabBar("ParticleEditorTabs")) {
		// パーティクル属性エディター
		if (ImGui::BeginTabItem("Particle Attributes")) {

			//属性の編集
			ImGui::SeparatorText("Attributes Setting");
			DrawParticleAttributesEditor();

			// エミッターコントロール
			ImGui::SeparatorText("Emitter Controls");
			DrawEmitterControls();

			ImGui::EndTabItem();
		}

		//プリセット管理
		if (ImGui::BeginTabItem("Preset Manager")) {

			// プレビュー設定
			DrawPreviewSettings();
			// プリセットマネージャー
			DrawPresetManager();
			// 上書き確認ダイアログ
			DrawOverwriteConfirmDialog();

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

//======================================================================
//			終了処理
//======================================================================

void ParticleEditor::Finalize() {

	// プレビューエミッターの解放
	previewEmitter_.reset();
}

//======================================================================
//			描画処理
//======================================================================

void ParticleEditor::Draw() {
	TakeCFrameWork::GetParticleManager()->Draw();
}

//======================================================================
//			プリセットの編集
//======================================================================

void ParticleEditor::DrawParticleAttributesEditor() {

	//テクスチャが再ロードされたかチェック
	TextureManager::GetInstance().CheckAndReloadTextures();

	ParticleAttributes& attributes = currentPreset_.attribute;

	//attributeの編集
#pragma region coodinate attribute
	ImGui::SeparatorText("Particle Attributes");

	//Scale
	ImGui::SliderInt("Scale Setting", reinterpret_cast<int*>(&attributes.scaleSetting), 0, 2, "None: %d, Scale Up: %d, Scale Down: %d");
	ImGui::DragFloat3("Scale", &attributes.scale.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat2("Scale Range", &attributes.scaleRange.min, 0.01f, 0.0f, 100.0f);

	//Rotate
	ImGui::DragFloat2("Rotate Range", &attributes.rotateRange.min, 0.01f, -3.14f, 3.14f);

	//Translate,Velocity
	ImGui::DragFloat2("Position Range", &attributes.positionRange.min, 0.01f, -100.0f, 100.0f);
	if (attributes.isTranslate) {
		ImGui::DragFloat2("Velocity Range", &attributes.velocityRange.min, 0.01f, -100.0f, 100.0f);
	}
	
	//Color
	ImGui::Checkbox("Edit Color", &attributes.editColor);
	if (attributes.editColor) {
		ImGui::ColorEdit3("Color", &attributes.color.x);
		ImGui::DragFloat2("Color Range", &attributes.colorRange.min, 0.01f, 0.0f, 1.0f);
	}
	//lifetime
	ImGui::DragFloat2("Lifetime Range", &attributes.lifetimeRange.min, 0.01f, 0.0f, 10.0f);

	//Billboard
	ImGui::Checkbox("Is Billboard", &attributes.isBillboard);
	//Follow Emitter
	ImGui::Checkbox("Enable Follow Emitter", &attributes.enableFollowEmitter);
	ImGui::Checkbox("TranslateUpdate", &attributes.isTranslate);
	//isDirectional
	ImGui::Checkbox("Is Directional", &attributes.isDirectional);
	//isParticleTrail
	ImGui::Checkbox("Is ParticleTrail", &attributes.isParticleTrail);
	ImGui::Checkbox("Is EmitterTrail", &attributes.isEmitterTrail);
	ImGui::SliderInt("Particles Per Interpolation", reinterpret_cast<int*>(&attributes.particlesPerInterpolation), 1, 20);
	ImGui::DragFloat("Trail Emit Interval", &attributes.trailEmitInterval, 0.001f, 0.001f, 1.0f);

	//設定の適用
	if (ImGui::Button("Apply Attributes")) {
		// 現在のグループに属性を適用
		TakeCFrameWork::GetParticleManager()->SetPreset(currentGroupName_, currentPreset_);
	}
#pragma endregion

	//テクスチャファイルの設定
#pragma region texture setting
	ImGui::SeparatorText("Texture Setting");

	//テクスチャファイル名の選択
	static int selectedTextureIndex = 0;

	//今の設定がリストにあればインデックスを合わせる
	auto it = std::find(textureFileNames_.begin(), textureFileNames_.end(), currentPreset_.textureFilePath);
	if(it != textureFileNames_.end()) {
		selectedTextureIndex = static_cast<int>(std::distance(textureFileNames_.begin(), it));
	}

	//コンボボックスの表示
	if(ImGui::BeginCombo("Texture File",textureFileNames_.empty() ? "None":textureFileNames_[selectedTextureIndex].c_str())) {
		
		//テクスチャ名一覧の表示
		for (int i = 0; i < textureFileNames_.size(); ++i) {
			bool isSelected = (selectedTextureIndex == i);

			//テクスチャ名の選択
			if (ImGui::Selectable(textureFileNames_[i].c_str(), isSelected)) {
				selectedTextureIndex = i;
				// 選択されたテクスチャをプリセットに設定
				currentPreset_.textureFilePath = textureFileNames_[i];
			}

			//デフォルトフォーカスの設定
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		
		ImGui::EndCombo();
	}
#pragma endregion

	//プリミティブタイプの選択

#pragma region primitive type setting
	ImGui::SeparatorText("Primitive Type");

	// magic_enumを使用してPrimitiveTypeの全情報を取得
	constexpr auto primitiveTypes = magic_enum::enum_entries<PrimitiveType>();

	// 現在の選択インデックスを取得
	int currentTypeIndex = static_cast<int>(magic_enum::enum_index(currentPreset_.primitiveType).value_or(0));

	if (ImGui::BeginCombo("Primitive Type", magic_enum::enum_name(currentPreset_.primitiveType).data())) {
		for (size_t i = 0; i < primitiveTypes.size(); ++i) {
			const bool isSelected = (currentTypeIndex == static_cast<int>(i));
			if (ImGui::Selectable(primitiveTypes[i].second.data(), isSelected)) {
				currentPreset_.primitiveType = primitiveTypes[i].first;
				// プリミティブタイプが変更された場合、グループのプリミティブを更新
				TakeCFrameWork::GetParticleManager()->UpdatePrimitiveType(currentGroupName_, currentPreset_.primitiveType, currentPreset_.primitiveParam);
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragFloat3("Primitive Parameters", &currentPreset_.primitiveParameters.x, 0.01f, 0.0f, 10.0f);
	if (ImGui::Button("Update Primitive")) {
		// プリミティブの更新
		TakeCFrameWork::GetParticleManager()->UpdatePrimitiveType(currentGroupName_, currentPreset_.primitiveType, currentPreset_.primitiveParameters);
	}

#pragma endregion

	//ブレンドモードの設定
#pragma region blend mode setting

	ImGui::SeparatorText("Blend Mode Setting");
	// BlendStateの全情報を取得
	constexpr auto blendStates = magic_enum::enum_entries<BlendState>();
	// 現在の選択インデックスを取得
	int currentBlendIndex = static_cast<int>(magic_enum::enum_index(currentPreset_.blendState).value_or(0));
	if (ImGui::BeginCombo("Blend Mode", magic_enum::enum_name(currentPreset_.blendState).data())) {
		for (size_t i = 0; i < blendStates.size(); ++i) {
			const bool isSelected = (currentBlendIndex == static_cast<int>(i));
			if (ImGui::Selectable(blendStates[i].second.data(), isSelected)) {
				//グループのブレンドモードを更新
				currentPreset_.blendState = blendStates[i].first;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

#pragma endregion

}

//======================================================================
//			エミッターコントロールの描画
//======================================================================

void ParticleEditor::DrawEmitterControls() {

	ImGui::Text("Emitter Controls");
	ImGui::Separator();

	//エミッターTransformの表示
	ImGui::DragFloat3("Emitter Translate", &emitterTransform_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Emitter Rotate", &emitterTransform_.rotate.x, 0.01f, -3.14f, 3.14f);
	ImGui::DragFloat3("Emitter Scale", &emitterTransform_.scale.x, 0.01f, 0.0f, 10.0f);

	//エミッターの発生設定
	ImGui::SliderInt("Emit Count", reinterpret_cast<int*>(&emitCount_), 1, 100, "Count: %d");
	ImGui::SliderFloat("Emit Frequency", &emitFrequency_, 0.0f, 1.0f, "Frequency: %.2f");

	//自動発生のチェックボックス
	ImGui::Checkbox("Auto Emit", &autoEmit_);

	//エミッターの発生ボタン
	if (ImGui::Button("Manual Emit")) {
		emitterDirection_ = previewEmitter_->GetEmitDirection();
		TakeCFrameWork::GetParticleManager()->Emit(currentGroupName_, emitterTransform_.translate,emitterDirection_, emitCount_);
	}

	ImGui::Separator();

	//エミッターのImGui表示
	if (ImGui::CollapsingHeader("Emitter Debug Info")) {

		previewEmitter_->UpdateImGui();
	}
}

//======================================================================
//			プレビュー設定の描画
//======================================================================

void ParticleEditor::DrawPreviewSettings() {

	ImGui::Text("Preview Settings");
	ImGui::Separator();

	// プレビューエミッターの状態表示
	ImGui::Text("Emitter Name: %s", previewEmitter_->GetEmitterName().c_str());
	ImGui::Text("Is Emit: %s", previewEmitter_->IsEmit() ? "True" : "False");
	ImGui::Text("Particle Count: %d", previewEmitter_->GetParticleCount());
	ImGui::Text("Translate: (%.2f, %.2f, %.2f)", emitterTransform_.translate.x, emitterTransform_.translate.y, emitterTransform_.translate.z);
	ImGui::Text("Rotate: (%.2f, %.2f, %.2f)", emitterTransform_.rotate.x, emitterTransform_.rotate.y, emitterTransform_.rotate.z);
	ImGui::Text("Scale: (%.2f, %.2f, %.2f)", emitterTransform_.scale.x, emitterTransform_.scale.y, emitterTransform_.scale.z);
	ImGui::Text("Emit Frequency: %.2f", previewEmitter_->GetFrequency());

	ImGui::Separator();

	//座標のリセット
	if (ImGui::Button("Reset Emitter Translate")) {
		emitterTransform_.translate = { 0.0f, 0.0f, 0.0f };
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop Emit")) {
		//パーティクルを発生を停止
		autoEmit_ = false;
	}
}

//=======================================================================
//			プリセットマネージャーの描画
//=======================================================================

void ParticleEditor::DrawPresetManager() {

	ImGui::Text("Preset Manager");
	ImGui::Separator();

	// プリセット名の入力
	static char presetNameBuffer[64] = "";
	ImGui::InputText("Preset Name", presetNameBuffer, sizeof(presetNameBuffer));

	if (ImGui::Button("Refresh Preset List")) {
		// プリセット名のリストを更新
		RefreshPresetList();
	}

	// プリセットの保存
	if (ImGui::Button("Save current as Preset")) {

		// プリセット名が空でない場合のみ保存
		if (strlen(presetNameBuffer) > 0) {
			SavePreset(std::string(presetNameBuffer));
		}
	}

	ImGui::Separator();

	//プリセット一覧
	for (const std::string& presetName : presetNames_) {

		bool isSelected = (selectedPresetName_ == presetName);

		// プリセット名を表示し、選択可能にする
		if (ImGui::Selectable(presetName.c_str(),isSelected)) {
			selectedPresetName_ = presetName;
		}

		if (isSelected) {
			// 選択されたプリセットを読み込む
			if (ImGui::Button(("Load##" + presetName).c_str())) {
				LoadPreset(presetName);
			}
			// プリセットを削除する
			if (ImGui::Button(("Delete##" + presetName).c_str())) {
				//プリセットの削除
				DeletePreset(presetName);
			}
		}
	}
}

void ParticleEditor::DrawOverwriteConfirmDialog() {
	if (showOverwriteConfirm_) {
		ImGui::OpenPopup("Overwrite Preset?");

		if (ImGui::BeginPopupModal("Overwrite Preset?", &showOverwriteConfirm_, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Preset '%s' already exists.", pendingPresetName_.c_str());
			ImGui::Text("Do you want to overwrite it?");
			ImGui::Separator();

			if (ImGui::Button("Yes", ImVec2(120, 0))) {
				// 上書き保存を実行
				currentPreset_.presetName = pendingPresetName_;
				TakeCFrameWork::GetJsonLoader()->SaveJsonData<ParticlePreset>(pendingPresetName_ + ".json", currentPreset_);
				presetNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<ParticlePreset>();

				showOverwriteConfirm_ = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0))) {
				showOverwriteConfirm_ = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}

//========================================================================
//			プリセットの保存
//========================================================================

void ParticleEditor::SavePreset(const std::string& presetName) {

	// プリセット名が空でないか、既に存在しないか確認
	if (presetName.empty() || presets_.find(presetName) != presets_.end()) {
		ImGui::Text("Preset already exists or name is empty: %s", presetName.c_str());
		return;
	}

	// プリセットが既に存在する場合は上書き確認
	if (presets_.find(presetName) != presets_.end()) {
		// 上書き確認フラグを立てる（次のフレームで確認ダイアログを表示）
		showOverwriteConfirm_ = true;
		pendingPresetName_ = presetName;
		return;
	}

	// 現在の属性をプリセットとして保存
	currentPreset_.presetName = presetName;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<ParticlePreset>(presetName + ".json", currentPreset_);
	// プリセット名を更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<ParticlePreset>();

}

//=======================================================================
//			プリセットの読み込み
//========================================================================

void ParticleEditor::LoadPreset(const std::string& presetName) {

	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	//JSONからプリセットを読み込む
	currentPreset_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>(presetName + ".json");

	// 現在のグループに属性を適用
	TakeCFrameWork::GetParticleManager()->SetPreset(currentGroupName_, currentPreset_);
}

//========================================================================
//			プリセットの削除
//========================================================================

void ParticleEditor::DeletePreset(const std::string& presetName) {
	// プリセットが存在するか確認
	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	// プリセットを削除
	TakeCFrameWork::GetJsonLoader()->DeleteJsonData<ParticlePreset>(presetName + ".json");

	//プリセット名更新
	RefreshPresetList();
}

//=======================================================================
//			デフォルトプリセットの読み込み
//========================================================================

void ParticleEditor::LoadDefaultPreset() {
	// デフォルトプリセットの読み込み
	currentPreset_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>("DefaultPreset.json");
	// 現在のグループに属性を適用
	TakeCFrameWork::GetParticleManager()->SetPreset(currentGroupName_, currentPreset_);
}

//=======================================================================
//			全てのプリセットを読み込み
//========================================================================

void ParticleEditor::LoadAllPresets() {

	//プリセットのリストを取得
	RefreshPresetList();

	// 各プリセットを読み込み
	for (const std::string& presetName : presetNames_) {
		// プリセットを読み込む
		presets_[presetName] = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>(presetName + ".json");
	}

	// デバッグ出力（必要に応じて削除）
	OutputDebugStringA(("Loaded " + std::to_string(presets_.size()) + " presets from folder\n").c_str());
}

//=======================================================================
//			プリセット名のリストを更新
//========================================================================

void ParticleEditor::RefreshPresetList() {
	// プリセット名のリストを更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<ParticlePreset>();
}