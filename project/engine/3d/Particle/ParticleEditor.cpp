#include "ParticleEditor.h"
#include "base/ImGuiManager.h"
#include "base/TakeCFrameWork.h"
#include <cassert>

void ParticleEditor::Initialize(ParticleManager* particleManager,ParticleCommon* particleCommon) {

	// ParticleManager初期化
	particleManager_ = particleManager;
	particleCommon_ = particleCommon;
	//デフォルトのパーティクルグループを作成
	currentGroupName_ = "DefaultGroup";
	particleManager_->CreateParticleGroup(particleCommon_,currentGroupName_, ParticleModelType::Primitive, "DefaultTexture.png", PRIMITIVE_PLANE);

	// エディター専用エミッターの初期化
	previewEmitter_ = std::make_unique<ParticleEmitter>();

	// エミッターの初期設定
	emitterTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	previewEmitter_->Initialize("PreviewEmitter", emitterTransform_, emitCount_, emitFrequency_);
	
	//エミッターに発生させるパーティクルを設定
	previewEmitter_->SetParticleName(currentGroupName_);

	//デフォルトプリセットの読み込み
	LoadDefaultPreset();
}

void ParticleEditor::Update() {

	// プレビューエミッターの更新
	previewEmitter_->SetTranslate(emitterTransform_.translate);
	previewEmitter_->SetRotate(emitterTransform_.rotate);
	previewEmitter_->SetScale(emitterTransform_.scale);
	previewEmitter_->SetIsEmit(autoEmit_);

	previewEmitter_->Update();

}

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
		if (ImGui::BeginTabItem("Attributes Editor")) {
			DrawParticleAttributesEditor();
			ImGui::EndTabItem();
		}
		// エミッターコントロール
		if (ImGui::BeginTabBar("Emitter Controls")) {
			DrawEmitterControls();
			ImGui::EndTabItem();
		}
		// プレビュー設定
		if (ImGui::BeginTabItem("Preview Settings")) {
			DrawPreviewSettings();
			ImGui::EndTabItem();
		}
		//プリセット管理
		if (ImGui::BeginTabItem("Preset Manager")) {
			DrawPresetManager();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void ParticleEditor::DrawParticleAttributesEditor() {

	ImGui::Text("Particle Attributes");
	ImGui::Separator();

	//Scale
	ImGui::SliderInt("Scale Setting", reinterpret_cast<int*>(&currentAttributes_.scaleSetting_), 0, 2, "None: %d, Scale Up: %d, Scale Down: %d");
	ImGui::DragFloat3("Scale", &currentAttributes_.scale.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat2("Scale Range", &currentAttributes_.scaleRange.min, 0.01f, 0.0f, 10.0f);

	//Rotate
	ImGui::DragFloat2("Rotate Range", &currentAttributes_.rotateRange.min, 0.01f, -3.14f, 3.14f);

	//Translate,Velocity
	ImGui::DragFloat2("Position Range", &currentAttributes_.positionRange.min, 0.01f, -10.0f, 10.0f);
	if (currentAttributes_.isTraslate_) {
		ImGui::DragFloat2("Velocity Range", &currentAttributes_.velocityRange.min, 0.01f, -10.0f, 10.0f);
	}
	
	//Color
	ImGui::Checkbox("Edit Color", &currentAttributes_.editColor);
	if (currentAttributes_.editColor) {
		ImGui::ColorEdit3("Color", &currentAttributes_.color.x);
		ImGui::DragFloat2("Color Range", &currentAttributes_.colorRange.min, 0.01f, 0.0f, 1.0f);
	}
	//lifetime
	ImGui::DragFloat2("Lifetime Range", &currentAttributes_.lifetimeRange.min, 0.01f, 0.0f, 10.0f);

	//Billboard
	ImGui::Checkbox("Is Billboard", &currentAttributes_.isBillboard);
	//Follow Emitter
	ImGui::Checkbox("Enable Follow Emitter", &currentAttributes_.enableFollowEmitter_);
	ImGui::Checkbox("TranslateUpdate", &currentAttributes_.isTraslate_);

	//設定の適用
	if (ImGui::Button("Apply Attributes")) {
		// 現在のグループに属性を適用
		particleManager_->SetAttributes(currentGroupName_, currentAttributes_);
	}
}

void ParticleEditor::DrawEmitterControls() {

	ImGui::Text("Emitter COntrols");
	ImGui::Separator();

	//エミッターTrasformの表示
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
		TakeCFrameWork::GetParticleManager()->Emit(currentGroupName_, emitterTransform_.translate, emitCount_);
	}

	ImGui::Separator();

	//エミッターのImGui表示
	if (ImGui::CollapsingHeader("Emitter Debug Info")) {

		previewEmitter_->UpdateImGui();
	}
}

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

void ParticleEditor::DrawPresetManager() {

	ImGui::Text("Preset Manager");
	ImGui::Separator();

	// プリセット名の入力
	static char presetNameBuffer[64] = "";
	ImGui::InputText("Preset Name", presetNameBuffer, sizeof(presetNameBuffer));

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
			ImGui::SameLine();
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

void ParticleEditor::SavePreset(const std::string& presetName) {

	if (presetName.empty() || presets_.find(presetName) != presets_.end()) {
		ImGui::Text("Preset already exists or name is empty: %s", presetName.c_str());
		return;
	}
	// 現在の属性をプリセットとして保存
	TakeCFrameWork::GetJsonLoader()->SaveParticlePreset(presetName, currentAttributes_);
	// プリセット名を更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetParticlePresetList();

}

void ParticleEditor::LoadPreset(const std::string& presetName) {

	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	//JSONからプリセットを読み込む
	currentAttributes_ = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset(presetName);

	// 現在のグループに属性を適用
	particleManager_->SetAttributes(currentGroupName_, currentAttributes_);
}

void ParticleEditor::DeletePreset(const std::string& presetName) {
	// プリセットが存在するか確認
	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	// プリセットを削除
	TakeCFrameWork::GetJsonLoader()->DeleteParticlePreset(presetName);

	//プリセット名更新
	RefreshPresetList();
}

void ParticleEditor::LoadDefaultPreset() {
	// デフォルトプリセットの読み込み
	currentAttributes_ = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset("DefaultPreset");
	// 現在のグループに属性を適用
	particleManager_->SetAttributes(currentGroupName_, currentAttributes_);
}

void ParticleEditor::RefreshPresetList() {
	// プリセット名のリストを更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetParticlePresetList();
}
