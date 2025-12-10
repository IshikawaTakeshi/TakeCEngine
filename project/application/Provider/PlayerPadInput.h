#pragma once
#include "engine/Input/IInputDevice.h"
#include "engine/Input/Input.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

//============================================================================
// PlayerPadInput class
//============================================================================
class PlayerPadInput : public IInputDevice<CharacterActionInput> {
public:

	//========================================================================
	// functions
	//========================================================================

    /// <summary>
	/// コンストラクタ・デストラクタ
    /// </summary>
    PlayerPadInput() = default;
    ~PlayerPadInput() override = default;

    /// <summary>
	/// スティック入力取得(float)
    /// </summary>
    /// <param name="axis"></param>
    /// <returns></returns>
    float GetVector(CharacterActionInput axis) const override;

    /// <summary>
	/// スティック入力取得(Vector2)
    /// </summary>
    /// <param name="axis"></param>
    /// <returns></returns>
    Vector2 GetVector2(CharacterActionInput axis) const override;

    /// <summary>
	/// ボタン入力取得
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsPressed(CharacterActionInput button) const override;

    /// <summary>
	/// ボタン入力取得(トリガー)
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsTriggered(CharacterActionInput button) const override;

    /// <summary>
	/// ボタン入力取得(リリース)
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsReleased(CharacterActionInput button) const override;
};