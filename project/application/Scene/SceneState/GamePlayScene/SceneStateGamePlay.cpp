#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

using namespace TakeC;

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

	playerHpBar->SetHP(player->GetHealth(), player->GetMaxHealth());
	playerHpBar->Update();

	energyUI->SetOverHeatState(player->GetIsOverHeated());
	energyUI->Update(player->GetEnergy(), player->GetMaxEnergy());

	// ================================
	// Enemy UI
	// ================================

	enemyHpBar->SetHP(enemy->GetHealth(), enemy->GetMaxHealth());
	enemyHpBar->Update();

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

	if(Input::GetInstance().TriggerButton(0,GamepadButtonType::Start)) {
		RequestTransition(SceneState::PAUSE);
	}
}
