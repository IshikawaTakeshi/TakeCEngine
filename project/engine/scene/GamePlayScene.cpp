#include "GamePlayScene.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include "Vector3Math.h"
#include "ImGuiManager.h"
#include "Collision/CollisionManager.h"
#include <format>
#include <numbers>
//====================================================================
//			初期化
//====================================================================

void GamePlayScene::Initialize() {
	//Camera0
	gameCamera_ = std::make_shared<Camera>();
	gameCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameCamera_->SetIsDebug(false);
	gameCamera_->SetTranslate({ 5.0f,0.0f,-10.0f });
	gameCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("gameCamera", *gameCamera_);

	//Camera1
	debugCamera_ = std::make_shared<Camera>();
	debugCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	debugCamera_->SetTranslate({ 5.0f,0.0f,-1.0f });
	debugCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	debugCamera_->SetIsDebug(true);
	CameraManager::GetInstance()->AddCamera("debugCamera", *debugCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	//Model読み込み
	ModelManager::GetInstance()->LoadModel("gltf","walk.gltf");
	ModelManager::GetInstance()->LoadModel("gltf", "player_animation.gltf");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
#pragma region CreateParticle
	//CreateParticle
	ParticleAttributes particleAttributes;
	particleAttributes.scale = { 0.2f,0.2f,1.0f };
	particleAttributes.positionRange = { -0.5f,0.5f };
	particleAttributes.scaleRange = { 0.1f,3.0f };
	particleAttributes.rotateRange = { -1.0f,1.0f };
	particleAttributes.velocityRange = { 0.0f,0.0f };
	particleAttributes.colorRange = { 0.0f,1.0f };
	particleAttributes.lifetimeRange = { 0.1f,0.3f };
	particleAttributes.editColor = true;
	particleAttributes.color = { 0.8f,0.8f,1.0f };
	particleAttributes.isBillboard = true;
	particleAttributes.scaleSetting_ = true;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"SmokeEffect",ParticleModelType::Primitive, "Spark.png",PRIMITIVE_PLANE);
	TakeCFrameWork::GetParticleManager()->SetAttributes("SmokeEffect", particleAttributes);

	ParticleAttributes particleAttributes2;
	particleAttributes2.scale = { 100.0f,3.0f,1.0f };
	particleAttributes2.positionRange = { 0.0f,0.0f };
	particleAttributes2.scaleRange = { 20.0f,100.0f };
	particleAttributes2.rotateRange = { 0.0f,0.0f };
	particleAttributes2.velocityRange = { 0.0f,0.0f };
	particleAttributes2.colorRange = { 0.0f,1.0f };
	particleAttributes2.lifetimeRange = { 0.1f,0.5f };
	particleAttributes2.editColor = true;
	particleAttributes2.color = { 0.5f,0.3f,1.0f };
	particleAttributes2.isBillboard = true;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "CircleEffect", ParticleModelType::Primitive, "Cross.png", PRIMITIVE_PLANE);
	TakeCFrameWork::GetParticleManager()->SetAttributes("CircleEffect", particleAttributes2);

	ParticleAttributes damageEffectAttributes;
	damageEffectAttributes.scale = { 10.0f,10.0f,1.0f };
	damageEffectAttributes.positionRange = { -1.0f,1.0f };
	damageEffectAttributes.scaleRange = { 1.0f,10.0f };
	damageEffectAttributes.rotateRange = { -3.14f,3.14f };
	damageEffectAttributes.velocityRange = { 0.0f,0.0f };
	damageEffectAttributes.lifetimeRange = { 0.05f,0.05f };
	damageEffectAttributes.editColor = true;
	damageEffectAttributes.color = { 0.4f,0.1f,1.0f };
	damageEffectAttributes.isTraslate_ = true;
	damageEffectAttributes.scaleSetting_ = 2;
	damageEffectAttributes.isBillboard = true;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "DamageEffectSpark", ParticleModelType::Primitive, "Spark.png", PRIMITIVE_PLANE);
	TakeCFrameWork::GetParticleManager()->SetAttributes("DamageEffectSpark", damageEffectAttributes);

	ParticleAttributes bulletLightAttributes;
	bulletLightAttributes.scale = { 5.1f,5.1f,0.1f };
	bulletLightAttributes.positionRange = { 0.0f,0.0f };
	bulletLightAttributes.scaleRange = { 0.1f,5.1f };
	bulletLightAttributes.rotateRange = { -3.14f,3.14f };
	bulletLightAttributes.velocityRange = { 0.0f,0.0f };
	bulletLightAttributes.lifetimeRange = { 0.5f,0.8f };
	bulletLightAttributes.editColor = true;
	bulletLightAttributes.color = { 0.8f,0.8f,1.0f };
	bulletLightAttributes.isTraslate_ = true;
	bulletLightAttributes.scaleSetting_ = 2; //縮小せる
	bulletLightAttributes.isBillboard = true;
	//bulletLightAttributes.enableFollowEmitter_ = true;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BulletLight", ParticleModelType::Primitive, "Circle.png", PRIMITIVE_PLANE);
	TakeCFrameWork::GetParticleManager()->SetAttributes("BulletLight", bulletLightAttributes);

	ParticleAttributes ExplosionAttributes;
	ExplosionAttributes.scale = { 1.0f,1.0f,1.0f };
	ExplosionAttributes.positionRange = { 0.0f,0.0f };
	ExplosionAttributes.scaleRange = { 0.1f,23.0f };
	ExplosionAttributes.rotateRange = { -3.14f,3.14f };
	ExplosionAttributes.velocityRange = { 0.0f,0.0f };
	ExplosionAttributes.lifetimeRange = { 0.1f,0.5f };
	ExplosionAttributes.editColor = true;
	ExplosionAttributes.color = { 0.8f,0.8f,1.0f };
	ExplosionAttributes.isTraslate_ = true;
	ExplosionAttributes.scaleSetting_ = 1; //拡大させる
	ExplosionAttributes.isBillboard = false;
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "ExplosionEffect", ParticleModelType::Primitive, "Spark2.png", PRIMITIVE_SPHERE);
	TakeCFrameWork::GetParticleManager()->SetAttributes("ExplosionEffect", ExplosionAttributes);

#pragma endregion

	//Animation読み込み
	TakeCFrameWork::GetAnimator()->LoadAnimation("Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("player_animation.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_pool.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	//sprite
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "rick.png");
	sprite_->SetTextureLeftTop({ 235.0f,40.0f });

	//BulletManager
	bulletManager_ = std::make_unique<BulletManager>();
	bulletManager_->Initialize(Object3dCommon::GetInstance(), 100); //弾の最大数:100

	//player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "player_animation.gltf");
	player_->WeaponInitialize(Object3dCommon::GetInstance(), bulletManager_.get(), "axis.obj");
	player_->GetObject3d()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("player_animation.gltf", "clear"));
	player_->SetTranslate({ 0.0f, 0.0f, -30.0f });

	//Enemy
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(Object3dCommon::GetInstance(), "player_animation.gltf");
	enemy_->WeaponInitialize(Object3dCommon::GetInstance(), bulletManager_.get(), "cube.obj");
	enemy_->GetObject3d()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("player_animation.gltf", "clear"));

	//cubeObject
	cubeObject_ = std::make_unique<Object3d>();
	cubeObject_->Initialize(Object3dCommon::GetInstance(), "cube.obj");
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	CollisionManager::GetInstance()->ClearGameCharacter(); // 当たり判定の解放
	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	player_.reset();
	sprite_.reset();
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void GamePlayScene::Update() {

	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();
	
	//particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();

	sprite_->Update();

	//player
	player_->SetFocusTargetPos(enemy_->GetObject3d()->GetTranslate());
	player_->Update();
	//enemy
	enemy_->Update();

	//cubeObject
	cubeObject_->Update();

	//弾の更新
	bulletManager_->UpdateBullet();

	//当たり判定の更新
	CheckAllCollisions();

  //パーティクルの更新
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
		// シーン切り替え依頼
		//AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	} else if (Input::GetInstance()->TriggerKey(DIK_O)) {
		//AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
}

void GamePlayScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();
	ParticleCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

	cubeObject_->UpdateImGui("cube");
	player_->UpdateImGui();
	enemy_->UpdateImGui();
	sprite_->UpdateImGui("gameScene");
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw();    //天球の描画

#pragma region Object3d描画

	//Object3dの描画前処理
	Object3dCommon::GetInstance()->DisPatch();
	player_->GetObject3d()->DisPatch();
	enemy_->GetObject3d()->DisPatch();
	Object3dCommon::GetInstance()->PreDraw();
	player_->Draw();
	enemy_->Draw();
	cubeObject_->Draw();
	bulletManager_->DrawBullet();

#pragma endregion

	//当たり判定の描画前処理
	CollisionManager::GetInstance()->PreDraw();
	player_->DrawCollider();
	enemy_->DrawCollider();
	bulletManager_->DrawCollider();

	//グリッド地面の描画
	TakeCFrameWork::GetWireFrame()->DrawGridGround({ 0.0f,0.0f,0.0f }, { 1000.0f, 1000.0f, 1000.0f }, 50);
	TakeCFrameWork::GetWireFrame()->DrawGridBox({
		{-500.0f,-500.0f,-500.0f},{500.0f,500.0f,500.0f } }, 2);
	TakeCFrameWork::GetWireFrame()->Draw();

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画


#pragma region スプライト描画
	//スプライトの描画前処理
	SpriteCommon::GetInstance()->PreDraw();
	//sprite_->Draw();    //スプライトの描画
#pragma endregion

	//GPUパーティクルの描画
	//ParticleCommon::GetInstance()->PreDrawForGPUParticle();

}

void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearGameCharacter();

	const std::vector<Bullet*>& bullets = bulletManager_->GetAllBullets();

	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(player_.get()));

	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(enemy_.get()));

	for (const auto& bullet : bullets) {
		if (bullet->GetIsActive()) {
			CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(bullet));
		}
	}


	CollisionManager::GetInstance()->CheckAllCollisionsForGameCharacter();
}