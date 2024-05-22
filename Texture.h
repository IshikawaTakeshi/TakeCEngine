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
class Texture {
public:

	Texture() = default;
	~Texture() = default;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Texture* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata);

	void uploadTextureData(
		Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
	/// textureSrvHandleGPUの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() const { return textureSrvHandleGPU_; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
};

