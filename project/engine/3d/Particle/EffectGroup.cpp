#include "EffectGroup.h"
#include "engine/base/TakeCFrameWork.h"
#include "Math/Quaternion.h"
#include "Math/MatrixMath.h"
#include "Math/Vector3Math.h"
#include <cassert>

using namespace TakeC;

//==================================================================================
// JSONファイルから初期化
//==================================================================================
void EffectGroup::Initialize(const std::string& configFilePath) {
	// JSONから設定を読み込み
	config_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<EffectGroupConfig>(configFilePath);

	// 共通初期化
	InitializeCommon();
}

//==================================================================================
// 設定から初期化
//==================================================================================
void EffectGroup::Initialize(const EffectGroupConfig& config) {
	config_ = config;
	InitializeCommon();
}

void TakeC::EffectGroup::SetParentMatrix(const Matrix4x4* parentMatrix) {
	parentMatrix_ = parentMatrix;	
}

//==================================================================================
// 共通初期化処理
//==================================================================================
void EffectGroup::InitializeCommon() {
	// デルタタイム取得
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	// デフォルトスケール適用
	transform_.scale = config_.defaultScale;

	// エミッターインスタンスを作成
	CreateEmitterInstances();
}

//==================================================================================
// エミッターインスタンスを作成
//==================================================================================
void EffectGroup::CreateEmitterInstances() {
	emitterInstances_.clear();
	emitterInstances_.reserve(config_.emitters.size());

	for (const auto& emitterConfig : config_.emitters) {
		EmitterInstance instance;
		instance.config = emitterConfig;
		instance.emitter = std::make_unique<ParticleEmitter>();

		if (emitterConfig.presetFilePath.empty()) {
			// プリセットファイルパスが空の場合,DefaultParticlePreset.jsonを使用
			instance.emitter->Initialize(emitterConfig.emitterName, "DefaultParticlePreset.json");
		}else {
			// 指定されたプリセットファイルパスを使用
			instance.emitter->Initialize(emitterConfig.emitterName, emitterConfig.presetFilePath);
		}
		instance.emitter->SetIsEmit(false); // 最初は停止

		emitterInstances_.push_back(std::move(instance));
	}
}

//==================================================================================
// 更新処理
//==================================================================================
void EffectGroup::Update() {
	if (!isPlaying_ || isPaused_) {
		return;
	}

	totalElapsedTime_ += deltaTime_;

	// -------------------------------------------------------------
	// 親行列がある場合、位置情報を同期する
	// -------------------------------------------------------------
	if (parentMatrix_) {
		// 親行列から位置成分を抽出して自身のtranslateに適用
		transform_.translate.x = parentMatrix_->m[3][0];
		transform_.translate.y = parentMatrix_->m[3][1];
		transform_.translate.z = parentMatrix_->m[3][2];
	}

	// グループ全体のワールド行列を計算
	Matrix4x4 groupWorldMatrix = MatrixMath::MakeAffineMatrix(
		transform_.scale, 
		transform_.rotate, 
		transform_.translate
	);

	// 親行列がある場合は乗算
	if (parentMatrix_) {
		groupWorldMatrix = MatrixMath::Multiply(groupWorldMatrix, *parentMatrix_);
	}

	bool anyEmitterActive = false;
	// グループの回転クォータニオンを作成
	Quaternion groupRot = QuaternionMath::FromEuler(transform_.rotate);

	// 親行列がある場合、親の回転も考慮する必要があるかもしれません。
	// ここでは親の位置だけ同期しているので、回転はEffectGroup自身のものを使います。

	for (auto& instance : emitterInstances_) {
		instance.elapsedTime += deltaTime_;

		if (!instance.hasStarted) {
			CheckEmitterStart(instance);
		}

		if (instance.isActive) {
			anyEmitterActive = true;

			// ▼▼▼ 行列を使わずベクトル演算で計算 ▼▼▼

			// A. エミッターのローカルオフセットを取り出す
			Vector3 offset = instance.config.positionOffset;

			// B. グループのスケールを適用
			offset.x *= transform_.scale.x;
			offset.y *= transform_.scale.y;
			offset.z *= transform_.scale.z;

			// C. グループの回転でオフセットを回転させる
			// (オフセット(0,0,0)ならここは(0,0,0)のまま)
			offset = QuaternionMath::RotateVector(offset, groupRot);

			// D. グループの現在位置(親位置)を加算して最終位置とする
			Vector3 finalPos = transform_.translate + offset;

			// 適用
			instance.emitter->SetTranslate(finalPos);

			// E. 回転の適用 (単純加算)
			// グループ回転 + エミッターオフセット回転
			Vector3 finalRot = transform_.rotate + instance.config.rotationOffset;
			instance.emitter->SetRotate(finalRot);

			instance.emitter->Update();
			CheckEmitterEnd(instance);
		}
	}

	// エフェクト全体の終了判定
	if (!anyEmitterActive && !config_.isLooping) {
		isFinished_ = true;
		isPlaying_ = false;
	}

	// ループ再生
	if (!isFinished_ && config_.isLooping) {
		Reset();
		Play(transform_.translate);
	}
}
//==================================================================================
// 実行中にエミッターの設定を更新する
//==================================================================================
void TakeC::EffectGroup::UpdateEmitterConfig(int index, const EmitterConfig& config) {

	// 範囲チェック
	if (index < 0 || index >= emitterInstances_.size()) {
		return;
	}

	// 設定を上書き
	auto& instance = emitterInstances_[index];
	instance.config = config;

	// 即座に位置・回転を反映させる（一時停止中でも動かせるようにするため）

	// オフセットを適用
	Vector3 offsetPos = instance.config.positionOffset;

	// スケールを適用
	offsetPos.x *= transform_.scale.x;
	offsetPos.y *= transform_.scale.y;
	offsetPos.z *= transform_.scale.z;

	// 回転を適用（quaternion使用）
	Quaternion rot = QuaternionMath::FromEuler(transform_.rotate);
	offsetPos = QuaternionMath::RotateVector(offsetPos, rot);

	// 最終位置を設定
	Vector3 finalPosition = transform_.translate + offsetPos;
	instance.emitter->SetTranslate(finalPosition);

	// 回転も適用
	Vector3 finalRotation = transform_.rotate + instance.config.rotationOffset;
	instance.emitter->SetRotate(finalRotation);
}

//==================================================================================
// エミッターの開始判定
//==================================================================================
void EffectGroup::CheckEmitterStart(EmitterInstance& instance) {
	// 遅延時間を過ぎたら開始
	if (instance.elapsedTime >= instance.config.delayTime) {
		instance.hasStarted = true;
		instance.isActive = true;
		instance.emitter->SetIsEmit(true);
	}
}

//==================================================================================
// エミッターの終了判定
//==================================================================================
void EffectGroup::CheckEmitterEnd(EmitterInstance& instance) {
	// 持続時間が設定されている場合
	if (instance.config.duration > 0.0f) {
		if (instance.elapsedTime >= instance.config.delayTime + instance.config.duration) {
			instance.isActive = false;
			instance.emitter->SetIsEmit(false);
		}
	}

	// 発生回数が設定されている場合
	if (instance.config.emitCount > 0) {
		if (instance.emitCounter >= instance.config.emitCount) {
			instance.isActive = false;
			instance.emitter->SetIsEmit(false);
		}
	}
}

//==================================================================================
// エミッター位置の更新
//==================================================================================
void EffectGroup::UpdateEmitterPositions() {
	for (auto& instance : emitterInstances_) {
		// オフセットを適用
		Vector3 offsetPos = instance.config.positionOffset;

		// スケールを適用
		offsetPos.x *= transform_.scale.x;
		offsetPos.y *= transform_.scale.y;
		offsetPos.z *= transform_.scale.z;

		// 回転を適用（quaternion使用）
		Quaternion rot = QuaternionMath::FromEuler(transform_.rotate);
		offsetPos = QuaternionMath::RotateVector(offsetPos, rot);

		// 最終位置を設定
		Vector3 finalPosition = transform_.translate + offsetPos;
		instance.emitter->SetTranslate(finalPosition);

		// 回転も適用
		Vector3 finalRotation = transform_.rotate + instance.config.rotationOffset;
		instance.emitter->SetRotate(finalRotation);
	}
}

//==================================================================================
// エフェクトを再生
//==================================================================================
void EffectGroup::Play(const Vector3& position) {
	Reset();
	transform_.translate = position;
	isPlaying_ = true;
	isFinished_ = false;
	isPaused_ = false;
}

//==================================================================================
// エフェクトを停止
//==================================================================================
void EffectGroup::Stop() {
	isPlaying_ = false;
	isFinished_ = true;

	// 全エミッターを停止
	for (auto& instance : emitterInstances_) {
		instance.emitter->SetIsEmit(false);
		instance.isActive = false;
	}
}

//==================================================================================
// エフェクトを一時停止
//==================================================================================
void TakeC::EffectGroup::Pause() {

	isPaused_ = true;
	// 全エミッターを停止
	for (auto& instance : emitterInstances_) {
		instance.emitter->SetIsEmit(false);
	}
}

//==================================================================================
// エフェクトを再開
//==================================================================================
void TakeC::EffectGroup::Resume() {

	isPaused_ = false;
	// 全エミッターを再開
	for (auto& instance : emitterInstances_) {
		if (instance.isActive) {
			instance.emitter->SetIsEmit(true);
		}
	}
}

//==================================================================================
// エフェクトをリセット
//==================================================================================
void EffectGroup::Reset() {
	totalElapsedTime_ = 0.0f;
	isFinished_ = false;

	for (auto& instance : emitterInstances_) {
		instance.elapsedTime = 0.0f;
		instance.emitCounter = 0;
		instance.isActive = false;
		instance.hasStarted = false;
		instance.emitter->Reset();
		instance.emitter->SetIsEmit(false);
	}
}

//==================================================================================
// 特定のエミッターを有効/無効化
//==================================================================================
void TakeC::EffectGroup::SetEmitterActive(const std::string& emitterName, bool isActive) {

	for (auto& instance : emitterInstances_) {
		if (instance.config.emitterName == emitterName) {
			instance.isActive = isActive;
			if (!isActive) {
				instance.emitter->SetIsEmit(false);
			}
			break;
		}
	}
}

//==================================================================================
// 特定のエミッターの遅延時間を設定
//==================================================================================
void TakeC::EffectGroup::SetEmitterDelay(const std::string& emitterName, float delay) {

	for (auto& instance : emitterInstances_) {
		if (instance.config.emitterName == emitterName) {
			instance.config.delayTime = delay;
			break;
		}
	}
}

//==================================================================================
// エミッター個別の位置オフセットを設定
//==================================================================================
void TakeC::EffectGroup::SetEmitterOffset(const std::string& emitterName, const Vector3& offset) {

	for (auto& instance : emitterInstances_) {
		if (instance.config.emitterName == emitterName) {
			instance.config.positionOffset = offset;
			break;
		}
	}
}

//==================================================================================
// エミッター個別の回転オフセットを設定
//==================================================================================
void TakeC::EffectGroup::SetEmitterRotation(const std::string& emitterName, const Vector3& rotation) {

	for (auto& instance : emitterInstances_) {
		if (instance.config.emitterName == emitterName) {
			instance.config.rotationOffset = rotation;
			break;
		}
	}
}

//==================================================================================
// 位置設定
//==================================================================================
void EffectGroup::SetPosition(const Vector3& position) {
	transform_.translate = position;
}

void EffectGroup::SetRotation(const Vector3& rotation) {
	transform_.rotate = rotation;
}

void EffectGroup::SetScale(const Vector3& scale) {
	transform_.scale = scale;
}

void EffectGroup::SetDirection(const Vector3& direction) {
	direction_ = Vector3Math::Normalize(direction);

	// 方向を全エミッターに適用
	for (auto& instance : emitterInstances_) {
		instance.emitter->SetEmitDirection(direction_);
	}
}

//==================================================================================
// ImGui更新処理
//==================================================================================
void EffectGroup::UpdateImGui() {
#ifdef _DEBUG
	ImGui::Begin(config_.effectName.c_str());

	ImGui::Text("Playing: %s", isPlaying_ ? "Yes" : "No");
	ImGui::Text("Finished: %s", isFinished_ ? "Yes" : "No");
	ImGui::Text("Elapsed Time: %.2f", totalElapsedTime_);

	ImGui::Separator();
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);

	ImGui::Separator();
	if (ImGui::Button("Play")) {
		Play(transform_.translate);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		Stop();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		Reset();
	}

	ImGui::Separator();
	ImGui::Text("Emitters:");
	for (size_t i = 0; i < emitterInstances_.size(); ++i) {
		auto& instance = emitterInstances_[i];
		if (ImGui::TreeNode((void*)(intptr_t)i, "%s", instance.config.emitterName.c_str())) {
			ImGui::Text("Active: %s", instance.isActive ? "Yes" : "No");
			ImGui::Text("Elapsed: %.2f", instance.elapsedTime);
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}