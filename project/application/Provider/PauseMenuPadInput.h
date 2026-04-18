#pragma once
#include "engine/Input/IInputDevice.h"
#include "engine/Input/Input.h"
#include "application/Provider/Enum/PauseMenuInputEnum.h"

//============================================================================
// PauseMenuPadInput class
//============================================================================
class PauseMenuPadInput : public IInputDevice<PauseMenuInput> {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	PauseMenuPadInput() = default;
	~PauseMenuPadInput() override = default;

	/// <summary>
	/// スティック入力取得(float)
	/// </summary>
	/// <param name="axis"></param>
	/// <returns></returns>
	float GetVector(PauseMenuInput axis) const override;

	/// <summary>
	/// スティック入力取得(Vector2)
	/// </summary>
	/// <param name="axis"></param>
	/// <returns></returns>
	Vector2 GetVector2(PauseMenuInput axis) const override;

	/// <summary>
	/// ボタン入力取得
	/// </summary>
	/// <param name="button"></param>
	/// <returns></returns>
	bool IsPressed(PauseMenuInput button) const override;

	/// <summary>
	/// ボタン入力取得(トリガー)
	/// </summary>
	/// <param name="button"></param>
	/// <returns></returns>
	bool IsTriggered(PauseMenuInput button) const override;

	/// <summary>
	/// ボタン入力取得(リリース)
	/// </summary>
	/// <param name="button"></param>
	/// <returns></returns>
	bool IsReleased(PauseMenuInput button) const override;
};