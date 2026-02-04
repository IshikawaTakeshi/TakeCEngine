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

//==================================================================================
// 共通初期化処理
//==================================================================================
void EffectGroup::InitializeCommon() {
	// デルタタイム取得
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	// デフォルトスケール適用
	scale_ = config_.defaultScale;

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

	// 総経過時間を更新
	totalElapsedTime_ += deltaTime_;

	// ジョイント追従の更新
	if (attachedSkeleton_ != nullptr) {
		UpdateJointAttachment();
	}

	// 各エミッターを更新
	bool anyEmitterActive = false;

	for (auto& instance : emitterInstances_) {
		// 経過時間を更新
		instance.elapsedTime += deltaTime_;

		// 開始判定
		if (!instance.hasStarted) {
			CheckEmitterStart(instance);
		}

		// アクティブなら更新
		if (instance.isActive) {
			anyEmitterActive = true;

			// 位置更新
			UpdateEmitterPositions();

			// エミッター更新
			instance.emitter->Update();

			// 終了判定
			CheckEmitterEnd(instance);
		}
	}

	// エフェクト全体の終了判定
	if (!anyEmitterActive && !config_.isLooping) {
		isFinished_ = true;
		isPlaying_ = false;
	}

	// ループ再生
	if (isFinished_ && config_.isLooping) {
		Reset();
		Play(position_);
	}
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
		offsetPos.x *= scale_.x;
		offsetPos.y *= scale_.y;
		offsetPos.z *= scale_.z;

		// 回転を適用（quaternion使用）
		Quaternion rot = QuaternionMath::FromEuler(rotation_);
		offsetPos = QuaternionMath::RotateVector(offsetPos, rot);

		// 最終位置を設定
		Vector3 finalPosition = position_ + offsetPos;
		instance.emitter->SetTranslate(finalPosition);

		// 回転も適用
		Vector3 finalRotation = rotation_ + instance.config.rotationOffset;
		instance.emitter->SetRotate(finalRotation);
	}
}

//==================================================================================
// ジョイント追従の更新
//==================================================================================
void EffectGroup::UpdateJointAttachment() {
	if (attachedSkeleton_ != nullptr && !attachedJointName_.empty()) {
		// ジョイント位置を取得
		std::optional<Vector3> jointPos = 
			attachedSkeleton_->GetJointPosition(attachedJointName_, MatrixMath::MakeIdentity4x4());

		if (jointPos.has_value()) {
			position_ = jointPos.value();
		}
	}
}

//==================================================================================
// エフェクトを再生
//==================================================================================
void EffectGroup::Play(const Vector3& position) {
	Reset();
	position_ = position;
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
		instance.emitter->SetIsEmit(false);
	}
}

//==================================================================================
// ジョイントにアタッチ
//==================================================================================
void EffectGroup::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	attachedSkeleton_ = skeleton;
	attachedJointName_ = jointName;
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
	position_ = position;
}

void EffectGroup::SetRotation(const Vector3& rotation) {
	rotation_ = rotation;
}

void EffectGroup::SetScale(const Vector3& scale) {
	scale_ = scale;
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
	ImGui::DragFloat3("Position", &position_.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation_.x, 0.01f);
	ImGui::DragFloat3("Scale", &scale_.x, 0.01f);

	ImGui::Separator();
	if (ImGui::Button("Play")) {
		Play(position_);
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