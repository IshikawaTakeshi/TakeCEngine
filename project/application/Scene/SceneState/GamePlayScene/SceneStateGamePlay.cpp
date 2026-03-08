#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

//===================================================================================
// 初期化
//===================================================================================
void SceneStateGamePlay::Initialize(GamePlayScene* scene) {

	// フェーズメッセージUIにFIGHTメッセージをセット
	scene->GetPhaseMessageUI()->SetNextMessage(PhaseMessage::FIGHT);

	TakeC::CameraManager::GetInstance().GetActiveCamera()->RequestCameraState(
		Camera::GameCameraState::LOCKON);

	scene->GetPlayer()->SetInCombat(true);
	scene->GetEnemy()->SetInCombat(true);
}

//===================================================================================
// 更新
//===================================================================================
void SceneStateGamePlay::Update(GamePlayScene* scene) {
	auto* player = scene->GetPlayer();
	auto* enemy = scene->GetEnemy();

	auto* playerReticle = scene->GetPlayerReticle();
	auto* playerHpBar = scene->GetPlayerHpBar();
	auto* enemyHpBar = scene->GetEnemyHpBar();
	auto* energyUI = scene->GetEnergyInfoUI();

	// ================================
	// Player UI
	// ================================

	playerReticle->SetIsFocus(player->IsFocus());
	playerReticle->Update(player->GetFocusTargetPos(), enemy->GetBodyPosition());

	playerHpBar->Update(player->GetHealth(), player->GetMaxHealth());

	energyUI->SetOverHeatState(player->GetIsOverHeated());
	energyUI->Update(player->GetEnergy(), player->GetMaxEnergy());

	// ================================
	// Enemy UI
	// ================================

	enemyHpBar->Update(enemy->GetHealth(), enemy->GetMaxHealth());

	// ================================
	// Weapon UI
	// ================================

	for (int i = 0; i < 4; ++i) {
		auto* weapon = player->GetCurrentWeapon(i);
		auto* bulletUI = scene->GetBulletCounterUI(i);

		bulletUI->SetBulletCount(weapon->GetBulletCount());
		bulletUI->SetRemainingBulletCount(weapon->GetRemainingBulletCount());
		bulletUI->SetReloadingState(weapon->GetIsReloading());
		bulletUI->SetWeaponIconUV(static_cast<int>(weapon->GetUnitPosition()));
		bulletUI->Update();
	}

	// ================================
	// Gauge UI Update
	// ================================
	auto* bulletGaugeUI = scene->GetBulletCounterGaugeUI();
	if (bulletGaugeUI) {
		// 現在の武器（一応0番目）の状態を反映
		auto* activeWeapon = player->GetCurrentWeapon(1);
		if (activeWeapon) {
			if (activeWeapon->GetIsReloading()) {
				// リロード中：赤色で進捗を表示
				bulletGaugeUI->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 赤
				float maxReload = activeWeapon->GetMaxReloadTime();
				float currentReload = activeWeapon->GetCurrentReloadTime();
				float ratio = (maxReload > 0.0f) ? (currentReload / maxReload) : 1.0f;
				bulletGaugeUI->SetRatio(ratio);
			} else {
				// 通常時：白色で残弾比率を表示
				bulletGaugeUI->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 白
				float ratio = 0.0f;
				if (activeWeapon->GetMagazineCount() > 0) {
					ratio = static_cast<float>(activeWeapon->GetBulletCount()) /
						activeWeapon->GetMagazineCount();
				}
				bulletGaugeUI->SetRatio(ratio);
			}
		}

		// レティクルの位置に追従
		Vector2 reticlePos = scene->GetPlayerReticle()->GetScreenPosition();
		bulletGaugeUI->SetPosition(
			{ reticlePos.x, reticlePos.y + 40.0f }); // レティクルの少し下に配置
		bulletGaugeUI->Update();
	}

	// ================================
	// Manager Update
	// ================================

	TakeCFrameWork::GetUIManager()->Update();
	TakeCFrameWork::GetSpriteManager()->Update();

	// ================================
	// Scene Transition
	// ================================

	if (player->GetHealth() <= 0.0f) {
		RequestTransition(SceneState::GAMEOVER);
	} else if (enemy->GetHealth() <= 0.0f) {
		RequestTransition(SceneState::ENEMYDESTROYED);
	}
}
