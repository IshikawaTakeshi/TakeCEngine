#include "SceneStateGamePlay.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"


//===================================================================================
// 初期化
//===================================================================================
void SceneStateGamePlay::Initialize(GamePlayScene *scene) {

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
void SceneStateGamePlay::Update(GamePlayScene *scene) {

  // playerReticleの更新
  scene->GetPlayerReticle()->SetIsFocus(scene->GetPlayer()->IsFocus());
  scene->GetPlayerReticle()->Update(scene->GetPlayer()->GetFocusTargetPos(),
                                    scene->GetEnemy()->GetBodyPosition());

  // playerのHPバーの更新
  scene->GetPlayerHpBar()->Update(scene->GetPlayer()->GetHealth(),
                                  scene->GetPlayer()->GetMaxHealth());
  // enemyのHPバーの更新
  scene->GetEnemyHpBar()->Update(scene->GetEnemy()->GetHealth(),
                                 scene->GetEnemy()->GetMaxHealth());

  // playerのエネルギーUIの更新
  scene->GetEnergyInfoUI()->SetOverHeatState(
      scene->GetPlayer()->GetIsOverHeated());
  scene->GetEnergyInfoUI()->Update(scene->GetPlayer()->GetEnergy(),
                                   scene->GetPlayer()->GetMaxEnergy());
  // bulletCounterUIの更新
  for (int i = 0; i < 4; i++) {
    scene->GetBulletCounterUI(i)->SetBulletCount(
        scene->GetPlayer()->GetCurrentWeapon(i)->GetBulletCount());
    scene->GetBulletCounterUI(i)->SetRemainingBulletCount(
        scene->GetPlayer()->GetCurrentWeapon(i)->GetRemainingBulletCount());
    scene->GetBulletCounterUI(i)->SetReloadingState(
        scene->GetPlayer()->GetCurrentWeapon(i)->GetIsReloading());
    scene->GetBulletCounterUI(i)->SetWeaponIconUV(static_cast<int>(
        scene->GetPlayer()->GetCurrentWeapon(i)->GetUnitPosition()));
    scene->GetBulletCounterUI(i)->Update();
  }

  // UIManagerの更新
  TakeCFrameWork::GetUIManager()->Update();
  // SpriteManagerの更新
  TakeCFrameWork::GetSpriteManager()->Update();

  if (scene->GetPlayer()->GetHealth() <= 0.0f) {
    // プレイヤーのHPが0以下になったらゲームオーバー
    RequestTransition(SceneState::GAMEOVER);

  } else if (scene->GetEnemy()->GetHealth() <= 0.0f) {
    // エネミーのHPが0以下になったらゲームクリア
    RequestTransition(SceneState::ENEMYDESTROYED);
  }
}
