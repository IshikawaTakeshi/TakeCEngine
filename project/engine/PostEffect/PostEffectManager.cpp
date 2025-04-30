#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"

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
	//RTVの生成
	dxCommon_->GetDevice()->CreateRenderTargetView(renderTextureResource_.Get(), &rtvDesc_, rtvHandle_);

	//SRVの生成
	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(renderTextureResource_.Get(), srvIndex_);

	//PSO初期化
	renderTexturePSO_ = std::make_unique<PSO>();
	renderTexturePSO_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/PostEffect/CopyImage.VS.hlsl");
	renderTexturePSO_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/PostEffect/CopyImage.PS.hlsl");
	renderTexturePSO_->CreateRenderTexturePSO(dxCommon_->GetDevice());

	rootSignature_ = renderTexturePSO_->GetGraphicRootSignature();
}

//====================================================================
// 終了処理
//====================================================================

void PostEffectManager::Finalize() {
	renderTextureResource_.Reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

//====================================================================
// 描画前処理
//====================================================================

void PostEffectManager::PreDraw() {

	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());

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

//======================================================================
// 描画処理
//======================================================================

void PostEffectManager::Draw() {

	//// RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		renderTextureResource_.Get());

	//ルートシグネチャの設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSOの設定
	dxCommon_->GetCommandList()->SetPipelineState(renderTexturePSO_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//gTexture
	srvManager_->SetGraphicsRootDescriptorTable(renderTexturePSO_->GetGraphicBindResourceIndex("gTexture"),srvIndex_);
	//描画コマンドを発行
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

//======================================================================
// 描画後処理
//======================================================================

void PostEffectManager::PostDraw() {
	//PIXCEL_SHADER_RESOURCE >> GENERIC_READ
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		renderTextureResource_.Get());
}
