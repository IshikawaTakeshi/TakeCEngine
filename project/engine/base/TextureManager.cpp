#include "TextureManager.h"
#include "SrvManager.h"
#include "StringUtility.h"
#include "TransformMatrix.h"
#include <cassert>

TextureManager* TextureManager::instance_ = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			シングルトンインスタンスの取得
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			初期化
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {


	//dxCommon_の取得
	dxCommon_ = dxCommon;
	//SRVManagerの取得
	srvManager_ = srvManager;
	//SRVの数と同数
	textureDatas_.reserve(srvManager_->kMaxSRVCount_);
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			終了処理
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Finalize() {
	dxCommon_ = nullptr;
	textureDatas_.clear();
	delete instance_;
	instance_ = nullptr;
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			テクスチャファイルの読み込み
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::LoadTexture(const std::string& filePath) {

	//読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return;
	}

	//テクスチャ枚数上限チェック
	assert(srvManager_->CheckTextureAllocate());

	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr;
	if (filePathW.empty()) {
		hr = DirectX::LoadFromWICFile(L"Resources/black.png", DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));

	} else if(filePathW.ends_with(L".dds")){
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));

	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));
	}

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};

	//圧縮フォーマットかどうかの判定
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image); //圧縮フォーマットの場合はそのまま使う

	} else { //非圧縮フォーマットの場合はミップマップを作成

		hr = DirectX::GenerateMipMaps(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DirectX::TEX_FILTER_SRGB,
			0, mipImages);

		assert(SUCCEEDED(hr));
	}
	//テクスチャデータを追加して書き込む
	TextureData& textureData = textureDatas_[filePath];

	//テクスチャデータの設定
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);
	textureData.intermediateResource = UploadTextureData(textureData.resource, mipImages);

	//テクスチャデータの要素数番号をSRVのインデックスとして設定
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetSrvDescriptorHandleCPU(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetSrvDescriptorHandleGPU(textureData.srvIndex);
	
	//metadataを基にSRVの設定
	srvManager_->CreateSRVforTexture2D(
		textureData.metadata.IsCubemap(),
		textureData.metadata.format,
		UINT(textureData.metadata.mipLevels),
		textureData.resource.Get(),
		textureData.srvIndex
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			テクスチャリソースの作成
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {

	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	//利用するHeapの設定。VRAMを使う設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	HRESULT hr;
	hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, //データ転送される設定
		nullptr, //Clear最適値。
		IID_PPV_ARGS(&textureResource));

	assert(SUCCEEDED(hr));
	return textureResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//サブリソースの作成
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(
		dxCommon_->GetDevice(),
		mipImages.GetImages(),
		mipImages.GetImageCount(),
		mipImages.GetMetadata(),
		subresources);

	//中間リソースのサイズの取得
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(),0,UINT(subresources.size()));

	//中間リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;
	intermediateResource = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), intermediateSize);

	//中間リソースにデータをコピー
	UpdateSubresources(dxCommon_->GetCommandList(),texture.Get(),
	intermediateResource.Get(),0,0,UINT(subresources.size()),subresources.data());

	//Textureへ転送後に利用できるようにResourseStateを変更
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).srvHandleGPU;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {

	//読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return textureDatas_.at(filePath).srvIndex;
	}
	assert(false);
	return 0;
}

const DirectX::TexMetadata& TextureManager::GetMetadata(const std::string& filePath) {
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).metadata;
}

