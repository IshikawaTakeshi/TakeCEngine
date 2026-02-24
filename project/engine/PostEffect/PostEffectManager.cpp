#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "engine/base/ImGuiManager.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/BloomEffect.h"
#include "PostEffect/DepthBasedOutline.h"
#include "PostEffect/RenderTexture.h"

using namespace TakeC;

//====================================================================
//	初期化
//====================================================================

void PostEffectManager::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,RenderTexture* renderTexture) {

	dxCommon_ = dxCommon; //DirectXCommonのセット
	srvManager_ = srvManager; //SrvManagerのセット
	renderTexture_ = renderTexture; //RenderTextureのセット

	//中間リソースの生成
	intermediateResource_[FRONT] = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(), WinApp::kScreenWidth, WinApp::kScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	intermediateResource_[FRONT]->SetName(L"intermediateResource_A");
	intermediateResource_[BACK] = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(), WinApp::kScreenWidth, WinApp::kScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	intermediateResource_[BACK]->SetName(L"intermediateResource_B");

	renderTextureResource_ = renderTexture_->GetRenderTextureResource();
	renderTextureSrvIndex_ = renderTexture_->GetSrvIndex();
}

//====================================================================
// ImGui更新処理
//====================================================================
void PostEffectManager::UpdateImGui() {
	ImGui::Begin("PostEffectManager");
	for (auto& postEffect : postEffects_) {
		postEffect.postEffect->UpdateImGui();
	}
	ImGui::End();
}

//====================================================================
// 終了処理
//====================================================================

void PostEffectManager::Finalize() {
	intermediateResource_[FRONT].Reset();
	intermediateResource_[BACK].Reset();
	renderTextureResource_.Reset();
	postEffects_.clear();
}

//======================================================================
// ComputeShaderによる全PostEffectの処理
//======================================================================

void PostEffectManager::AllDispatch() {

	// RENDER_TARGET >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	for (auto& postEffect : postEffects_) {
		postEffect.postEffect->Dispatch();
	}

	//NON_PIXCEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}

//======================================================================
// 描画処理
//======================================================================

void PostEffectManager::Draw(PSO* pso) {

	// gTexture
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),postEffects_.back().postEffect->GetOutputTextureSrvIndex());
}

//======================================================================
// 特定のPostEffectの処理
//======================================================================
void PostEffectManager::ApplyEffect(const std::string& name) {

	// 読み込み済みかどうか検索
	for (const auto& postEffect : postEffects_) {
		if (postEffect.name == name) {
			// 名前が一致したPostEffectのDispatchを実行
			postEffect.postEffect->Dispatch();
			return;
		}
	}
}

//======================================================================
// 特定のPostEffectの初期化
//======================================================================
void PostEffectManager::InitializeEffect(const std::string& name, const std::wstring& csFilePath) {

	// 読み込み済みかどうか検索
	for (const auto& postEffect : postEffects_) {
		if (postEffect.name == name) {
			Logger::Log("PostEffectManager::InitializeEffect() : PostEffect is already initialized.");
			return;
		}
	}

	std::unique_ptr<PostEffect> postEffect;

	// 入出力テクスチャを決定（Ping-Pong切り替え）
	ComPtr<ID3D12Resource> inputResource = nullptr;
	uint32_t inputSrvIndex = 0;
	ComPtr<ID3D12Resource> outputResource = nullptr;

	// PostEffectの初期化
	// ファクトリーからPostEffectを生成
	postEffect = postEffectFactory_->Create(name);
	if (!postEffect) {
		Logger::Log("PostEffectManager:: InitializeEffect() : Unknown PostEffect name:  " + name);
		return;
	}
	if(name == "DepthBasedOutline" || name == "ShadowMapEffect") {
		// 深度テクスチャリソースとSRVインデックスを設定
		postEffect->SetDepthTextureResource(
			renderTexture_->GetDepthStencilResource(),
			renderTexture_->GetDepthSrvIndex());

		if(name == "ShadowMapEffect") {
			// ShadowMapEffectの場合,lightCameraの深度テクスチャの設定も行う
			postEffect->SetLightCameraDepthTextureResource(
				lightCameraRenderTexture_->GetDepthStencilResource(),
				lightCameraRenderTexture_->GetDepthSrvIndex());
		}
	}

	if (postEffects_.empty()) {

		// 最初の入力は通常の描画結果（renderTextureResource_）
		inputResource = renderTextureResource_;
		inputSrvIndex = renderTextureSrvIndex_;
	}
	else {
		// 直前の出力を次の入力に
		const auto& prevEffect = postEffects_.back().postEffect;
		inputResource = prevEffect->GetOutputTextureResource();
		inputSrvIndex = prevEffect->GetOutputTextureSrvIndex();
	}

	//ping-pong状態で出力リソースとインデックスを決定
	IntermediateResourceType bufferIndex = currentWriteBufferIsA_ ? FRONT : BACK;
	outputResource = intermediateResource_[bufferIndex];

	// PostEffectの初期化（Ping-Pongバッファ指定）
	postEffect->Initialize(dxCommon_, srvManager_, csFilePath,
		inputResource, inputSrvIndex,outputResource);


	// PostEffectのコンテナに追加
	postEffects_.push_back({ name, std::move(postEffect) });
	// Ping-Pongバッファの切り替え
	currentWriteBufferIsA_ = !currentWriteBufferIsA_; 
}

//======================================================================
// 特定のPostEffectが存在するか検索
//======================================================================
PostEffect* TakeC::PostEffectManager::FindEffect(const std::string& name) const {
	
	for (const auto& postEffect : postEffects_) {
		if (postEffect.name == name) {
			return postEffect.postEffect.get();
		}
	}
	return nullptr; // 見つからない場合はnullptrを返す
}

void TakeC::PostEffectManager::SetEffectActive(const std::string& name, bool isActive) {

	PostEffect* effect = FindEffect(name);
	if (effect) {
		
		effect->SetIsActive(isActive);
	}
	else {
		Logger::Log("PostEffectManager::SetEffectActive() : PostEffect not found: " + name);
	}
}


//======================================================================
// 最終出力テクスチャのSRVインデックスを取得
//======================================================================
uint32_t PostEffectManager::GetFinalOutputSrvIndex() const {
	return postEffects_.back().postEffect->GetOutputTextureSrvIndex();
}
