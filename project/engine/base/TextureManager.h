#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

#include <wrl.h>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>

#include "engine/base/DirectXCommon.h"
#include "engine/base/ComPtrAliasTemplates.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


//===================================================================================
// TextureManager class
//===================================================================================
namespace TakeC {

	//前方宣言
	class SrvManager;

	class TextureManager {
	private:

		//コンストラクタ・デストラクタ・コピー禁止
		TextureManager() = default;
		~TextureManager() = default;
		TextureManager(TextureManager&) = delete;
		TextureManager& operator=(TextureManager&) = delete;

	public:

		/////////////////////////////////////////////////////////////////////////////////////
		///			構造体
		/////////////////////////////////////////////////////////////////////////////////////

		//テクスチャ1枚分のデータ
		struct TextureData {
			DirectX::TexMetadata metadata; //テクスチャのメタデータ
			ComPtr<ID3D12Resource> resource; //テクスチャリソース
			ComPtr<ID3D12Resource> intermediateResource; //中間リソース
			uint32_t srvIndex; //SRVインデックス
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; //CPUディスクリプタハンドル
			D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; //GPUディスクリプタハンドル
		};

		struct TextureCPUData {
			std::string filePath; //テクスチャファイルのパス
			DirectX::ScratchImage mipImages; //ミップマップを含むテクスチャイメージ
			DirectX::TexMetadata metadata; //テクスチャのメタデータ
		};

	public:

		//================================================================================
		// functions
		//================================================================================

		/// <summary>
		/// インスタンス取得
		/// </summary>
		static TextureManager& GetInstance();

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize();

		void Update();

		/// <summary>
		/// テクスチャの更新チェックと再読み込み
		/// </summary>
		void CheckAndReloadTextures();

		/// <summary>
		/// テクスチャファイルの読み込み
		/// </summary>
		/// <param name="filePath">テクスチャファイルのパス</param>
		/// <param name="forceReload">強制再読み込みフラグ</param>
		/// <returns>画像イメージデータ</returns>
		void LoadTexture(const std::string& filePath, bool forceReload);

		/// <summary>
		/// 全テクスチャの読み込み
		/// </summary>
		void LoadTextureAll();

		/// <summary>
		/// すべての読み込み完了を待機
		/// </summary>
		void WaitForAllLoads();

	private:

		std::string NormalizeTextureFilePath(const std::string& filePath);

		/// <summary>
		/// テクスチャリソースの作成
		/// </summary>
		/// <param name="metadata"></param>
		/// <returns></returns>
		[[nodiscard]]
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

		/// <summary>
		/// テクスチャデータのアップロード
		/// </summary>
		/// <param name="texture"></param>
		/// <param name="mipImages"></param>
		/// <returns></returns>
		[[nodiscard]]
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

		/// <summary>
		/// ファイルの最終更新時刻を取得
		/// </summary>
		time_t GetFileLastWriteTime(const std::string& filePath);

		/// <summary>
		/// テクスチャファイルの拡張子がサポートされているかチェック
		/// </summary>
		/// <param name="ext"></param>
		/// <returns></returns>
		bool IsSupportedTextureExt(const std::string& ext);

		/// <summary>
		/// テクスチャファイルのデコード
		/// </summary>
		/// <param name="filePath"></param>
		/// <param name="outImage"></param>
		/// <param name="outMetadata"></param>
		/// <returns></returns>
		bool DecodeTexture(const std::string& filePath, DirectX::ScratchImage& outImage, DirectX::TexMetadata& outMetadata);

		/// <summary>
		/// ミップマップの生成
		/// </summary>
		/// <param name="srcImage"></param>
		/// <param name="metadata"></param>
		/// <param name="outMipImage"></param>
		/// <returns></returns>
		bool BuildMipMaps(const DirectX::ScratchImage& srcImage, const DirectX::TexMetadata& metadata, DirectX::ScratchImage& outMipImage);

		/// <summary>
		/// テクスチャのGPUアップロード
		/// </summary>
		/// <param name="mipImages"></param>
		/// <param name="metadata"></param>
		/// <param name="textureData"></param>
		/// <returns></returns>
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadTexture(const DirectX::ScratchImage& mipImages, DirectX::TexMetadata& metadata, TextureData& textureData);

		void StartWorker();
		void WorkerLoop();
	public:

		//=============================================================================
		// accessor
		//=============================================================================

		//----- getter ---------------

		/// GPUハンドル取得
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);
		/// SRVインデックス取得
		uint32_t GetSrvIndex(const std::string& filePath);
		/// メタデータ取得
		const DirectX::TexMetadata& GetMetadata(const std::string& filePath);
		/// SrvManager取得
		SrvManager* GetSrvManager() { return srvManager_; }

		//現在読み込み済みのファイル名一覧を取得する
		std::vector<std::string> GetLoadedTextureFileNames() const;
		//CubeMapを除いたテクスチャのファイル名一覧を取得する
		std::vector<std::string> GetLoadedNonCubeTextureFileNames() const;

	


	private:

		//directXCommonのインスタンス
		TakeC::DirectXCommon* dxCommon_ = nullptr;

		//テクスチャデータのリスト
		std::unordered_map<std::string, TextureData> textureDatas_;
		//テクスチャファイルの更新時刻を管理するマップ
		std::unordered_map<std::string, time_t> fileUpdateTimes_;

		TakeC::SrvManager* srvManager_ = nullptr;


		std::vector<std::thread> workerThreads_;
		// リクエスト（読み込み依頼）
		std::queue<std::string> requestQueue_;
		// 完了（CPU処理済み）
		std::queue<TextureCPUData> completedQueue_;
		std::mutex mutex_; // queue用
		mutable std::mutex mapMutex_; // textureDatas_用
		std::atomic<bool> threadRunning_ = true;

		// 読み込み待ち数
		std::atomic<int> pendingCount_ = 0;
		// プレースホルダ用（白テクスチャ）
		TextureData whiteTextureData_;
	};
}