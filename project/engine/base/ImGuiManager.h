#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "engine/Base/WinApp.h"
#include "engine/Base/DirectXCommon.h"
#include "engine/Base/SrvManager.h"
#include "engine/Utility/JsonLoader.h"
#include "engine/Math/Quaternion.h"
#include "engine/Math/Vector3.h"
#include "engine/Math/Matrix4x4.h"
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
		/// 保存ポップアップの表示
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="popupId"></param>
		/// <param name="defaultFilename"></param>
		/// <param name="data"></param>
		/// <param name="outFilePath"></param>
		/// <returns></returns>
		template<typename T>
		static bool ShowSavePopup(JsonLoader* jsonLoader,const char* popupId, const char* defaultFilename,
			const T& data, std::string& outFilePath);

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


	///====================================================================
	/// 保存ポップアップの表示
	///====================================================================
	template<typename T>
	inline bool ImGuiManager::ShowSavePopup(
		JsonLoader* jsonLoader,
		const char* popupId, const char* defaultFilename, const T& data, std::string& outFilePath) {

		// 保存完了フラグ
		bool saved = false;

		// 保存ポップアップの表示
		if (ImGui::BeginPopupModal(popupId, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			static char filenameBuf[256];
			static bool initialized = false;
			if (!initialized) {
				strcpy_s(filenameBuf, defaultFilename);
				initialized = true;
			}

			// ファイル名入力欄
			ImGui::Text("Input filename:");
			ImGui::InputText("Filename", filenameBuf, sizeof(filenameBuf));
			ImGui::Separator();

			// OKボタン
			if (ImGui::Button("OK", ImVec2(120, 0))) {
				outFilePath = std::string(filenameBuf);
				jsonLoader->SaveJsonData<T>(outFilePath, data);
				saved = true;
				initialized = false;  // リセット
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			// キャンセルボタン
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				initialized = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return saved;
	}
}