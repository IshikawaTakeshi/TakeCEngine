#include "TextureManager.h"
#include "Include/DirectXCommon.h"
#include "StringUtility/StringUtility.h"

//ImGUiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

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

void TextureManager::Initialize() {

	//SRVの数と同数
	textureDatas_.reserve(DirectXCommon::kMaxSRVCount);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			終了処理
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Finalize() {
	delete instance_;
	instance_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			テクスチャファイルの読み込み
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::LoadTexture(const std::string& filePath) {

	//読み込み済みテクスチャを検索
	auto it = std::find_if(
		textureDatas_.begin(),
		textureDatas_.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath;}
	);
	if(it != textureDatas_.end()) {
		return;
	}

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

	//テクスチャデータの追加
	textureDatas_.resize(textureDatas_.size() + 1);
	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas_.back();

	//テクスチャデータの設定
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	//テクスチャデータの要素数番号をSRVのインデックスとして設定
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas_.size());
	textureData.srvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	//metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	//SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
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

	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; //細かい設定で行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; //WritebackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //プロセッサの近くに配置

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ, //初回のResourceState.Textureは基本読むだけ
		nullptr, //Clear最適値。
		IID_PPV_ARGS(&textureResource)
	);
	assert(SUCCEEDED(hr));
	return textureResource;
}
