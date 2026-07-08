#include "CameraCapture.h"

#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "externals/imgui/imgui.h"

#include <mferror.h>

#include <cassert>
#include <cstring>
#include <format>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace TakeC;

namespace {
	void ReleaseActivateArray(IMFActivate** activates, UINT32 count) {
		if (!activates) {
			return;
		}

		for (UINT32 i = 0; i < count; ++i) {
			if (activates[i]) {
				activates[i]->Release();
			}
		}
		CoTaskMemFree(activates);
	}

	std::wstring ReadAllocatedStringAttribute(IMFActivate* activate, REFGUID key) {
		wchar_t* value = nullptr;
		UINT32 length = 0;
		const HRESULT hr = activate->GetAllocatedString(key, &value, &length);
		if (FAILED(hr) || !value) {
			return L"";
		}

		std::wstring result(value, length);
		CoTaskMemFree(value);
		return result;
	}
}

CameraCapture::~CameraCapture() {
	Finalize();
}

//=============================================================
// 初期化
//=============================================================
bool CameraCapture::Initialize() {
	if (initialized_) {
		return true;
	}

	const HRESULT coHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(coHr)) {
		comInitialized_ = true;
	} else if (coHr != RPC_E_CHANGED_MODE) {
		SetLastErrorMessage(L"CoInitializeEx failed.", coHr);
		return false;
	}

	const HRESULT mfHr = MFStartup(MF_VERSION);
	if (FAILED(mfHr)) {
		SetLastErrorMessage(L"MFStartup failed.", mfHr);
		if (comInitialized_) {
			CoUninitialize();
			comInitialized_ = false;
		}
		return false;
	}

	mfStarted_ = true;
	initialized_ = true;
	lastErrorMessage_.clear();
	return true;
}


//=============================================================
// 終了処理
//=============================================================
void CameraCapture::Finalize() {
	CloseDevice();
	ReleaseDisplayResources();
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	displaySrvAllocated_ = false;

	if (mfStarted_) {
		MFShutdown();
		mfStarted_ = false;
	}

	if (comInitialized_) {
		CoUninitialize();
		comInitialized_ = false;
	}

	initialized_ = false;
	lastErrorMessage_.clear();
}

//=============================================================
// ImGui表示用リソースの初期化
//=============================================================
bool CameraCapture::InitializeImGuiResources(DirectXCommon* dxCommon, SrvManager* srvManager) {
	if (!dxCommon || !srvManager) {
		SetLastErrorMessage(L"InitializeImGuiResources received null pointer.");
		return false;
	}

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	return true;
}

//=============================================================
// カメラデバイスの列挙
//=============================================================
std::vector<CameraCapture::DeviceInfo> CameraCapture::EnumerateDevices() const {
	std::vector<DeviceInfo> devices;

	// Media Foundationの初期化がされていない場合は空のリストを返す
	ComPtr<IMFAttributes> attributes;
	HRESULT hr = MFCreateAttributes(&attributes, 1);
	if (FAILED(hr)) {
		return devices;
	}

	// ビデオキャプチャデバイスの列挙
	hr = attributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) {
		return devices;
	}

	IMFActivate** activates = nullptr;
	UINT32 count = 0;
	// デバイスの列挙
	hr = MFEnumDeviceSources(attributes.Get(), &activates, &count);
	if (FAILED(hr)) {
		return devices;
	}

	devices.reserve(count);

	// デバイス情報の取得
	for (UINT32 i = 0; i < count; ++i) {
		DeviceInfo info{};
		// デバイス名とシンボリックリンクを取得
		info.name = ReadAllocatedStringAttribute(activates[i], MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME);
		info.symbolicLink = ReadAllocatedStringAttribute(activates[i], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK);
		devices.push_back(std::move(info));
	}

	// IMFActivateの解放
	ReleaseActivateArray(activates, count);
	return devices;
}

//=============================================================
// カメラデバイスのオープン
//=============================================================
bool CameraCapture::OpenDevice(uint32_t deviceIndex, uint32_t preferredWidth, uint32_t preferredHeight, uint32_t preferredFps) {
	if (!initialized_ && !Initialize()) {
		return false;
	}

	// 既にデバイスがオープンされている場合はクローズ
	CloseDevice();

	ComPtr<IMFAttributes> attributes;
	HRESULT hr = MFCreateAttributes(&attributes, 1);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"MFCreateAttributes failed.", hr);
		return false;
	}

	hr = attributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Set video capture source attribute failed.", hr);
		return false;
	}

	IMFActivate** activates = nullptr;
	UINT32 count = 0;
	hr = MFEnumDeviceSources(attributes.Get(), &activates, &count);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"MFEnumDeviceSources failed.", hr);
		return false;
	}

	if (deviceIndex >= count) {
		ReleaseActivateArray(activates, count);
		SetLastErrorMessage(L"Camera device index is out of range.");
		return false;
	}

	// 選択したデバイスのソースリーダーを作成
	const bool created = CreateSourceReader(activates[deviceIndex]);
	ReleaseActivateArray(activates, count);
	if (!created) {
		return false;
	}

	if (!ConfigureOutputMediaType(preferredWidth, preferredHeight, preferredFps)) {
		CloseDevice();
		return false;
	}

	hasNewFrame_ = false;
	lastErrorMessage_.clear();
	return true;
}

//=============================================================
// カメラデバイスのクローズ
//=============================================================
void CameraCapture::CloseDevice() {
	sourceReader_.Reset();

	if (mediaSource_) {
		mediaSource_->Shutdown();
		mediaSource_.Reset();
	}

	frameRGBA_.clear();
	width_ = 0;
	height_ = 0;
	sourceStride_ = 0;
	hasNewFrame_ = false;
	ReleaseDisplayResources();
}

//=============================================================
// 更新処理
//=============================================================
bool CameraCapture::Update() {
	hasNewFrame_ = false;

	// カメラデバイスがオープンされていない場合はエラー	
	if (!sourceReader_) {
		SetLastErrorMessage(L"Camera device is not open.");
		return false;
	}

	// サンプルの読み込み
	DWORD streamIndex = 0;
	DWORD flags = 0;
	LONGLONG timestamp = 0;
	ComPtr<IMFSample> sample;

	// ReadSampleを呼び出して、カメラからのサンプルを取得
	const HRESULT hr = sourceReader_->ReadSample(
		static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
		0,
		&streamIndex,
		&flags,
		&timestamp,
		&sample);
	static_cast<void>(streamIndex);
	static_cast<void>(timestamp);

	if (FAILED(hr)) {
		// ReadSampleの呼び出しに失敗した場合はエラー
		SetLastErrorMessage(L"ReadSample failed.", hr);
		return false;
	}

	if ((flags & MF_SOURCE_READERF_ERROR) != 0) {
		// ソースリーダーがエラーを返した場合はエラー
		SetLastErrorMessage(L"Source reader returned MF_SOURCE_READERF_ERROR.");
		return false;
	}

	if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
		// ストリームの終端に達した場合はエラー
		SetLastErrorMessage(L"Camera stream reached end of stream.");
		return false;
	}

	if ((flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0) {
		// メディアタイプが変更された場合は、現在のフォーマットを更新
		if (!UpdateCurrentFormat()) {
			return false;
		}
	}

	if (!sample) {
		// サンプルが取得できなかった場合はエラー
		SetLastErrorMessage(L"Failed to read sample from camera.");
		return false;
	}

	// サンプルからフレームデータをコピー
	return CopySampleToFrame(sample.Get());
}

//=============================================================
// ImGui更新処理
//=============================================================
void CameraCapture::UpdateImGui(const char* windowName) {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::Begin(windowName ? windowName : "Camera Capture");

	// カメラデバイスの列挙
	const std::vector<DeviceInfo> devices = EnumerateDevices();
	ImGui::Text("Device Count: %d", static_cast<int>(devices.size()));

	// カメラデバイスのオープンボタンとデバイス名の表示
	for (size_t i = 0; i < devices.size(); ++i) {
		const std::string buttonLabel = "Open##CameraDevice" + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str())) {
			OpenDevice(static_cast<uint32_t>(i));
		}
		ImGui::SameLine();
		ImGui::Text("%ls", devices[i].name.c_str());
	}

	if (IsOpen()) {
		if (ImGui::Button("Close Camera")) {
			CloseDevice();
		}

		if (HasNewFrame()) {
			UploadFrameToTexture();
		}

		ImGui::Text("Frame: %u x %u", width_, height_);

		if (displayTexture_ && displaySrvAllocated_ && srvManager_) {
			const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvManager_->GetSrvDescriptorHandleGPU(displaySrvIndex_);
			const float maxWidth = ImGui::GetContentRegionAvail().x;
			const float aspect = width_ != 0 ? static_cast<float>(height_) / static_cast<float>(width_) : 1.0f;
			ImGui::Image(
				reinterpret_cast<ImTextureID>(gpuHandle.ptr),
				ImVec2(maxWidth, maxWidth * aspect));
		} else {
			ImGui::TextUnformatted("Display texture is not ready.");
		}
	} else {
		ImGui::TextUnformatted("Camera is not open.");
	}

	if (!lastErrorMessage_.empty()) {
		ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f), "%ls", lastErrorMessage_.c_str());
	}

	ImGui::End();
#else
	static_cast<void>(windowName);
#endif
}

//=============================================================
// 最後のエラーメッセージを設定
//=============================================================
void CameraCapture::SetLastErrorMessage(const wchar_t* message) {
	lastErrorMessage_ = message ? message : L"";
}

//=============================================================
// 最後のエラーメッセージを設定（HRESULT付き）
//=============================================================
void CameraCapture::SetLastErrorMessage(const wchar_t* message, HRESULT hr) {
	lastErrorMessage_ = std::format(L"{} HRESULT=0x{:08X}", message ? message : L"", static_cast<uint32_t>(hr));
}

//=============================================================
// カメラデバイスのソースリーダーを作成
//=============================================================
bool CameraCapture::CreateSourceReader(IMFActivate* activate) {
	assert(activate);

	HRESULT hr = activate->ActivateObject(IID_PPV_ARGS(&mediaSource_));
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Activate camera media source failed.", hr);
		return false;
	}

	ComPtr<IMFAttributes> readerAttributes;
	hr = MFCreateAttributes(&readerAttributes, 1);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"MFCreateAttributes for source reader failed.", hr);
		return false;
	}

	// Enables Media Foundation conversion to RGB32 when the camera outputs YUY2/NV12/MJPG.
	hr = readerAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Set source reader video processing attribute failed.", hr);
		return false;
	}

	hr = MFCreateSourceReaderFromMediaSource(mediaSource_.Get(), readerAttributes.Get(), &sourceReader_);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"MFCreateSourceReaderFromMediaSource failed.", hr);
		return false;
	}

	hr = sourceReader_->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Disable source reader streams failed.", hr);
		return false;
	}

	hr = sourceReader_->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Enable video stream failed.", hr);
		return false;
	}

	return true;
}

//=============================================================
// 出力メディアタイプを設定
//=============================================================
bool CameraCapture::ConfigureOutputMediaType(uint32_t preferredWidth, uint32_t preferredHeight, uint32_t preferredFps) {
	assert(sourceReader_);

	ComPtr<IMFMediaType> mediaType;
	HRESULT hr = MFCreateMediaType(&mediaType);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"MFCreateMediaType failed.", hr);
		return false;
	}

	hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Set MF_MT_MAJOR_TYPE failed.", hr);
		return false;
	}

	hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Set MF_MT_SUBTYPE RGB32 failed.", hr);
		return false;
	}

	if (preferredWidth != 0 && preferredHeight != 0) {
		MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, preferredWidth, preferredHeight);
	}
	if (preferredFps != 0) {
		MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, preferredFps, 1);
	}

	hr = sourceReader_->SetCurrentMediaType(
		static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
		nullptr,
		mediaType.Get());

	if (FAILED(hr)) {
		ComPtr<IMFMediaType> fallbackType;
		HRESULT fallbackHr = MFCreateMediaType(&fallbackType);
		if (SUCCEEDED(fallbackHr)) {
			fallbackHr = fallbackType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		}
		if (SUCCEEDED(fallbackHr)) {
			fallbackHr = fallbackType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
		}
		if (SUCCEEDED(fallbackHr)) {
			fallbackHr = sourceReader_->SetCurrentMediaType(
				static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
				nullptr,
				fallbackType.Get());
		}
		if (FAILED(fallbackHr)) {
			SetLastErrorMessage(L"Set source reader RGB32 media type failed.", hr);
			return false;
		}
	}

	return UpdateCurrentFormat();
}


//=============================================================
// 現在のフレームフォーマットを更新
//=============================================================
bool CameraCapture::UpdateCurrentFormat() {
	assert(sourceReader_);

	ComPtr<IMFMediaType> currentType;
	// 現在のメディアタイプを取得
	HRESULT hr = sourceReader_->GetCurrentMediaType(
		static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
		&currentType);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"GetCurrentMediaType failed.", hr);
		return false;
	}

	UINT32 width = 0;
	UINT32 height = 0;
	// フレームサイズを取得
	hr = MFGetAttributeSize(currentType.Get(), MF_MT_FRAME_SIZE, &width, &height);
	if (FAILED(hr) || width == 0 || height == 0) {
		SetLastErrorMessage(L"Get camera frame size failed.", hr);
		return false;
	}

	UINT32 strideValue = 0;
	LONG stride = 0;
	// フレームの1行のバイト数を取得
	hr = currentType->GetUINT32(MF_MT_DEFAULT_STRIDE, &strideValue);
	if (SUCCEEDED(hr)) {
		// strideValueが取得できた場合はそれを使用
		stride = static_cast<LONG>(strideValue);
	} else {
		stride = static_cast<LONG>(width * 4);
	}

	width_ = width;
	height_ = height;
	sourceStride_ = stride;
	// RGBA形式のフレームデータ用のバッファを確保
	frameRGBA_.resize(static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4u);
	return true;
}

//=============================================================
// ImGui表示用リソースを確保
//=============================================================
bool CameraCapture::EnsureDisplayResources() {
	if (!dxCommon_ || !srvManager_) {
		SetLastErrorMessage(L"Display resources are not initialized.");
		return false;
	}

	if (width_ == 0 || height_ == 0 || frameRGBA_.empty()) {
		return false;
	}

	if (displayTexture_ && uploadBuffer_ && displayWidth_ == width_ && displayHeight_ == height_) {
		return true;
	}

	ReleaseDisplayResources();

	displayWidth_ = width_;
	displayHeight_ = height_;

	D3D12_RESOURCE_DESC textureDesc{};
	textureDesc.Width = displayWidth_;
	textureDesc.Height = displayHeight_;
	textureDesc.MipLevels = 1;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES defaultHeapProperties{};
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&displayTexture_));
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Camera display texture creation failed.", hr);
		ReleaseDisplayResources();
		return false;
	}
	displayTextureState_ = D3D12_RESOURCE_STATE_GENERIC_READ;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT numRows = 0;
	UINT64 rowSizeInBytes = 0;
	UINT64 totalBytes = 0;
	dxCommon_->GetDevice()->GetCopyableFootprints(
		&textureDesc,
		0,
		1,
		0,
		&footprint,
		&numRows,
		&rowSizeInBytes,
		&totalBytes);
	static_cast<void>(numRows);
	static_cast<void>(rowSizeInBytes);

	uploadBufferRowPitch_ = footprint.Footprint.RowPitch;
	uploadBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), static_cast<size_t>(totalBytes));

	hr = uploadBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedUploadData_));
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Camera upload buffer map failed.", hr);
		ReleaseDisplayResources();
		return false;
	}

	if (!displaySrvAllocated_) {
		displaySrvIndex_ = srvManager_->Allocate();
		displaySrvAllocated_ = true;
	}

	srvManager_->CreateSRVforTexture2D(false, DXGI_FORMAT_R8G8B8A8_UNORM, 1, displayTexture_.Get(), displaySrvIndex_);
	return true;
}

//=============================================================
// 最新フレームをGPUテクスチャへアップロード
//=============================================================
bool CameraCapture::UploadFrameToTexture() {
	if (!EnsureDisplayResources()) {
		return false;
	}
	// RGBA形式のフレームデータをアップロードバッファへコピー
	const uint64_t srcRowPitch = static_cast<uint64_t>(width_) * 4u;
	// アップロードバッファの1行のバイト数がフレームの1行のバイト数より大きい場合は、余分な部分をスキップしてコピー
	for (uint32_t y = 0; y < height_; ++y) {
		const uint8_t* src = frameRGBA_.data() + static_cast<size_t>(y) * static_cast<size_t>(srcRowPitch);
		uint8_t* dst = mappedUploadData_ + static_cast<size_t>(y) * static_cast<size_t>(uploadBufferRowPitch_);
		std::memcpy(dst, src, static_cast<size_t>(srcRowPitch));
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	if (displayTextureState_ != D3D12_RESOURCE_STATE_COPY_DEST) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = displayTexture_.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = displayTextureState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		commandList->ResourceBarrier(1, &barrier);
		displayTextureState_ = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	D3D12_TEXTURE_COPY_LOCATION dstLocation{};
	dstLocation.pResource = displayTexture_.Get();
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION srcLocation{};
	srcLocation.pResource = uploadBuffer_.Get();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint.Offset = 0;
	srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srcLocation.PlacedFootprint.Footprint.Width = displayWidth_;
	srcLocation.PlacedFootprint.Footprint.Height = displayHeight_;
	srcLocation.PlacedFootprint.Footprint.Depth = 1;
	srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(uploadBufferRowPitch_);

	commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = displayTexture_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList->ResourceBarrier(1, &barrier);
	displayTextureState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	return true;
}

//=============================================================
// ImGui表示用リソースを解放
//=============================================================
void CameraCapture::ReleaseDisplayResources() {
	if (uploadBuffer_) {
		uploadBuffer_->Unmap(0, nullptr);
	}

	mappedUploadData_ = nullptr;
	uploadBuffer_.Reset();
	displayTexture_.Reset();
	displayWidth_ = 0;
	displayHeight_ = 0;
	uploadBufferRowPitch_ = 0;
	displayTextureState_ = D3D12_RESOURCE_STATE_GENERIC_READ;
}

//=============================================================
// IMFSampleからフレームデータをコピー
//=============================================================
bool CameraCapture::CopySampleToFrame(IMFSample* sample) {
	assert(sample);

	ComPtr<IMFMediaBuffer> buffer;
	// サンプルを連続したバッファに変換
	HRESULT hr = sample->ConvertToContiguousBuffer(&buffer);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"ConvertToContiguousBuffer failed.", hr);
		return false;
	}

	BYTE* srcData = nullptr;
	DWORD maxLength = 0;
	DWORD currentLength = 0;
	// バッファをロックして、フレームデータへのポインタを取得
	hr = buffer->Lock(&srcData, &maxLength, &currentLength);
	if (FAILED(hr)) {
		SetLastErrorMessage(L"Media buffer lock failed.", hr);
		return false;
	}
	static_cast<void>(maxLength);
	static_cast<void>(currentLength);

	// フレームフォーマットが有効かどうかをチェック
	if (width_ == 0 || height_ == 0 || sourceStride_ == 0) {
		buffer->Unlock();
		SetLastErrorMessage(L"Camera frame format is invalid.");
		return false;
	}

	// sourceStride_の絶対値を計算
	const LONG absStride = sourceStride_ < 0 ? -sourceStride_ : sourceStride_;
	if (absStride < static_cast<LONG>(width_ * 4)) {
		buffer->Unlock();
		SetLastErrorMessage(L"Camera frame stride is invalid.");
		return false;
	}

	const BYTE* firstRow = srcData;
	if (sourceStride_ < 0) {
		firstRow = srcData + static_cast<size_t>(absStride) * static_cast<size_t>(height_ - 1);
	}

	// RGB32形式のフレームデータをRGBA形式に変換してコピー
	for (uint32_t y = 0; y < height_; ++y) {
		const BYTE* srcRow = firstRow + static_cast<ptrdiff_t>(sourceStride_) * static_cast<ptrdiff_t>(y);
		uint8_t* dstRow = frameRGBA_.data() + static_cast<size_t>(y) * static_cast<size_t>(width_) * 4u;

		for (uint32_t x = 0; x < width_; ++x) {
			const BYTE* src = srcRow + static_cast<size_t>(x) * 4u;
			uint8_t* dst = dstRow + static_cast<size_t>(x) * 4u;
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = 255;
		}
	}

	buffer->Unlock();
	hasNewFrame_ = true;
	lastErrorMessage_.clear();
	return true;
}
