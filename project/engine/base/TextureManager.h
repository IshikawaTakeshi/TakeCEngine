#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

#include <wrl.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "DirectXCommon.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

class SrvManager;
class TextureManager {
public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			エイリアステンプレート
	/////////////////////////////////////////////////////////////////////////////////////

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	/////////////////////////////////////////////////////////////////////////////////////
	///			publicメンバ関数
	/////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// インスタンス取得
	/// </summary>
	static TextureManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	/// <returns>画像イメージデータ</returns>
	void LoadTexture(const std::string& filePath);

	/// <summary>
	/// テクスチャリソースの作成
	/// </summary>
	/// <param name="device"></param>
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
	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

public:

	/////////////////////////////////////////////////////////////////////////////////////
	///			getter
	/////////////////////////////////////////////////////////////////////////////////////

	/// GPUハンドル取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);
	/// SRVインデックス取得
	uint32_t GetSrvIndex(const std::string& filePath);
	/// メタデータ取得
	const DirectX::TexMetadata& GetMetadata(const std::string& filePath);

	SrvManager* GetSrvManager() { return srvManager_; }

private:

	/////////////////////////////////////////////////////////////////////////////////////
	///			構造体
	/////////////////////////////////////////////////////////////////////////////////////
	
	//テクスチャ1枚分のデータ
	struct TextureData {
		DirectX::TexMetadata metadata; //テクスチャのメタデータ
		ComPtr<ID3D12Resource> resource; //テクスチャリソース
		uint32_t srvIndex; //SRVインデックス
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; //CPUディスクリプタハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; //GPUディスクリプタハンドル
	};

private:

	/////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	/////////////////////////////////////////////////////////////////////////////////////

	//シングルトンインスタンス
	static TextureManager* instance_;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

private:

	//directXCommonのインスタンス
	DirectXCommon* dxCommon_ = nullptr;

	//テクスチャデータのリスト
	std::unordered_map<std::string, TextureData> textureDatas_;

	SrvManager* srvManager_ = nullptr;

};

