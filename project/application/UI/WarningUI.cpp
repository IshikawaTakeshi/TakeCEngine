#include "WarningUI.h"
#include "engine/Camera/CameraManager.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/MathEnv.h"
#include "engine/Math/Vector3Math.h"
#include "engine/Base/SpriteManager.h"
#include "engine/Base/TakeCFramework.h"

using namespace TakeC;

//======================================================================
//	初期化
//======================================================================

void WarningUI::Initialize(TakeC::SpriteManager* spriteManager, const std::string& configName) {

    // 親クラスの初期化
    BaseUI::Initialize(spriteManager);

    // カメラの取得（メインカメラ）
    targetCamera_ = TakeC::CameraManager::GetInstance().GetActiveCamera();

    // 4方向分のスプライト生成と登録
    const std::string suffixes[] = { "_front","_right","_back","_left"};

    for (int i = 0; i < static_cast<int>(WarningDirection::COUNT); ++i) {
		std::string fullName = configName + suffixes[i] + ".json";

        // BaseUIの機能を使ってスプライトを生成・登録
        Sprite* newSprite = CreateAndRegisterSpriteFromJson(fullName);

        // 生成されたスプライトへのポインタを各方向の状態管理構造体に保持
        directionStates_[i].spritePtr = newSprite;

		// 初期状態は非表示(alpha=0)にする
        if (newSprite) {
			newSprite->SetAlpha(0.0f);
        }

        // タイマー初期化
        directionStates_[i].timer.Initialize(kFlashDuration_, 0.0f);
    }

    // イベントオブザーバー登録
    // std::any で Vector3 (敵のワールド座標) が送られてくると想定
    TakeCFrameWork::GetEventManager()->AddObserver("EnemyHighPowerAttack", [this](const std::any& data) {

        // データ型チェック（Vector3か？）
        if (const Vector3* enemyPosPtr = std::any_cast<Vector3>(&data)) {
            // 値のコピーを取得
            Vector3 enemyPos = *enemyPosPtr;

            // 方向を計算
            WarningDirection dir = this->CalculateDirection(enemyPos);

            // 警告開始
            this->TriggerWarning(dir);
        }
        });
}

//======================================================================
//	更新処理
//======================================================================
void WarningUI::Update() {

    targetCamera_ = TakeC::CameraManager::GetInstance().GetActiveCamera();

    // 全方向の状態更新
    for (int i = 0; i < static_cast<int>(WarningDirection::COUNT); ++i) {
        auto& state = directionStates_[i];

        if (!state.isFlashing) {
            // 点滅していない場合は非表示
            if (state.spritePtr) {
                state.spritePtr->SetAlpha(0.0f); // アルファ0にする
            }
            continue;
        }

        // 時間経過
		state.timer.Update();

        // 点滅ロジック
        float alpha = std::abs(std::sin(state.timer.GetProgress() * kFlashSpeed_));

        // スプライトにアルファ値を適用
        if (state.spritePtr) {
			state.spritePtr->SetAlpha(alpha);
        }

        // 一定時間経過したら終了
        if (state.timer.IsFinished()) {
            state.isFlashing = false;
			state.timer.Reset();
            if (state.spritePtr) {
				state.spritePtr->SetAlpha(0.0f); // 完全に非表示にする
            }
        }
    }
}

//======================================================================
//	ImGuiの更新
//======================================================================
void WarningUI::UpdateImGui(const std::string& name) {
    BaseUI::UpdateImGui(name);

#ifdef _DEBUG
    if (ImGui::TreeNode("WarningUI Debug")) {
        const char* dirNames[] = { "Front", "Right","Back", "Left" };
        for (int i = 0; i < static_cast<int>(WarningDirection::COUNT); ++i) {
            if (ImGui::Button(dirNames[i])) {
                TriggerWarning(static_cast<WarningDirection>(i));
            }
        }
        ImGui::TreePop();
    }
#endif
}

//======================================================================
//	方向計算
//======================================================================
WarningDirection WarningUI::CalculateDirection(const Vector3& targetPos) {
    if (!targetCamera_) return WarningDirection::FRONT;

    // 1. カメラとターゲットの情報を取得
    Vector3 followTargetPos = targetCamera_->GetFollowTargetPosition();
    // カメラの回転情報から、前方ベクトルと右方向ベクトルを計算
    // ※カメラの実装に合わせて取得方法は調整してください（GetWorldMatrixから取得するのが確実です）
    Matrix4x4 cameraWorld = targetCamera_->GetWorldMatrix();
    Vector3 cameraForward = { cameraWorld.m[2][0], 0.0f, cameraWorld.m[2][2] }; // Z軸 (Y成分無視)
    Vector3 cameraRight   = { cameraWorld.m[0][0], 0.0f, cameraWorld.m[0][2] }; // X軸 (Y成分無視)

    // 正規化 (長さ0対策含む)
    if (cameraForward.Length() < kEpsilon) cameraForward = { 0.0f, 0.0f, 1.0f };
    else cameraForward = Vector3Math::Normalize(cameraForward);

    if (cameraRight.Length() < kEpsilon) cameraRight = { 1.0f, 0.0f, 0.0f };
    else cameraRight = Vector3Math::Normalize(cameraRight);


    // 2. カメラからターゲットへのベクトル (Y成分無視して水平面で計算)
    Vector3 toTarget = targetPos - followTargetPos;
    toTarget.y = 0.0f; 

    // ターゲットがほぼ同じ位置にいる場合はFRONTとする
    if (toTarget.Length() < kEpsilon) return WarningDirection::FRONT;
    toTarget = Vector3Math::Normalize(toTarget);


    // 3. 角度計算
    // カメラの前方ベクトルを基準(0度)として、ターゲットが何度ずれているか計算する
    // 内積 (Dot) と 外積 (CrossのY成分) を利用して atan2 で角度(-PI ~ PI)を出す

    // Dot(F, T) = |F||T|cosθ = cosθ
    float dot = Vector3Math::Dot(cameraForward, toTarget);
    // Cross(F, T).y = |F||T|sinθ = sinθ
    Vector3 cross = Vector3Math::Cross(cameraForward, toTarget);
    float det = cross.y; // Y軸回りの回転成分

    // atan2(y, x) -> atan2(sin, cos)
    // これで -PI(左/後) ～ 0(前) ～ PI(右/後) の値が取れる
    // ※座標系によりますが、通常 右手系Y-upなら Cross(Forward, Target).y > 0 が左側、< 0 が右側になることが多い
    //   atan2(det, dot) の場合:
    //   dot=1 (前) -> 0
    //   det>0 (左) -> プラス
    //   det<0 (右) -> マイナス
    float angleRad = std::atan2(det, dot);

    // MathEnvの関数で Degree に変換
    float angleDeg = radianToDegree(angleRad);

    // 4. WarningDirection にマッピング
    // angleDeg は -180 ~ 180 の範囲
    // 0度 = FRONT
    // 90度 = LEFT (atan2のdetの符号による。左手座標系か右手座標系かで逆になる可能性あり)
    // 180/-180度 = BACK
    // -90度 = RIGHT

    // 0~360度 に正規化 (0=Front, 90=Left, 180=Back, 270=Right と仮定)
    if (angleDeg < 0.0f) angleDeg += 360.0f;

    // 現在の angleDeg の並び（推定）:
    // Front(0) -> Left(90) -> Back(180) -> Right(270)  [反時計回り]

    // WarningDirection の並び（時計回り）に合わせる:
    // FRONT(0), RIGHT(1), BACK(2), LEFT(3)
    float clockwiseAngle = angleDeg;
    if (clockwiseAngle >= 360.0f) clockwiseAngle -= 360.0f;

    // 4分割用の定数
    constexpr float kAngleStep   = 90.0f; // 1区画
    constexpr float kAngleOffset = 45.0f; // 半分ずらす

    // FRONT(0) が 315~45 (-45~45) の範囲になるようオフセットを加算
    float calcAngle = clockwiseAngle + kAngleOffset;
    if (calcAngle >= 360.0f) calcAngle -= 360.0f;

    int index = static_cast<int>(calcAngle / kAngleStep);

    // 範囲チェック
    if (index < 0) index = 0;
    if (index >= static_cast<int>(WarningDirection::COUNT)) index = 0;

    return static_cast<WarningDirection>(index);
}

//======================================================================
//	警告開始
//======================================================================
void WarningUI::TriggerWarning(WarningDirection dir) {

    int idx = static_cast<int>(dir);
    auto& state = directionStates_[idx];

    // 既に点滅中なら時間をリセット（延長）するだけにする
	state.timer.Reset();
    state.isFlashing = true;
}
