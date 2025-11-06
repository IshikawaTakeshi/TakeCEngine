#pragma once
#include <memory>
#include <optional>
#include "engine/2d/Sprite.h"
#include "engine/Utility/Timer.h"


// フェーズメッセージ列挙型
enum class PhaseMessage {
	READY,
	FIGHT,
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


	PhaseMessage GetCurrentMessage() const {
		return currentMessage_;
	}

	std::optional<PhaseMessage> GetNextMessage() const {
		return nextMessage_;
	}

	void SetNextMessage(PhaseMessage message);
private:

	//========================================================================
	// variables
	//========================================================================
	
	// フェーズメッセージスプライト
	std::unique_ptr<Sprite> phaseMessageText_ = nullptr;
	//帯スプライト
	std::unique_ptr<Sprite> bandSprite_ = nullptr;

	// アルファ値
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	std::optional<PhaseMessage> nextMessage_ = std::nullopt;
	PhaseMessage currentMessage_ = PhaseMessage::COUNT;

	Vector2 originalTextSizeMin_ = { 0.0f,0.0f };
	Vector2 originalTextSizeMax_ = { 0.0f,0.0f };

	Vector2 originalBandSizeMin_ = { 0.0f,0.0f };
	Vector2 originalBandSizeMax_ = { 0.0f,0.0f };

	Vector2 screenCenter_;

	bool isMessageFinished_ = false;
};