#include "Texture.h"

#include "Logger.h"
#include "DirectXCommon.h"
#include "WinApp.h"

Texture* Texture::GetInstance() {
	static Texture* texture;
	return texture;
}

void Texture::Initialize(DirectXCommon* dxCommon) {

	//Textureを読み込んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("./Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = CreateTextureResource(dxCommon->GetDevice(), metadata);
	uploadTextureData(textureResource_, mipImages);

	//DepthstencilTextureをウィンドウのサイズで作成
	depthStencilResource_ = CreateDepthStencilTextureResource(
		dxCommon->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);

	//metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //format.基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture
	//DSVHeapの先頭にDSVを作る
	dxCommon->GetDevice()->CreateDepthStencilView(
		depthStencilResource_, &dsvDesc, dxCommon->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart());

	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU_ = dxCommon->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU_ = dxCommon->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart();
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU_.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU_.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource_.Get(), &srvDesc, textureSrvHandleCPU_);

}

void Texture::Finalize() {

	depthStencilResource_.Reset();
	textureResource_.Reset();
}

DirectX::ScratchImage Texture::LoadTexture(const std::string& filePath) {
	
	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = Logger::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0,mipImages);
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

void Texture::uploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//ぜんMipMapについて
	for (size_t mipLevel = 0;  mipLevel < metadata.mipLevels;  ++mipLevel) {
		
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

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::CreateDepthStencilTextureResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
	
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; //Texureの幅
	resourceDesc.Height = height; //Textureの高さ
	resourceDesc.MipLevels = 1; //mipmapの数
	resourceDesc.DepthOrArraySize = 1; //奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; //サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; //2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; //DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; //VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthCLearValue{};
	depthCLearValue.DepthStencil.Depth = 1.0f; //1.0f(最大値)でクリア
	depthCLearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //フォーマット。Resoureと合わせる

	//Resoureの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
		&depthCLearValue, //Clear最適値。
		IID_PPV_ARGS(&depthStencilResource_) //作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return depthStencilResource_;
}
