#pragma once
#include "BaseUI.h"
#include "engine/Input/InputData.h"
#include "engine/math/Vector2.h"
#include "application/Provider/PlayerInputProvider.h"
#include <memory>
#include <string>

//============================================================================
// ActionButtonICon class
//============================================================================
class ActionButtonICon : public BaseUI {
public:
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ActionButtonICon() = default;
	~ActionButtonICon() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="spriteManager">SpriteManagerへのポインタ</param>
	/// <param name="position">表示座標</param>
	/// <param name="targetButton">監視するゲームパッドボタン</param>
	void Initialize(TakeC::SpriteManager* spriteManager,const std::string& configName, 
		PlayerInputProvider* inputProvider,CharacterActionInput targetAction);

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui(const std::string& name) override;

private:

	// 監視するボタン
	PlayerInputProvider* inputProvider_ = nullptr;
	CharacterActionInput targetAction_ = CharacterActionInput::JUMP;

	// 基準サイズ（押していないときのサイズ）
	Vector2 baseSize_ = { 64.0f, 64.0f };

	// 押し込み時の縮小率
	float pressScale_ = 0.9f;

};