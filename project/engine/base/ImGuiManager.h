#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "engine/Base/WinApp.h"
#include "engine/Base/DirectXCommon.h"
#include "engine/Base/SrvManager.h"
#include "engine/Utility/JsonLoader.h"
#include "engine/Utility/StringUtility.h"
#include "engine/Math/Quaternion.h"
#include "engine/Math/Vector3.h"
#include "engine/Math/Matrix4x4.h"
#include <string>



//============================================================================
// ImGuiManager class
//============================================================================
namespace TakeC {

	// enumのみに制約をかける
	template<typename T>
	concept InputEnum = std::is_enum_v<T>;

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

		template<InputEnum TEnum>
		static bool ComboBoxEnum(const char* label, TEnum& currentItem);

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


	///--------------------------------------------------------------------
	/// 保存ポップアップの表示
	///---------------------------------------------------------------------
	template<typename T>
	inline bool ImGuiManager::ShowSavePopup(
		JsonLoader* jsonLoader,
		const char* popupId, const char* defaultFilename, const T& data, std::string& outFilePath) {

		// 保存完了フラグ
		bool saved = false;

		//保存ボタン
		if (ImGui::Button("SaveConfig"))
		{
			ImGui::OpenPopup(popupId);
		}

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

	///--------------------------------------------------------------------
	/// Enum用コンボボックス
	///---------------------------------------------------------------------
	template<InputEnum TEnum>
	inline bool ImGuiManager::ComboBoxEnum(const char* label, TEnum& currentItem) {
		
		// 変更フラグ
		bool valueChanged = false;

		// enumの全エントリを取得
		const auto enumTypes = magic_enum::enum_entries<TEnum>();
		// 現在のenumのインデックスを取得
		int currentIndex = static_cast<int>(magic_enum::enum_index(currentItem).value_or(0));
		// 変更前の値を保存
		TEnum oldValue = currentItem;

		// コンボボックスの表示
		if(ImGui::BeginCombo(label, magic_enum::enum_name(currentItem).data())) {
			// enumエントリの表示
			for (size_t i = 0; i < enumTypes.size(); ++i) {
				const bool isSelected = (currentIndex == static_cast<int>(i));
				if (ImGui::Selectable(enumTypes[i].second.data(), isSelected)) {
					currentItem = enumTypes[i].first;
					// 値が変更された場合、フラグを立てる
					if (oldValue != currentItem) {
						valueChanged = true;
					}
				}
				// デフォルトフォーカスの設定
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return valueChanged;
	}
}