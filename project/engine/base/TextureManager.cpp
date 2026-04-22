#include "TextureManager.h"
#include "base/SrvManager.h"
#include "Utility/StringUtility.h"
#include "Utility/Logger.h"
#include "math/MathEnv.h"
#include "TransformMatrix.h"
#include <cassert>
#include <filesystem>
#include <chrono>

//================================================================
// シングルトンインスタンスの取得
//================================================================
TakeC::TextureManager& TakeC::TextureManager::GetInstance() {
	static TextureManager instance;
	return instance;
}

//================================================================
//			初期化
//================================================================
void TakeC::TextureManager::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {


	//dxCommon_の取得
	dxCommon_ = dxCommon;
	//SRVManagerの取得
	srvManager_ = srvManager;
	//SRVの数と同数
	textureDatas_.reserve(srvManager_->kMaxSRVCount_);

	// --- プレースホルダ（白テクスチャ）の生成 ---
	DirectX::ScratchImage image;
	DirectX::TexMetadata metadata;
	DecodeTexture("", image, metadata); // 空パスで白1x1が読み込まれる

	DirectX::ScratchImage mipImages;
	BuildMipMaps(image, metadata, mipImages);

	whiteTextureData_.srvIndex = srvManager_->Allocate();
	whiteTextureData_.srvHandleCPU = srvManager_->GetSrvDescriptorHandleCPU(whiteTextureData_.srvIndex);
	whiteTextureData_.srvHandleGPU = srvManager_->GetSrvDescriptorHandleGPU(whiteTextureData_.srvIndex);

	UploadTexture(mipImages, metadata, whiteTextureData_);

	srvManager_->CreateSRVforTexture2D(
		whiteTextureData_.metadata.IsCubemap(),
		whiteTextureData_.metadata.format,
		UINT(whiteTextureData_.metadata.mipLevels),
		whiteTextureData_.resource.Get(),
		whiteTextureData_.srvIndex
	);

	//workerスレッド開始
	threadRunning_ = true;
	StartWorker();
}

//================================================================
//			終了処理
//================================================================
void TakeC::TextureManager::Finalize() {
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
	threadRunning_ = false;
	
	for (auto& thread : workerThreads_) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	workerThreads_.clear();
	textureDatas_.clear();

	// 白テクスチャのリソース解放
	if (whiteTextureData_.resource) {
		whiteTextureData_.resource.Reset();
	}
	// 中間リソースの解放
	if (whiteTextureData_.intermediateResource) {
		whiteTextureData_.intermediateResource.Reset();
	}
}

//================================================================
// 更新処理
//================================================================
void TakeC::TextureManager::Update() {

	while (true) {

		TextureCPUData data;
		{
			std::lock_guard lock(mutex_);
			if (completedQueue_.empty()) break;

			data = std::move(completedQueue_.front());
			completedQueue_.pop();
		}

		// GPUアップロード
		{
			std::lock_guard<std::mutex> mapLock(mapMutex_);
			TextureData& textureData = textureDatas_[data.filePath];

			UploadTexture(data.mipImages, data.metadata, textureData);

			// SRV処理
			if (textureData.srvIndex == 0) {
				textureData.srvIndex = srvManager_->Allocate();
				textureData.srvHandleCPU = srvManager_->GetSrvDescriptorHandleCPU(textureData.srvIndex);
				textureData.srvHandleGPU = srvManager_->GetSrvDescriptorHandleGPU(textureData.srvIndex);
			}

			srvManager_->CreateSRVforTexture2D(
				textureData.metadata.IsCubemap(),
				textureData.metadata.format,
				UINT(textureData.metadata.mipLevels),
				textureData.resource.Get(),
				textureData.srvIndex
			);
		}

		// 読み込み完了
		pendingCount_--;
	}
}

//=============================================================================================
///			テクスチャの更新チェック
//=============================================================================================

void TakeC::TextureManager::CheckAndReloadTextures() {

	//テクスチャの更新チェック
	for (auto& [filePath, textureData] : textureDatas_) {
		std::string fullPath = "Resources/Images/" + filePath;
		//ファイルの最終更新日時を取得
		time_t newTime = GetFileLastWriteTime(fullPath);

		if (fileUpdateTimes_[filePath] != newTime) {
			//更新されていたら再読み込み
			LoadTexture(filePath, true);
			//更新日時を更新
			fileUpdateTimes_[filePath] = newTime;
		}
	}
}

//=============================================================================================
///			テクスチャの読み込み
//=============================================================================================
void TakeC::TextureManager::LoadTexture(const std::string& filePath, bool forceReload) {

	std::string normalizedPath = NormalizeTextureFilePath(filePath);

	{
		std::lock_guard<std::mutex> lock(mapMutex_);
		if (!forceReload && textureDatas_.contains(normalizedPath)) return;

		// エントリがなければ仮登録
		if (!textureDatas_.contains(normalizedPath)) {
			TextureData& data = textureDatas_[normalizedPath];
			data.srvIndex = srvManager_->Allocate();
			data.srvHandleCPU = srvManager_->GetSrvDescriptorHandleCPU(data.srvIndex);
			data.srvHandleGPU = srvManager_->GetSrvDescriptorHandleGPU(data.srvIndex);

			// とりあえず白テクスチャを割り当て
			srvManager_->CreateSRVforTexture2D(
				false,
				whiteTextureData_.metadata.format,
				1,
				whiteTextureData_.resource.Get(),
				data.srvIndex
			);
		}
	}

	{
		std::lock_guard<std::mutex> lock(mutex_);
		requestQueue_.push(normalizedPath);
		pendingCount_++;
	}
}

//=============================================================================================
///			全テクスチャの読み込み
//=============================================================================================
void TakeC::TextureManager::LoadTextureAll() {

	//Resources/imagesフォルダ内の全ての画像ファイルを読み込む
	namespace fs = std::filesystem;
	std::string directoryPath = "Resources/Images/";

	if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
		assert(false && "ディレクトリが存在しないか、ディレクトリではありません");
		return;
	}

	// 先に対象ファイル一覧を作る（将来の並列化にも流用しやすい）
	std::vector<fs::path> targets;
	targets.reserve(1024);

	for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		if (!IsSupportedTextureExt(entry.path().extension().string())) {
			continue;
		}
		// テクスチャファイルのパスを保存
		targets.push_back(entry.path());
	}

	for (const auto& path : targets) {
		fs::path relative = fs::relative(path, directoryPath);
		std::string normalizedPath = relative.generic_string();

		LoadTexture(normalizedPath, false);
		fileUpdateTimes_[normalizedPath] = GetFileLastWriteTime(path.string());
	}

	// すべての読み込みが終わるまで待機
	WaitForAllLoads();
}

void TakeC::TextureManager::WaitForAllLoads() {
	while (pendingCount_ > 0) {
		Update();
		std::this_thread::yield();
	}
}

//=============================================================================================
///			テクスチャファイルパスの正規化
//=============================================================================================
std::string TakeC::TextureManager::NormalizeTextureFilePath(const std::string& filePath) {
	namespace fs = std::filesystem;

	// すでに"Resources/Images/"を含む場合は相対に変換
	const fs::path base = "Resources/Images";
	fs::path p = fs::path(filePath);

	// もしfilePathが絶対"Resources/Images"を含むなら相対化
	if (p.is_absolute() || filePath.find("Resources/Images/") != std::string::npos) {
		p = fs::relative(p, base); //ui/foo.pngなど
	}

	return p.generic_string();
}

///=================================================================================================
///			テクスチャリソースの作成
//==================================================================================================
Microsoft::WRL::ComPtr<ID3D12Resource> TakeC::TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {

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

//============================================================================================
//			テクスチャデータのアップロード
//============================================================================================

ComPtr<ID3D12Resource> TakeC::TextureManager::UploadTextureData(
	ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//サブリソースの作成
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(
		dxCommon_->GetDevice(),
		mipImages.GetImages(),
		mipImages.GetImageCount(),
		mipImages.GetMetadata(),
		subresources);

	//中間リソースのサイズの取得
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));

	//中間リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;
	intermediateResource = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), intermediateSize);

	//中間リソースにデータをコピー
	UpdateSubresources(dxCommon_->GetCommandList(), texture.Get(),
		intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());

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

//============================================================================================
///			テクスチャのSRVハンドルを取得
//============================================================================================

D3D12_GPU_DESCRIPTOR_HANDLE TakeC::TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	std::lock_guard<std::mutex> lock(mapMutex_);
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).srvHandleGPU;
}

//============================================================================================
// テクスチャのSRVハンドルを取得
//============================================================================================

uint32_t TakeC::TextureManager::GetSrvIndex(const std::string& filePath) {

	std::lock_guard<std::mutex> lock(mapMutex_);
	//読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return textureDatas_.at(filePath).srvIndex;
	}

	assert(false);
	return 0;
}

//============================================================================================
// テクスチャのメタデータを取得
//============================================================================================

const DirectX::TexMetadata& TakeC::TextureManager::GetMetadata(const std::string& filePath) {
	std::lock_guard<std::mutex> lock(mapMutex_);
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).metadata;
}

//============================================================================================
// 読み込んだテクスチャのファイル名を取得
//============================================================================================

std::vector<std::string> TakeC::TextureManager::GetLoadedTextureFileNames() const {

	std::vector<std::string> fileNames;
	{
		std::lock_guard<std::mutex> lock(mapMutex_); // Requires mutable mutex
		for (const auto& pair : textureDatas_) {
			fileNames.push_back(pair.first);
		}
	}
	return fileNames;
}

//============================================================================================
// CubeMapを除いたテクスチャのファイル名を取得
//============================================================================================
std::vector<std::string> TakeC::TextureManager::GetLoadedNonCubeTextureFileNames() const {
	std::vector<std::string> fileNames;
	{
		std::lock_guard<std::mutex> lock(mapMutex_); // Requires mutable mutex
		for (const auto& pair : textureDatas_) {
			if (!pair.second.metadata.IsCubemap()) {
				fileNames.push_back(pair.first);
			}
		}
	}
	return fileNames;
}

//============================================================================================
//			ファイルの最終更新日時を取得
//============================================================================================

time_t TakeC::TextureManager::GetFileLastWriteTime(const std::string& filePath) {
	namespace fs = std::filesystem;
	namespace chrono = std::chrono;
	try {
		if (!fs::exists(filePath)) {
			return 0;
		}

		//ファイルの最終更新日時を取得
		auto ftime = fs::last_write_time(filePath);
		// std::filesystem::file_time_typeはクロックの型なので、system_clockに変換
		auto sctp = chrono::clock_cast<chrono::system_clock>(ftime);
		return chrono::system_clock::to_time_t(sctp);


	}
	catch (...) {
		// エラー処理
		assert(false && "ファイルの最終更新日時の取得に失敗しました。");
		return 0;
	}
}

//============================================================================================
//  テクスチャファイルの拡張子がサポートされているかチェック
//============================================================================================
bool TakeC::TextureManager::IsSupportedTextureExt(const std::string& ext) {
	const std::string e = StringUtility::ToLowerCopy(ext);
	return e == ".png" || e == ".jpg" || e == ".dds";
}

//============================================================================================
// 			テクスチャファイルのデコード
//============================================================================================
bool TakeC::TextureManager::DecodeTexture(const std::string& filePath, DirectX::ScratchImage& outImage, DirectX::TexMetadata& outMetadata) {
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	std::wstring fullPathW = L"Resources/Images/" + filePathW;
	HRESULT hr;

	if (filePathW.empty()) {
		hr = DirectX::LoadFromWICFile(L"Resources/Images/white1x1.png", DirectX::WIC_FLAGS_FORCE_SRGB, &outMetadata, outImage);
	}
	else if (filePathW.ends_with(L".dds")) {
		// "dds/" が含まれていなければ
		if (filePathW.find(L"dds/") == std::wstring::npos && filePathW.find(L"dds\\") == std::wstring::npos) {
			filePathW = L"dds/" + filePathW;
		}
		fullPathW = L"Resources/Images/" + filePathW;
		hr = DirectX::LoadFromDDSFile(fullPathW.c_str(), DirectX::DDS_FLAGS_NONE, &outMetadata, outImage);
	}
	else {
		hr = DirectX::LoadFromWICFile(fullPathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, &outMetadata, outImage);
	}
	return SUCCEEDED(hr);
}
//============================================================================================
// ミップ生成 
//============================================================================================
bool TakeC::TextureManager::BuildMipMaps(const DirectX::ScratchImage& srcImage, const DirectX::TexMetadata& metadata, DirectX::ScratchImage& outMipImage) {
	// 圧縮 or 1x1の場合はスキップ
	if (DirectX::IsCompressed(metadata.format) || (metadata.width == 1 && metadata.height == 1)) {
		outMipImage = std::move(const_cast<DirectX::ScratchImage&>(srcImage));
		return true;
	}
	HRESULT hr = DirectX::GenerateMipMaps(
		srcImage.GetImages(),
		srcImage.GetImageCount(),
		metadata,
		DirectX::TEX_FILTER_SRGB,
		0, outMipImage
	);
	return SUCCEEDED(hr);
}

//============================================================================================
// GPUアップロード
// metadataはミップ画像のものに更新される
//============================================================================================
Microsoft::WRL::ComPtr<ID3D12Resource> TakeC::TextureManager::UploadTexture(
	const DirectX::ScratchImage& mipImages, DirectX::TexMetadata& metadata, TextureData& textureData) {
	textureData.metadata = mipImages.GetMetadata();
	metadata = textureData.metadata;

	// リソース作成
	textureData.resource = CreateTextureResource(metadata);
	textureData.intermediateResource = UploadTextureData(textureData.resource, mipImages);
	// SRVアロケートはLoadTexture内で
	return textureData.resource;
}

void TakeC::TextureManager::StartWorker() {
	// スレッド数を決定（CPUコア数 - 1、最低1）
	uint32_t numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
	// あまり多すぎてもオーバーヘッドになるため上限を設ける
	numThreads = std::min(numThreads, 4u);

	for (uint32_t i = 0; i < numThreads; ++i) {
		workerThreads_.emplace_back([this]() { WorkerLoop(); });
	}
}

void TakeC::TextureManager::WorkerLoop() {
	while (threadRunning_) {
		std::string path;
		bool isEmpty = false;

		{
			std::lock_guard lock(mutex_);
			isEmpty = requestQueue_.empty();
			if (!isEmpty) {
				path = requestQueue_.front();
				requestQueue_.pop();
			}
		}

		if (isEmpty) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		// --- CPU処理 ---
		DirectX::ScratchImage image;
		DirectX::TexMetadata metadata;
		DecodeTexture(path, image, metadata);

		DirectX::ScratchImage mipImages;
		BuildMipMaps(image, metadata, mipImages);

		TextureCPUData data;
		data.filePath = path;
		data.mipImages = std::move(mipImages);
		data.metadata = metadata;

		{
			std::lock_guard lock(mutex_);
			completedQueue_.push(std::move(data));
		}
	}
}
