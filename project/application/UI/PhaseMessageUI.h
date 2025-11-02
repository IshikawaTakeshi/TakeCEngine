#pragma once
#include <memory>
#include <optional>
#include "engine/2d/Sprite.h"
#include "engine/Utility/Timer.h"

// フェーズメッセージ列挙型
enum class PhaseMessage {
	READY,
	GO,
	WIN,
	LOSE,
	COUNT,
};

//============================================================================
// PhaseMessageUI class
//============================================================================
class PhaseMessageUI {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	PhaseMessageUI() = default;
	~PhaseMessageUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:

	//========================================================================
	// variables
	//========================================================================
	
	// フェーズメッセージスプライト
	std::unique_ptr<Sprite> phaseMessageText_ = nullptr;
	//帯スプライト
	std::unique_ptr<Sprite> bandSprite_ = nullptr;
	// 表示タイマー
	Timer displayTimer_;
	// アルファ値
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	std::optional<PhaseMessage> nextMessage_ = std::nullopt;
	PhaseMessage currentMessage_ = PhaseMessage::COUNT;
};