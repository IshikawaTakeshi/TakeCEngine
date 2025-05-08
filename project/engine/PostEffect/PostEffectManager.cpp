#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/GrayScale.h"
#include "PostEffect/Vignette.h"

//====================================================================
//	初期化
//====================================================================

void PostEffectManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//RTVManagerの取得
	rtvManager_ = dxCommon_->GetRtvManager();

	//rtvIndexの取得	
	rtvIndex_ = rtvManager_->Allocate();
	//RTVのハンドルを取得
	rtvHandle_ = rtvManager_->GetRtvDescriptorHandleCPU(rtvIndex_);
	//rtvDescの初期化
	rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	//RenderTextureResourceの生成
	renderTextureResource_ = dxCommon_->CreateRenderTextureResource(
		dxCommon_->GetDevice(),WinApp::kClientWidth,WinApp::kClientHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,kRenderTargetClearColor_);
	renderTextureResource_->SetName(L"renderTextureResource_");

	//RTVの生成
	dxCommon_->GetDevice()->CreateRenderTargetView(renderTextureResource_.Get(), &rtvDesc_, rtvHandle_);

	//SRVの生成
	renderTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(renderTextureResource_.Get(),  renderTextureSrvIndex_);

	//PSO初期化
	renderTexturePSO_ = std::make_unique<PSO>();
	renderTexturePSO_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/PostEffect/FullScreen.VS.hlsl");
	renderTexturePSO_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/PostEffect/CopyImage.PS.hlsl");
	renderTexturePSO_->CreateRenderTexturePSO(dxCommon_->GetDevice());

	rootSignature_ = renderTexturePSO_->GetGraphicRootSignature();

	AddEffect("grayScale", L"Resources/shaders/PostEffect/GrayScale.CS.hlsl");
	AddEffect("vignette", L"Resources/shaders/PostEffect/Vignette.CS.hlsl");
}

void PostEffectManager::UpdateImGui() {

	for (auto& postEffect : postEffects_) {
		postEffect.second->UpdateImGui();
	}
}

//====================================================================
// 終了処理
//====================================================================

void PostEffectManager::Finalize() {
	renderTextureResource_.Reset();
	renderTexturePSO_.reset();
	rootSignature_.Reset();
	postEffects_.clear();
	rtvManager_ = nullptr;
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

//====================================================================
// 描画前処理
//====================================================================


void PostEffectManager::ClearRenderTarget() {
	//深度ステンシルビューの設定
	dsvHandle_ = dxCommon_->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	//描画先のRTVとDSVを設定する
	dxCommon_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);
	// 全画面クリア
	dxCommon_->GetCommandList()->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);
	//指定した深度で画面全体をクリアにする
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// Viewportを設定
	dxCommon_->GetCommandList()->RSSetViewports(1, &dxCommon_->GetViewport());
	// Scissorの設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &dxCommon_->GetScissorRect());
}
void PostEffectManager::PreDraw() {

	
}

//======================================================================
// ComputeShaderによる全PostEffectの処理
//======================================================================

void PostEffectManager::AllDispatch() {

	// RENDER_TARGET >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	for (auto& postEffect : postEffects_) {
		postEffect.second->DisPatch();
	}

	//PIXCEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}

//======================================================================
// 描画処理
//======================================================================

void PostEffectManager::Draw() {

	// RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	//コンテナの末尾のリソース状態を変更する
	//NON_PIXEL_SHADER_RESOURCE >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		postEffects_.rbegin()->second->GetOutputTextureResource().Get());

	// ルートシグネチャの設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	// PSOの設定
	dxCommon_->GetCommandList()->SetPipelineState(renderTexturePSO_->GetGraphicPipelineState());
	// プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// gTexture
	srvManager_->SetGraphicsRootDescriptorTable(renderTexturePSO_->GetGraphicBindResourceIndex("gTexture"),postEffects_.rbegin()->second->GetOutputTextureUavIndex());
	
	// 描画コマンドを発行
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}


//======================================================================
// 描画後処理
//======================================================================

void PostEffectManager::PostDraw() {

	//PIXCEL_SHADER_RESOURCE >> GENERIC_READ
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());

	//コンテナの全ての出力リソース状態を変更する
	//PIXEL_SHADER_RESOURCE >> NON_PIXEL_SHADER_RESOURCE
	for (auto& postEffect : postEffects_) {
		ResourceBarrier::GetInstance()->Transition(
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			postEffect.second->GetOutputTextureResource().Get());
	}
}

void PostEffectManager::AddEffect(const std::string& name, const std::wstring& csFilePath) {

	// 読み込み済みかどうか検索
	if (postEffects_.contains(name)) {
		Logger::Log("PostEffectManager::AddEffect() : PostEffect is already loaded.");
		return;
	}

	std::unique_ptr<PostEffect> postEffect;

	// PostEffectの初期化
	if (name == "grayScale") {
		postEffect = std::make_unique<GrayScale>();
	} else if (name == "vignette") {
		postEffect = std::make_unique<Vignette>();
	} else {
		Logger::Log("PostEffectManager::AddEffect() : PostEffect is not found.");
		return;
	}

	// 入出力テクスチャを決定（Ping-Pong切り替え）
	ComPtr<ID3D12Resource> inputResource = nullptr;
	UINT inputSrvIndex = 0;
	ComPtr<ID3D12Resource> outputResource = nullptr;
	UINT outputSrvIndex = 0;

	if (postEffects_.empty()) {
		// 最初の入力は通常の描画結果（renderTextureResource_）
		inputResource = renderTextureResource_;
		inputSrvIndex = renderTextureSrvIndex_;
	}
	else {
		// 直前の出力を次の入力に
		const auto& prevEffect = postEffects_.rbegin()->second;
		inputResource = prevEffect->GetOutputTextureResource();
		inputSrvIndex = prevEffect->GetOutputTextureSrvIndex();
	}

	// Ping-Pongバッファを交互に切り替える
	if (currentWriteBufferIsA_) {
		outputResource = renderTextureResourceB_;
		outputSrvIndex = renderTextureSrvIndexB_;
		currentWriteBufferIsA_ = false;
	}
	else {
		outputResource = renderTextureResourceA_;
		outputSrvIndex = renderTextureSrvIndexA_;
		currentWriteBufferIsA_ = true;
	}

	// PostEffectの初期化（Ping-Pongバッファ指定）
	postEffect->Initialize(dxCommon_, srvManager_, csFilePath,
		inputResource, inputSrvIndex,
		outputResource, outputSrvIndex);

	// PostEffectのコンテナに追加
	postEffects_.insert(std::make_pair(name, std::move(postEffect)));
}