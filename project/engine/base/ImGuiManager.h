#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "base/WinApp.h"
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <string>

//============================================================================
// ImGuiManager class
//============================================================================
namespace TakeC {
	class ImGuiManager {
	public:

		//====================================================================
		// functions
		//====================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		ImGuiManager() = default;
		~ImGuiManager() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// ImGUi受付開始
		/// </summary>
		void Begin();

		/// <summary>
		/// ImGui受付終了
		/// </summary>
		void End();

		/// <summary>
		/// デバッグ画面の描画
		/// </summary>
		void DrawDebugScreen();

		/// <summary>
		/// 描画後処理
		/// </summary>
		void PostDraw();

		/// <summary>
		/// クォータニオンの画面表示
		/// </summary>
		/// <param name="label"></param>
		/// <param name="q"></param>
		static void QuaternionScreenPrintf(const std::string& label, const Quaternion& q);

		/// <summary>
		/// ベクトル3の画面表示
		/// </summary>
		/// <param name="label"></param>
		/// <param name="v"></param>
		static void Vector3ScreenPrintf(const std::string& label, const Vector3& v);

		/// <summary>
		/// 行列4x4の画面表示
		/// </summary>
		/// <param name="label"></param>
		/// <param name="m"></param>
		static void Matrix4x4ScreenPrintf(const std::string& label, const Matrix4x4& m);

		/// <summary>
		/// レンダーテクスチャのSRVインデックス設定
		/// </summary>
		/// <param name="index"></param>
		void SetRenderTextureIndex(uint32_t index) {
			renderTextureIndex_ = index;
		}
	private:

		bool useReleaseSize_ = false;
		DirectXCommon* dxCommon_ = nullptr; //DXCommon
		SrvManager* srvManager_ = nullptr; //SrvManager
		uint32_t renderTextureIndex_ = 0; // ImGuiのレンダリングターゲットのSRVインデックス
		ImVec2 releaseImageSize_{ 1920.0f,1080.0f }; // リリース時のイメージサイズ
		ImGuiWindowFlags windowFlags_ = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	};
}