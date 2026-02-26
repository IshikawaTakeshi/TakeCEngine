#include "RenderTexture.h"

#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "PostEffect/PostEffectManager.h"

using namespace TakeC;

//======================================================================
// デストラクタ
//======================================================================
RenderTexture::~RenderTexture() {
	rootSignature_.Reset();
	renderTexturePSO_.reset();
	renderTextureResource_.Reset();
	depthStencilResource_.Reset();
}

//======================================================================
// 初期化
//======================================================================

void RenderTexture::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
	int32_t depthWidth,int32_t depthHeight){

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//RTVManagerの取得
	rtvManager_ = dxCommon_->GetRtvManager();
	
	//rtvDescの初期化
	rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	//RenderTextureResourceの生成
	renderTextureResource_ = dxCommon_->CreateRenderTextureResource(
		dxCommon_->GetDevice(),depthWidth,depthHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,kRenderTargetClearColor_);
	renderTextureResource_->SetName(L"renderTextureResource_front");

	//深度ステンシルバッファリソースの生成
	depthStencilResource_ = dxCommon_->CreateDepthStencilTextureResource(
		dxCommon_->GetDevice(), depthWidth, depthHeight);

	//RTVの生成
	rtvIndex_ = rtvManager_->Allocate();
	rtvManager_->CreateRTV(renderTextureResource_.Get(), rtvIndex_);

	//DSVの生成
	dsvIndex_ = dxCommon_->GetDsvManager()->Allocate();
	dxCommon_->GetDsvManager()->CreateDSV(depthStencilResource_, dsvIndex_);
	//深度テクスチャのSRV生成
	depthSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(depthStencilResource_.Get(), depthSrvIndex_);

	//レンダーテクスチャのSRV生成
	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(renderTextureResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvIndex_);

	//RTVのハンドルの取得
	rtvHandle_ = rtvManager_->GetRtvDescriptorHandleCPU(rtvIndex_);
	//DSVHandleの設定
	dsvHandle_ = dxCommon_->GetDsvManager()->GetDsvDescriptorHandleCPU(dsvIndex_);

	//PSO初期化
	renderTexturePSO_ = std::make_unique<PSO>();
	renderTexturePSO_->CompileVertexShader(dxCommon_->GetDXC(), L"PostEffect/FullScreen.VS.hlsl");
	renderTexturePSO_->CompilePixelShader(dxCommon_->GetDXC(), L"PostEffect/CopyImage.PS.hlsl");
	renderTexturePSO_->CreateRenderTexturePSO(dxCommon_->GetDevice());
	renderTexturePSO_->SetGraphicPipelineName("RenderTexturePSO");
	rootSignature_ = renderTexturePSO_->GetGraphicRootSignature();
	//Viewportの設定
	SetViewport(depthWidth, depthHeight);
	//ScissorRectの設定
	SetScissorRect(depthWidth, depthHeight);

	//GBufferのフォーマット
	DXGI_FORMAT formats[kNumGBuffers] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,       // Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normal
		DXGI_FORMAT_R8G8B8A8_UNORM,       // Material
	};

	//GBufferの生成
	for (int i = 0; i < kNumGBuffers; i++) {
		//Gバッファリソースの生成
		gBufferResources_[i] = dxCommon_->CreateRenderTextureResource(
			dxCommon_->GetDevice(), depthWidth, depthHeight, formats[i], kRenderTargetClearColor_);
		gBufferResources_[i]->SetName(StringUtility::ConvertString("gBufferResource_" + std::to_string(i)).c_str());
		//GバッファのRTV生成
		gBufferRtvIndices_[i] = rtvManager_->Allocate();
		rtvManager_->CreateRTV(gBufferResources_[i].Get(), gBufferRtvIndices_[i]);
		gBufferRtvHandles_[i] = rtvManager_->GetRtvDescriptorHandleCPU(gBufferRtvIndices_[i]);
		//GバッファのSRV生成
		gBufferSrvIndices_[i] = srvManager_->Allocate();
		srvManager_->CreateSRVforRenderTexture(gBufferResources_[i].Get(), DXGI_FORMAT_R16G16B16A16_FLOAT, gBufferSrvIndices_[i]);
	}
}

//=======================================================================
// レンダーターターゲットのクリア
//=======================================================================

void RenderTexture::ClearRenderTarget(RenderTargetType type) {

	if (type == RenderTargetType::Forward) {
		//描画先のRTVとDSVを設定
		dxCommon_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, FALSE, &dsvHandle_);
		//レンダーテクスチャのクリア
		dxCommon_->GetCommandList()->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);
	}
	else if(type == RenderTargetType::Deferred){

		//描画先のRTVとDSVを設定
		dxCommon_->GetCommandList()->OMSetRenderTargets(kNumGBuffers, gBufferRtvHandles_.data(), FALSE, &dsvHandle_);
		for (int i = 0; i < kNumGBuffers; i++) {
			//Gバッファのクリア
			dxCommon_->GetCommandList()->ClearRenderTargetView(gBufferRtvHandles_[i], clearValue_, 0, nullptr);
		}
		
	} else {
		assert(0 && "Invalid RenderTargetType");
	}

	//深度ステンシルバッファのクリア
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// Viewportを設定
	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport_);
	// Scissorの設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);
}

//========================================================================
// 描画前処理
//========================================================================

void RenderTexture::PreDraw() {
	// RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	// ルートシグネチャの設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	// PSOの設定
	dxCommon_->GetCommandList()->SetPipelineState(renderTexturePSO_->GetGraphicPipelineState());
	// プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//=======================================================================
// 描画
//========================================================================

void RenderTexture::Draw() {
	// 描画コマンドを発行
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

//=======================================================================
// 描画後処理
//========================================================================

void RenderTexture::PostDraw() {

	//PIXCEL_SHADER_RESOURCE >> GENERIC_READ
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}

void RenderTexture::TransitionToSRV() {
	// DEPTH_WRITE >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		depthStencilResource_.Get());
}

void RenderTexture::TransitionToDepthWrite() {
	//PIXEL_SHADER_RESOURCE >> DEPTH_WRITE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		depthStencilResource_.Get());
}

void RenderTexture::SetViewport(int32_t width, int32_t height) {
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}

void RenderTexture::SetScissorRect(int32_t width, int32_t height) {
	scissorRect_.left = LONG(0);
	scissorRect_.top = LONG(0);
	scissorRect_.right = static_cast<LONG>(width);
	scissorRect_.bottom = static_cast<LONG>(height);
}
