#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#include <wrl.h>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

class DirectXCommon;
class TextureManager {
public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			エイリアステンプレート
	/////////////////////////////////////////////////////////////////////////////////////

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	/// <summary>
	/// インスタンス取得
	/// </summary>
	static TextureManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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



private:
	//テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath; //ファイルパス
		DirectX::TexMetadata metadata; //テクスチャのメタデータ
		ComPtr<ID3D12Resource> resource; //テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; //CPUディスクリプタハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; //GPUディスクリプタハンドル
	};

private:
	//シングルトンインスタンス
	static TextureManager* instance_;

	//SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

private:

	//directXCommonのインスタンス
	DirectXCommon* dxCommon_ = nullptr;

	//テクスチャデータのリスト
	std::vector<TextureData> textureDatas_;

};

