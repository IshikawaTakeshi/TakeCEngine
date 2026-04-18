#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include <Xinput.h>
#include <wrl.h>
#include <cstdint>
#include <vector>
#include <memory>

#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/math/Vector2.h"
#include "engine/Input/Gamepad.h"

//前方宣言
namespace TakeC {
	class WinApp;
}

//============================================================================
// Input class
//============================================================================
namespace TakeC {
	class Input {
	private:

		//コピーコンストラクタ・代入演算子禁止
		Input() = default;
		~Input() = default;
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

	public:

		// マウス移動量構造体
		struct MouseMove {
			LONG lX;
			LONG lY;
			LONG lZ;
		};

	public:

		/// <summary>
		/// シングルトンインスタンスの取得
		/// </summary>
		/// <returns></returns>
		static Input& GetInstance();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="winApp"></param>
		void Initialize(TakeC::WinApp* winApp);

		/// <summary>
		/// 更新処理
		/// </summary>
		/// <param name="winApp"></param>
		void Update();

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// キーの押下をチェック
		/// </summary>
		/// <param name="keyNumber">キー番号</param>
		/// <returns>押されているか</returns>
		bool PushKey(BYTE keyNumber);

		/// <summary>
		/// キーのトリガーをチェック。押した瞬間だけtrueになる
		/// </summary>
		/// <param name="keyNumber"></param>
		/// <returns></returns>
		bool TriggerKey(BYTE keyNumber);

		/// <summary>
		/// キーのリリースをチェック。離した瞬間だけtrueになる
		/// </summary>
		/// <param name="keyNumber"></param>
		/// <returns></returns>
		bool ReleaseKey(BYTE keyNumber);

		/// <summary>
		/// 全マウス情報取得
		/// </summary>
		/// <returns>マウス情報</returns>
		const DIMOUSESTATE2& GetAllMouse() const;

		/// <summary>
		/// マウスの押下をチェック
		/// </summary>
		/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
		/// <returns>押されているか</returns>
		bool PressMouse(int32_t mouseNumber) const;

		/// <summary>
		/// マウスのトリガーをチェック。押した瞬間だけtrueになる
		/// </summary>
		/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
		/// <returns>トリガーか</returns>
		bool TriggerMouse(int32_t buttonNumber) const;

		/// <summary>
		/// マウスのリリースをチェック。離した瞬間だけtrueになる
		/// </summary>
		/// <param name="buttonNumber"></param>
		/// <returns></returns>
		bool ReleaseMouse(int32_t buttonNumber) const;

		/// <summary>
		/// マウス移動量を取得
		/// </summary>
		/// <returns>マウス移動量</returns>
		MouseMove GetMouseMove();

		/// <summary>
		/// ホイールスクロール量を取得する
		/// </summary>
		/// <returns>ホイールスクロール量。奥側に回したら+。Windowsの設定で逆にしてたら逆</returns>
		int32_t GetWheel() const;

		/// <summary>
		/// マウスの位置を取得する（ウィンドウ座標系）
		/// </summary>
		/// <returns>マウスの位置</returns>
		const Vector2& GetCursorPosition() const;

		//====================================================================
		//		GamePad
		//====================================================================


		//ゲームパッドの押下チェック
		bool PushButton(int stickNo, GamepadButtonType button) const;
		//ゲームパッドのトリガーチェック
		bool TriggerButton(int stickNo, GamepadButtonType button) const;
		//ゲームパッドのリリースチェック
		bool ReleaseButton(int stickNo, GamepadButtonType button) const;

		//ボタンの押し込み量を取得
		float GetTriggerValue(int stickNo, GamepadButtonType button) const;
		
		//スティックの状況を取得
		Vector2 GetLeftStickState(int stickNo) const;
		//スティックの状況を取得
		Vector2 GetRightStickState(int stickNo) const;
		// スティックの値を取得
		Vector2 GetStickValue(int stickNo, GamepadValueType valueType) const;


	private:

		//DirectInput
		ComPtr<IDirectInput8> directInput_ = nullptr;
		//キーボードデバイス
		ComPtr<IDirectInputDevice8> keyboardDevice_ = nullptr;
		//マウスデバイス
		ComPtr<IDirectInputDevice8> mouseDevice_ = nullptr;

		//GamePad
		std::unique_ptr<GamePad> gamePad_;

		//WindowsAPI
		TakeC::WinApp* winApp_ = nullptr;
		//キー情報
		BYTE key[256] = {};
		//前回フレームのキーデータ
		BYTE preKey[256] = {};
		//マウス情報
		DIMOUSESTATE2 mouse_;
		//前回フレームのマウス情報
		DIMOUSESTATE2 mousePre_;
		//マウス位置
		Vector2 mousePosition_;
	};
}