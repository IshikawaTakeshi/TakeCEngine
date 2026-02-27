#include "RenderTexture.h"

#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "PostEffect/PostEffectManager.h"
#include "base/TakeCFrameWork.h"
#include "base/TextureManager.h"

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
	//DXGI_FORMAT formats[kNumGBuffers] = {
	//	DXGI_FORMAT_R8G8B8A8_UNORM,       // Albedo
	//	DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normal
	//	DXGI_FORMAT_R8G8B8A8_UNORM,       // Material
	//};

	//GBufferの生成
	auto gBufferType = magic_enum::enum_entries<GBufferTypeEnum>();
	for (int i = 0; i < kNumGBuffers; i++) {
		//Gバッファリソースの生成
		gBufferResources_[i] = dxCommon_->CreateRenderTextureResource(
			dxCommon_->GetDevice(), depthWidth, depthHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor_);
		auto BufferTypeName = std::string(gBufferType[i].second.data());
		gBufferResources_[i]->SetName(StringUtility::ConvertString("gBufferResource_" + BufferTypeName).c_str());
		//GバッファのRTV生成
		gBufferRtvIndices_[i] = rtvManager_->Allocate();
		rtvManager_->CreateRTV(gBufferResources_[i].Get(), gBufferRtvIndices_[i]);
		gBufferRtvHandles_[i] = rtvManager_->GetRtvDescriptorHandleCPU(gBufferRtvIndices_[i]);
		//GバッファのSRV生成
		gBufferSrvIndices_[i] = srvManager_->Allocate();
		srvManager_->CreateSRVforRenderTexture(gBufferResources_[i].Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, gBufferSrvIndices_[i]);
	}

	//LightingPass用のPSO初期化
	lightingPassPSO_ = std::make_unique<PSO>();
	lightingPassPSO_->CompileVertexShader(dxCommon_->GetDXC(), L"PostEffect/FullScreen.VS.hlsl");
	lightingPassPSO_->CompilePixelShader(dxCommon_->GetDXC(), L"Light/LightingPass.PS.hlsl");
	lightingPassPSO_->CreateRenderTexturePSO(dxCommon_->GetDevice());
	lightingPassPSO_->SetGraphicPipelineName("LightingPassPSO");
	//LightingPass用のRootSignatureの取得
	lightingPassRootSignature_ = lightingPassPSO_->GetGraphicRootSignature();
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
//=======================================================================

void RenderTexture::PostDraw() {

	//PIXCEL_SHADER_RESOURCE >> GENERIC_READ
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}

//=======================================================================
// SRVへの遷移
//=======================================================================
void RenderTexture::TransitionDepthWriteToSRV() {
	// DEPTH_WRITE >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		depthStencilResource_.Get());
}

//=======================================================================
// 深度書き込みへの遷移
//=======================================================================
void RenderTexture::TransitionSRVToDepthWrite() {
	//PIXEL_SHADER_RESOURCE >> DEPTH_WRITE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		depthStencilResource_.Get());
}

//================================================================
//GバッファのSRVをライティングシェーダーにバインド
//================================================================
void RenderTexture::PreDrawLightingPass() {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//PSOの設定
	commandList->SetPipelineState(lightingPassPSO_->GetGraphicPipelineState());
	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(lightingPassRootSignature_.Get());
	// プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Albedo:RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		gBufferResources_[GBUFFER_Albedo].Get());
	//Normal:RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		gBufferResources_[GBUFFER_Normal].Get());
	//Material:RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		gBufferResources_[GBUFFER_Material].Get());
	//depth:DEPTH_WRITE >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		depthStencilResource_.Get());

	//gGBufferAlbedo
	srvManager_->SetGraphicsRootDescriptorTable(
		lightingPassPSO_->GetGraphicBindResourceIndex("gGBufferAlbedo"),
		gBufferSrvIndices_[GBUFFER_Albedo]);
	//gGBufferNormal
	srvManager_->SetGraphicsRootDescriptorTable(
		lightingPassPSO_->GetGraphicBindResourceIndex("gGBufferNormal"),
		gBufferSrvIndices_[GBUFFER_Normal]);
	//gGBufferMaterial
	srvManager_->SetGraphicsRootDescriptorTable(
		lightingPassPSO_->GetGraphicBindResourceIndex("gGBufferMaterial"),
		gBufferSrvIndices_[GBUFFER_Material]);
	//gSceneDepth
	srvManager_->SetGraphicsRootDescriptorTable(
		lightingPassPSO_->GetGraphicBindResourceIndex("gSceneDepth"),
		depthSrvIndex_);

	//gEnvMapTexture
	srvManager_->SetGraphicsRootDescriptorTable(
		lightingPassPSO_->GetGraphicBindResourceIndex("gEnvMapTexture"),
		TakeC::TextureManager::GetInstance().GetSrvIndex("skyBox_blueSky.dds"));

	//gMainCameraInfo
	commandList->SetGraphicsRootConstantBufferView(
		lightingPassPSO_->GetGraphicBindResourceIndex("gMainCameraInfo"), 
		TakeC::CameraManager::GetInstance().GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());
	//ライトリソースの設定
	TakeCFrameWork::GetLightManager()->SetLightResources(lightingPassPSO_.get());

	
}

void RenderTexture::PostDrawLightingPass() {
	//Albedo:PIXEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		gBufferResources_[GBUFFER_Albedo].Get());
	//Normal:PIXEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		gBufferResources_[GBUFFER_Normal].Get());
	//Material:PIXEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		gBufferResources_[GBUFFER_Material].Get());
	//Depth:PIXEL_SHADER_RESOURCE >> DEPTH_WRITE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
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
