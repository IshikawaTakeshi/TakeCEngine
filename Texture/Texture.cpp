#include "Texture.h"

#include "../Include/Logger.h"
#include "../Include/DirectXCommon.h"
#include "../Include/WinApp.h"
#include "../StringUtility/StringUtility.h"

Texture::~Texture() {
	textureResource_.Reset();

}

void Texture::Initialize(uint32_t index, DirectXCommon* dxCommon, const std::string& filePath) {
	filePath_ = filePath;
	//Textureを読み込んで転送する
	DirectX::ScratchImage mipImages = LoadTexture(filePath_);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = CreateTextureResource(dxCommon->GetDevice(), metadata);
	UploadTextureData(textureResource_, mipImages);


	//metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//DSVの設定
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //format.基本的にはResourceに合わせる
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture
	//
	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU_ = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvHeap(), dxCommon->GetDescriptorSizeSRV(), index);
	textureSrvHandleGPU_ = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvHeap(), dxCommon->GetDescriptorSizeSRV(), index);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource_.Get(), &srvDesc, textureSrvHandleCPU_);

}

void Texture::Finalize() {
	textureResource_.Reset();
}

DirectX::ScratchImage Texture::LoadTexture(const std::string& filePath) {

	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0, mipImages);

	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata) {

	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; //細かい設定で行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; //WritebackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //プロセッサの近くに配置


	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ, //初回のResourceState.Textureは基本読むだけ
		nullptr, //Clear最適値。
		IID_PPV_ARGS(&textureResource_)
	);
	assert(SUCCEEDED(hr));
	return textureResource_;
}

void Texture::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//ぜんMipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {

		//MipMapLevelを指定して各imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		//Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}
}
