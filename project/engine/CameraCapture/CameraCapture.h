#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl.h>

#include <cstdint>
#include <string>
#include <vector>

namespace TakeC {

	class DirectXCommon;
	class SrvManager;

	//=============================================================
	// CameraCapture class
	//=============================================================
	class CameraCapture {
	public:
		template <class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

		/// <summary>
		/// カメラデバイス情報
		/// </summary>
		struct DeviceInfo {
			std::wstring name;
			std::wstring symbolicLink;
		};

		struct ImGuiImageRect {
			float minX = 0.0f;
			float minY = 0.0f;
			float maxX = 0.0f;
			float maxY = 0.0f;
		};

	public:

		//=============================================================
		// Public methods
		//=============================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		CameraCapture() = default;
		~CameraCapture();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns></returns>
		bool Initialize();
		/// <summary>
		/// ImGui表示用リソースの初期化
		/// </summary>
		/// <param name="dxCommon"></param>
		/// <param name="srvManager"></param>
		/// <returns></returns>
		bool InitializeImGuiResources(DirectXCommon* dxCommon, SrvManager* srvManager);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// カメラデバイスの列挙
		/// </summary>
		/// <returns></returns>
		std::vector<DeviceInfo> EnumerateDevices() const;

		/// <summary>
		/// カメラデバイスのオープン
		/// </summary>
		/// <param name="deviceIndex"></param>
		/// <param name="preferredWidth"></param>
		/// <param name="preferredHeight"></param>
		/// <param name="preferredFps"></param>
		/// <returns></returns>
		bool OpenDevice(uint32_t deviceIndex, uint32_t preferredWidth = 640, uint32_t preferredHeight = 480, uint32_t preferredFps = 30);

		/// <summary>
		/// カメラデバイスのクローズ
		/// </summary>
		void CloseDevice();

		/// <summary>
		/// 更新処理
		/// </summary>
		/// <returns></returns>
		bool Update();
		/// <summary>
		/// ImGui更新処理
		/// </summary>
		/// <param name="windowName"></param>
		void UpdateImGui(const char* windowName = "Camera Capture");

		/// <summary>
		/// 初期化済みかどうかを取得
		/// </summary>
		/// <returns></returns>
		bool IsInitialized() const { return initialized_; }
		/// <summary>
		/// カメラデバイスがオープンされているかどうかを取得
		/// </summary>
		/// <returns></returns>
		bool IsOpen() const { return sourceReader_ != nullptr; }

		/// <summary>
		/// 新しいフレームがあるかどうかを取得
		/// </summary>
		/// <returns></returns>
		bool HasNewFrame() const { return hasNewFrame_; }


	public:

		//=============================================================
		// accessor
		//=============================================================

		//---- getter ----

		// RGBA形式のフレームデータを取得
		const uint8_t* GetFrameRGBA() const { return frameRGBA_.empty() ? nullptr : frameRGBA_.data(); }
		// RGBA形式のフレームデータをstd::vectorで取得
		const std::vector<uint8_t>& GetFrameRGBAData() const { return frameRGBA_; }
		// フレームの幅を取得
		uint32_t GetWidth() const { return width_; }
		// フレームの高さを取得
		uint32_t GetHeight() const { return height_; }
		// RGBA形式のフレームの1行のバイト数を取得
		uint32_t GetFrameStrideRGBA() const { return width_ * 4; }
		// RGBA形式のフレームのバイト数を取得
		size_t GetFrameByteSize() const { return frameRGBA_.size(); }
		// 最後のエラーメッセージを取得
		const std::wstring& GetLastErrorMessage() const { return lastErrorMessage_; }
		// 直近のImGui::Image表示矩形を取得
		bool HasLastImGuiImageRect() const { return hasLastImGuiImageRect_; }
		const ImGuiImageRect& GetLastImGuiImageRect() const { return lastImGuiImageRect_; }

	private:

		//=============================================================
		// Private methods
		//=============================================================

		// 最後のエラーメッセージを設定
		void SetLastErrorMessage(const wchar_t* message);
		// 最後のエラーメッセージを設定（HRESULT付き）
		void SetLastErrorMessage(const wchar_t* message, HRESULT hr);

		/// <summary>
		/// カメラデバイスのソースリーダーを作成
		/// </summary>
		/// <param name="activate"></param>
		/// <returns></returns>
		bool CreateSourceReader(IMFActivate* activate);

		/// <summary>
		/// 出力メディアタイプを設定
		/// </summary>
		/// <param name="preferredWidth"></param>
		/// <param name="preferredHeight"></param>
		/// <param name="preferredFps"></param>
		/// <returns></returns>
		bool ConfigureOutputMediaType(uint32_t preferredWidth, uint32_t preferredHeight, uint32_t preferredFps);

		/// <summary>
		/// 現在のフレームフォーマットを更新
		/// </summary>
		/// <returns></returns>
		bool UpdateCurrentFormat();

		/// <summary>
		/// IMFSampleからフレームデータをコピー
		/// </summary>
		/// <param name="sample"></param>
		/// <returns></returns>
		bool CopySampleToFrame(IMFSample* sample);
		/// <summary>
		/// ImGui表示用リソースを確保
		/// </summary>
		/// <returns></returns>
		bool EnsureDisplayResources();
		/// <summary>
		/// 最新フレームをGPUテクスチャへアップロード
		/// </summary>
		/// <returns></returns>
		bool UploadFrameToTexture();
		/// <summary>
		/// ImGui表示用リソースを解放
		/// </summary>
		void ReleaseDisplayResources();

	private:

		// 初期化済みかどうか
		bool initialized_ = false;
		// Media Foundationの初期化済みかどうか
		bool mfStarted_ = false;
		// COMの初期化済みかどうか
		bool comInitialized_ = false;
		// 新しいフレームがあるかどうか
		bool hasNewFrame_ = false;

		// Media Foundationのオブジェクト
		ComPtr<IMFMediaSource> mediaSource_;
		// ソースリーダー
		ComPtr<IMFSourceReader> sourceReader_;

		// ImGui表示用リソース
		DirectXCommon* dxCommon_ = nullptr;
		SrvManager* srvManager_ = nullptr;
		ComPtr<ID3D12Resource> displayTexture_;
		ComPtr<ID3D12Resource> uploadBuffer_;
		uint8_t* mappedUploadData_ = nullptr;
		uint32_t displaySrvIndex_ = 0;
		uint32_t displayWidth_ = 0;
		uint32_t displayHeight_ = 0;
		uint64_t uploadBufferRowPitch_ = 0;
		D3D12_RESOURCE_STATES displayTextureState_ = D3D12_RESOURCE_STATE_GENERIC_READ;
		bool displaySrvAllocated_ = false;
		bool hasLastImGuiImageRect_ = false;
		ImGuiImageRect lastImGuiImageRect_;

		// RGBA形式のフレームデータ
		std::vector<uint8_t> frameRGBA_;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		LONG sourceStride_ = 0;

		// 最後のエラーメッセージ
		std::wstring lastErrorMessage_;
	};

}
