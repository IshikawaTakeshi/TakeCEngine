#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/3d/Object3dCommon.h"
#include "scene/LevelData.h"
#include <string>

//================================================================================
// LevelObject class
//================================================================================
class LevelObject : public GameCharacter {
public:

	//============================================================================
	// functions
	//============================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	LevelObject() = default;
	~LevelObject() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon"></param>
	/// <param name="filePath"></param>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;

	/// <summary>
	/// コリジョン初期化(BoxCollider)
	/// </summary>
	/// <param name="boxInfo"></param>
	void CollisionInitialize(const LevelData::BoxCollider& boxInfo);

	/// <summary>
	/// コリジョン初期化(SphereCollider)
	/// </summary>
	/// <param name="sphereInfo"></param>
	void CollisionInitialize(const LevelData::SphereCollider& sphereInfo);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// コライダー描画処理
	/// </summary>
	void DrawCollider() override;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	/// <param name="other"></param>
	void OnCollisionAction(GameCharacter* other) override;

public:

	//=============================================================================
	// accessor
	//=============================================================================
	
	//----- getter ----------------

	// コライダーの取得
	Collider* GetCollider() override { return collider_.get(); }
	// キャラクタータイプの取得・設定
	CharacterType GetCharacterType() override { return characterType_; }

	//----- setter ----------------

	// キャラクタータイプの設定
	void SetCharacterType(CharacterType type) override { characterType_ = type; }
	// オブジェクトの名前の設定
	void SetName(const std::string& name) { name_ = name; }

private:

	Object3dCommon* object3dCommon_ = nullptr; // Object3dCommonの参照

	std::string name_; // オブジェクトの名前

	float deltaTime_ = 0.0f; // 更新間隔

	//衝突時の点滅タイマー
	bool isBlinking_ = false; // 点滅中かどうか
	float blinkTimer_ = 0.0f;
	const float kBlinkInterval = 0.1f; // 点滅間隔
	const float kBlinkDuration = 1.0f; // 点滅持続時間
};