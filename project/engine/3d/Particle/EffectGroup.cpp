#include "EffectGroup.h"
#include "Math/MatrixMath.h"
#include "Math/Quaternion.h"
#include "Math/Vector3Math.h"
#include "engine/base/TakeCFrameWork.h"
#include <cassert>


using namespace TakeC;

//==================================================================================
// JSONファイルから初期化
//==================================================================================
void EffectGroup::Initialize(const std::string& configFilePath) {
	// JSONから設定を読み込み
	config_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<EffectGroupConfig>(
		configFilePath);

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
	transform_.rotate = { 0.0f, 0.0f, 0.0f, 1.0f }; // 単位クォータニオン

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
			instance.emitter->Initialize(emitterConfig.emitterName,
				"DefaultParticlePreset.json");
		} else {
			// 指定されたプリセットファイルパスを使用
			instance.emitter->Initialize(emitterConfig.emitterName,
				emitterConfig.presetFilePath);
		}
		instance.emitter->SetIsEmit(false); // 最初は停止

		emitterInstances_.push_back(std::move(instance));
	}
}

//==================================================================================
// 更新処理
//==================================================================================
void EffectGroup::Update() {

	totalElapsedTime_ += deltaTime_;

	// -------------------------------------------------------------
	// 1. 基本となる位置と回転を決定
	// -------------------------------------------------------------
	Vector3 currentPos = transform_.translate;
	Quaternion currentRot = transform_.rotate;

	// 親行列がある場合、親の位置と回転を反映させる
	if (parentMatrix_) {
		// A. 位置の同期（親のワールド座標）
		currentPos.x = parentMatrix_->m[3][0];
		currentPos.y = parentMatrix_->m[3][1];
		currentPos.z = parentMatrix_->m[3][2];

		// B. 回転の同期（親の回転 × 自分の回転）
		// 行列から回転成分を抽出してクォータニオン化
		Matrix4x4 rotationMatrix = *parentMatrix_;
		rotationMatrix.m[3][0] = 0.0f;
		rotationMatrix.m[3][1] = 0.0f;
		rotationMatrix.m[3][2] = 0.0f;

		// スケールを除去（各軸を正規化）
		Vector3 up = { rotationMatrix.m[1][0], rotationMatrix.m[1][1],
			rotationMatrix.m[1][2] };
		Vector3 fwd = { rotationMatrix.m[2][0], rotationMatrix.m[2][1],
			rotationMatrix.m[2][2] };
		up = Vector3Math::Normalize(up);
		fwd = Vector3Math::Normalize(fwd);

		Quaternion parentRot = QuaternionMath::LookRotation(fwd, up);
		parentRot = QuaternionMath::Normalize(parentRot);

		// 親の回転 × 自分の回転
		currentRot = QuaternionMath::Multiply(parentRot, currentRot);
	}

	bool anyEmitterActive = false;

	// -------------------------------------------------------------
	// 2. 各エミッターの更新
	// -------------------------------------------------------------
	for (auto& instance : emitterInstances_) {
		instance.elapsedTime += deltaTime_;

		if (!instance.hasStarted) {
			CheckEmitterStart(instance);
		}

		if (instance.isActive) {
			anyEmitterActive = true;

			// エミッターのローカルオフセットを取り出す
			Vector3 offset = instance.config.positionOffset;

			// 合成した回転(親×自分)でオフセットを回転させる
			offset = QuaternionMath::RotateVector(offset, currentRot);

			// 最終的なワールド座標
			Vector3 finalPos = currentPos + offset;
			instance.emitter->SetTranslate(finalPos);

			// エミッターの回転設定
			//  エミッターオフセット回転も加味する
			//  instance.config.rotationOffset
			//  はオイラー角(Vector3)のままなので変換が必要
			Quaternion offsetRotQ =
				QuaternionMath::FromEuler(instance.config.rotationOffset);
			Quaternion finalRotQ = QuaternionMath::Multiply(currentRot, offsetRotQ);
			finalRotQ = QuaternionMath::Normalize(finalRotQ);
			// Quaternionからオイラー角に戻して設定
			// (ParticleEmitterがオイラー角を受け取る場合)
			Vector3 finalRotEuler = QuaternionMath::ToEuler(finalRotQ);
			instance.emitter->SetRotate(finalRotEuler);

			instance.emitter->Update();
			CheckEmitterEnd(instance);
		} else {
			// エミッターのローカルオフセットを取り出す
			Vector3 offset = instance.config.positionOffset;

			// 合成した回転(親×自分)でオフセットを回転させる
			offset = QuaternionMath::RotateVector(offset, currentRot);

			Vector3 finalPos = currentPos + offset;
			instance.emitter->SetTranslate(finalPos);
		}
	}

	// エフェクト全体の終了判定
	if (!anyEmitterActive && !config_.isLooping) {
		isFinished_ = true;
		isPlaying_ = false;
	}

	// ループ再生（Stop()で明示的に止められた場合はループしない）
	if (!anyEmitterActive && config_.isLooping && !isLoopingSuspended_) {
		Reset(); // 1回
		transform_.translate = currentPos;
		isPlaying_ = true;
		isFinished_ = false;
		isPaused_ = false;
	}
}
//==================================================================================
// 実行中にエミッターの設定を更新する
//==================================================================================
void TakeC::EffectGroup::UpdateEmitterConfig(int index,
	const EmitterConfig& config) {

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
	Quaternion rot = transform_.rotate;
	offsetPos = QuaternionMath::RotateVector(offsetPos, rot);

	// 最終位置を設定
	Vector3 finalPosition = transform_.translate + offsetPos;
	instance.emitter->SetTranslate(finalPosition);

	// 回転も適用
	Quaternion offsetRotQ =
		QuaternionMath::FromEuler(instance.config.rotationOffset);
	Vector3 finalRotation =
		QuaternionMath::ToEuler(transform_.rotate * offsetRotQ);
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
		if (instance.elapsedTime >=
			instance.config.delayTime + instance.config.duration) {
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

		// 回転を適用（quaternion使用）
		Quaternion rot = transform_.rotate;
		offsetPos = QuaternionMath::RotateVector(offsetPos, rot);

		// 最終位置を設定
		Vector3 finalPosition = transform_.translate + offsetPos;
		instance.emitter->SetTranslate(finalPosition);

		// 回転も適用
		Quaternion offsetRotQ =
			QuaternionMath::FromEuler(instance.config.rotationOffset);
		Vector3 finalRotation =
			QuaternionMath::ToEuler(transform_.rotate * offsetRotQ);
		instance.emitter->SetRotate(finalRotation);
	}
}

//==================================================================================
// エフェクトを再生
//==================================================================================
void EffectGroup::Play(const Vector3& position) {
	isLoopingSuspended_ = false; // Stop()で止めていた場合はループを再開
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
	isLoopingSuspended_ = true; // ループ再生を抑制する

	// 全エミッターを停止
	for (auto& instance : emitterInstances_) {
		instance.emitter->SetIsEmit(false);
		instance.isActive = false;
		instance.hasStarted = false;
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
void TakeC::EffectGroup::SetEmitterActive(const std::string& emitterName,
	bool isActive) {

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
void TakeC::EffectGroup::SetEmitterDelay(const std::string& emitterName,
	float delay) {

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
void TakeC::EffectGroup::SetEmitterOffset(const std::string& emitterName,
	const Vector3& offset) {

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
void TakeC::EffectGroup::SetEmitterRotation(const std::string& emitterName,
	const Vector3& rotation) {

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
	UpdateEmitterPositions();
}

void EffectGroup::SetScale(const Vector3& scale) {
	transform_.scale = scale;
	UpdateEmitterPositions();
}

void EffectGroup::SetRotate(const Quaternion& rotation) {
	transform_.rotate = rotation;
	UpdateEmitterPositions();
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
void EffectGroup::UpdateImGui([[maybe_unused]] const std::string& windowName) {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::SeparatorText((windowName + config_.effectName).c_str());

	ImGui::Text("Playing: %s", isPlaying_ ? "Yes" : "No");
	ImGui::Text("Finished: %s", isFinished_ ? "Yes" : "No");
	ImGui::Text("Elapsed Time: %.2f", totalElapsedTime_);

	ImGui::Separator();
	transform_.EditConfig("Effect Transform");

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
	// エミッター情報の表示
	ImGui::Text("Emitters:");
	for (size_t i = 0; i < emitterInstances_.size(); ++i) {
		auto& instance = emitterInstances_[i];
		if (ImGui::TreeNode((void*)(intptr_t)i, "%s",
			instance.config.emitterName.c_str())) {
			ImGui::Text("Active: %s", instance.isActive ? "Yes" : "No");
			ImGui::Text("Elapsed: %.2f", instance.elapsedTime);
			ImGui::DragFloat3("Position Offset", &instance.config.positionOffset.x,
				0.1f);
			ImGui::DragFloat3("Rotation Offset", &instance.config.rotationOffset.x,
				0.1f);
			ImGui::TreePop();
		}
	}
	// 設定保存
	if (ImGui::Button("Save Config")) {
		TakeCFrameWork::GetJsonLoader()->SaveJsonData<EffectGroupConfig>(
			config_.effectName, config_);
	}
#endif
}