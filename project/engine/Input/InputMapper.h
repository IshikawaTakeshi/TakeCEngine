#pragma once

//	include
#include "Input/IInputDevice.h"
#include "engine/math/Vector2.h"

// c++
#include <memory>
#include <vector>
#include <algorithm>

//============================================================================
//	InputMapper class
//============================================================================
template<InputEnum Enum>
class InputMapper {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	InputMapper() = default;
	~InputMapper() = default;

	// 使用する入力デバイスを追加
	void AddDevice(std::unique_ptr<IInputDevice<Enum>> device);

	//--------- accessor -----------------------------------------------------

	//----- getter --------------------

	// 入力取得
	float GetVector(Enum action) const;
	// 2Dベクトル入力取得
	Vector2 GetVector2(Enum action) const;

	// ボタンが押されているか
	bool IsPressed(Enum button) const;
	// ボタンがトリガーされたか
	bool IsTriggered(Enum button) const;

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 使用する入力デバイス
	std::vector<std::unique_ptr<IInputDevice<Enum>>> devices_;
};

//============================================================================
//	InputMapper templateMethods
//============================================================================

//-------------------------------------------------------------------------
// 使用する入力デバイスを追加
//-------------------------------------------------------------------------
template<InputEnum Enum>
inline void InputMapper<Enum>::AddDevice(std::unique_ptr<IInputDevice<Enum>> device) {

	devices_.emplace_back(std::move(device));
}

//-------------------------------------------------------------------------
// 入力取得
//-------------------------------------------------------------------------
template<InputEnum Enum>
inline float InputMapper<Enum>::GetVector(Enum action) const {

	float vector = 0.0f;
	// 各入力から値を入れる
	for (const auto& device : devices_) {

		vector += device->GetVector(action);
	}
	vector = std::clamp(vector, -1.0f, 1.0f);
	return vector;
}

//-------------------------------------------------------------------------
// 2Dベクトル入力取得
//-------------------------------------------------------------------------
template<InputEnum Enum>
inline Vector2 InputMapper<Enum>::GetVector2(Enum action) const {

	Vector2 vector{ 0.0f, 0.0f };
	// 各入力から値を入れる
	for (const auto& device : devices_) {
		Vector2 inputVec = device->GetVector2(action);
		vector.x += inputVec.x;
		vector.y += inputVec.y;
	}
	// 正規化
	if (vector.Length() > 1.0f) {
		vector = vector.Normalize();
	}
	return vector;
}

//-------------------------------------------------------------------------
// ボタンが押されているか
//-------------------------------------------------------------------------
template<InputEnum Enum>
inline bool InputMapper<Enum>::IsPressed(Enum button) const {

	// 入力結果を取得
	for (const auto& device : devices_) {
		if (device->IsPressed(button)) {

			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------
// ボタンがトリガーされたか
//-------------------------------------------------------------------------
template<InputEnum Enum>
inline bool InputMapper<Enum>::IsTriggered(Enum button) const {

	// 入力結果を取得
	for (const auto& device : devices_) {
		if (device->IsTriggered(button)) {

			return true;
		}
	}
	return false;
}