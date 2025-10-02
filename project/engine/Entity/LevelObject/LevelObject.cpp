#include "LevelObject.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "engine/base/imGuiManager.h"
#include "engine/base/TakeCFrameWork.h"

void LevelObject::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	object3dCommon_ = object3dCommon;
	// Object3dの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon_, filePath);
	// キャラクタータイプの設定
	characterType_ = CharacterType::LEVEL_OBJECT;

	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void LevelObject::CollisionInitialize(const LevelData::BoxCollider& boxInfo) {
	collider_ = std::make_unique<BoxCollider>();
	collider_->SetHalfSize(boxInfo.size);
	collider_->Initialize(object3dCommon_->GetDirectXCommon(), object3d_.get());
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Level_Object)); // レベルオブジェクトの衝突レイヤーを設定
	//水色に設定
	collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 水色
}

void LevelObject::CollisionInitialize(const LevelData::SphereCollider& sphereInfo) {
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetRadius(sphereInfo.radius);
	collider_->Initialize(object3dCommon_->GetDirectXCommon(), object3d_.get());
	//水色に設定
	collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 水色
}

void LevelObject::Update() {

	if(isBlinking_) {
		// 点滅タイマーの更新
		blinkTimer_ += deltaTime_;
		if (blinkTimer_ >= kBlinkDuration) {
			isBlinking_ = false; // 点滅終了
			blinkTimer_ = 0.0f; // タイマーリセット
			collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 元の色に戻す
		} else {
			// 点滅中は色を変える
			if (static_cast<int>(blinkTimer_ / kBlinkInterval) % 2 == 0) {
				collider_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色
			} else {
				collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 水色
			}
		}
	}

	// Object3dの更新
	object3d_->Update();
	// コライダーの更新
	collider_->Update(object3d_.get());
	
}

void LevelObject::UpdateImGui() {
	// ImGuiの更新
	std::string windowName = "LevelObject" + name_;
	if (ImGui::TreeNode(windowName.c_str())) {
		ImGui::Text("Scale: %.2f, %.2f, %.2f", object3d_->GetScale().x, object3d_->GetScale().y, object3d_->GetScale().z);
		ImGui::Text("Rotate: %.2f, %.2f, %.2f", object3d_->GetRotate().x, object3d_->GetRotate().y, object3d_->GetRotate().z);
		ImGui::Text("Translate: %.2f, %.2f, %.2f", object3d_->GetTranslate().x, object3d_->GetTranslate().y, object3d_->GetTranslate().z);
		ImGui::SeparatorText("Collider Settings");
		if (collider_) {
			collider_->UpdateImGui(name_);
		} else {
			ImGui::Text("Collider not initialized.");
		}
		ImGui::TreePop();
	}
}

void LevelObject::Draw() {

	// Object3dの描画
	object3d_->Draw();
}

void LevelObject::DrawCollider() {
#ifdef _DEBUG
	//collider_->DrawCollider();
#endif
}

void LevelObject::OnCollisionAction(GameCharacter* other) {


	if(other->GetCharacterType() == CharacterType::PLAYER_BULLET ||
	   other->GetCharacterType() == CharacterType::ENEMY_BULLET) {
		isBlinking_ = true; // 点滅開始
	} 

	if(other->GetCharacterType() == CharacterType::PLAYER) {
		// ここにレベルオブジェクトとプレイヤーまたは敵の衝突時の処理を追加
		
		//衝突面に応じて処理を分ける
		if(other->GetCollider()->GetSurfaceType() == SurfaceType::TOP) {
			//地面に接触した場合
			collider_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 緑色に変更
		} else if(other->GetCollider()->GetSurfaceType() == SurfaceType::WALL) {
			//壁に接触した場合]
			collider_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色に変更
		} else if(other->GetCollider()->GetSurfaceType() == SurfaceType::BOTTOM) {
			//天井に接触した場合
			collider_->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f }); // 青色に変更
		} else {
			collider_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色に変更
		}
	} 
}
