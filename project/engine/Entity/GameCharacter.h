#pragma once
#include "engine/3d/Object3d.h"
#include "engine/Collision/Collider.h"

//キャラクターの種類列挙型
enum CharacterType {
	NONE,
	PLAYER,
	PLAYER_BULLET,
	PLAYER_MISSILE,
	ENEMY,
	ENEMY_BULLET,
	ENEMY_MISSILE,
	LEVEL_OBJECT,
	BULLET_SENSOR,
};

//============================================================================
// GameCharacter class
//============================================================================
class GameCharacter {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	GameCharacter() = default;
	virtual ~GameCharacter() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon"></param>
	/// <param name="filePath"></param>
	virtual void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// コライダーの描画
	/// </summary>
	virtual void DrawCollider() = 0;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollisionAction(GameCharacter* other) = 0;

public:

	//=======================================================================
	// accessor
	//=======================================================================

	//----- getter ---------------

	/// コライダーの取得
	virtual Collider* GetCollider() { return collider_.get(); }
	/// キャラクタータイプの取得
	virtual CharacterType GetCharacterType() { return characterType_; }
	/// Object3dの取得
	Object3d* GetObject3d() { return object3d_.get(); }

	//----- setter ---------------

	/// キャラクタータイプの設定
	virtual void SetCharacterType(CharacterType type) { characterType_ = type; }
	/// 位置の設定
	virtual void SetTranslate(const Vector3& translate) { object3d_->SetTranslate(translate); }
	/// 回転の設定
	virtual void SetRotate(const Vector3& rotate) { object3d_->SetRotate(rotate); }
	/// スケールの設定
	virtual void SetScale(const Vector3& scale) { object3d_->SetScale(scale); }
	/// カメラの設定
	virtual void SetCamera(Camera* camera) { object3d_->SetCamera(camera); }

protected:

	// 3Dオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// コライダー
	std::unique_ptr<Collider> collider_;
	// キャラクタータイプ
	CharacterType characterType_;
};