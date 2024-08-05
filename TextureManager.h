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
	/// 終了処理
	/// </summary>
	void Finalize();

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

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

private:

	//テクスチャデータのリスト
	std::vector<TextureData> textureDataList_;

};

