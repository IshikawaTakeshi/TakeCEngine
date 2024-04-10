#include "DirectXCommon.h"

#include "Logger.h"

#include <format>

DirectXCommon* DirectXCommon::GetInstance() {
	static DirectXCommon instance;
	return &instance;
}

void DirectXCommon::Initialize() {

	//DXGIデバイス初期化
	InitializeDXGIDevice();

	// コマンド関連初期化
	InitializeCommand();

	// スワップチェーンの生成
	CreateSwapChain();

	// レンダーターゲット生成
	CreateFinalRenderTargets();

	// フェンス生成
	CreateFence();
}

void DirectXCommon::PreDraw() {

	//全画面クリア
	ClearRenderTarget();

}

void DirectXCommon::PostDraw() {
	HRESULT result;

	//画面に描く処理はすべて終わり、画面に移すので、状態を遷移
	//今回はRenderTargetからPresentにする
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);

	// コマンドリストの内容を確定
	result = commandList_->Close();
	assert(SUCCEEDED(result));
	
	// コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { commandList_.Get() }; // コマンドリストの配列
	commandQueue_->ExecuteCommandLists(1, cmdLists);

	//GPUとOSに画面の交換を行うよう通知
	swapChain_->Present(1, 0);

	//Fenceの値の更新
	fenceVal_++;
	//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_.Get(), fenceVal_);

	//Fenceの値が指定したSignal値にたどり着いているか確認する
	//GetComleteValueの初期値作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceVal_) {
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
		//イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	//次のフレーム用のコマンドリストを準備
	result = commandAllocator_->Reset();
	assert(SUCCEEDED(result));
	result = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(result));
}

void DirectXCommon::ClearRenderTarget() {

	//書き込むバックバッファのインデックスを取得
	UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();


	
	//今回のバリアはTransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier_.Transition.pResource = swapChainResources_[bbIndex].Get();
	//遷移前(現在)のResourceState
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);



	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandles_[bbIndex], false, nullptr);

	// 全画面クリア          Red   Green  Blue  Alpha
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 青っぽい色
	commandList_->ClearRenderTargetView(rtvHandles_[bbIndex], clearColor, 0, nullptr);

}

void DirectXCommon::InitializeDXGIDevice() {

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = {
		"12.2",
		"12.1",
		"12.0"
	};

	//////////////////////////////////////////////////////////
	//DXGIファクトリーの作成
	//////////////////////////////////////////////////////////


	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	//////////////////////////////////////////////////////////
	//使用するアダプタ(GPU)の決定
	//////////////////////////////////////////////////////////

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	useAdapter_ = nullptr;

	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプタの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); //取得できないのは一大事
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(Logger::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr; //ソフトウェアアダプタの場合は見なかったことにする
	}

	//適切なアダプタが見つからなかった場合は起動できない
	assert(useAdapter_ != nullptr);

	//////////////////////////////////////////////////////////
	//D3D12Deviceの生成
	//////////////////////////////////////////////////////////

	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		//採用したアダプタでデバイスを生成
		hr = D3D12CreateDevice(useAdapter_, featureLevels[i], IID_PPV_ARGS(&device_));

		//指定した機能レベルでデバイスが生成出来たかを確認
		if (SUCCEEDED(hr)) {
			//生成出来た場合はログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	//デバイスの生成がうまくいかなかった場合は起動できない
	assert(device_ != nullptr);

	//初期化完了のログを出す
	Logger::Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//ヤバいエラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデッバグレイヤーとDX12デッバグレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		//解放
		infoQueue->Release();
	}
#endif // _DEBUG

}

void DirectXCommon::InitializeCommand() {
	HRESULT result = S_FALSE;

	// コマンドアロケータを生成
	result = device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));

	// コマンドリストを生成
	result = device_->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(&commandList_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));

	// コマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	result = device_->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));
}

void DirectXCommon::CreateSwapChain() {

	HRESULT result = S_FALSE;

	// 各種設定をしてスワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 色情報の書式
	swapChainDesc.SampleDesc.Count = 1;                 // マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // バックバッファとして使えるように
	swapChainDesc.BufferCount = 2;                      // バッファ数を２つに設定(ダブルバッファ)
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに映したら(フリップ後)は速やかに破棄

	result = dxgiFactory_->CreateSwapChainForHwnd(
		commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(result));
}

void DirectXCommon::CreateFinalRenderTargets() {
	HRESULT result = S_FALSE;

	// 各種設定をしてディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー
	heapDesc.NumDescriptors = 2; //ダブルバッファ用に2つ。
	result = device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap_));
	assert(SUCCEEDED(result));

	//SwapChainからResourceを引っ張ってくる
	for (int i = 0; i < 2; i++){
		swapChainResources_[i] = { nullptr };
	}
	
	result = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	//取得できなければ起動できない
	assert(SUCCEEDED(result));
	result = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(result));

	// レンダーターゲットビューの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	//1つ目。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(
		swapChainResources_[0].Get(),
		&rtvDesc,
		rtvHandles_[0]
	);
	//2つ目のディスクリプタハンドルを得る
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV
	);
	//2つ目。
	device_->CreateRenderTargetView(
		swapChainResources_[1].Get(),
		&rtvDesc,
		rtvHandles_[1]
	);
}

void DirectXCommon::CreateFence() {
	HRESULT result = S_FALSE;
	// フェンスの生成
	result = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(result));

	//FenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}
