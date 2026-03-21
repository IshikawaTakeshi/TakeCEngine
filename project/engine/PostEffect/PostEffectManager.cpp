#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "engine/base/ImGuiManager.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/BloomEffect.h"
#include "PostEffect/DepthBasedOutline.h"
#include "PostEffect/RenderTexture.h"
#include "engine/Base/TakeCFrameWork.h"

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

//======================================================================
// 更新処理(一時エフェクトのタイマー進行・パラメータ反映)
//======================================================================
void PostEffectManager::Update() {
	// エフェクト名ごとの合計強度を保持するマップ
	std::unordered_map<std::string, float> totalIntensities;

	// 再生中のリクエストを更新
	for (auto it = activeRequests_.begin(); it != activeRequests_.end(); ) {
		// タイマーの更新
		it->timer.Update();

		// タイマーの進捗(0.0~1.0)
		float progress = it->timer.GetProgress();
		// Easing適用後の進捗
		float easeProgress = it->timer.GetEase(static_cast<Easing::EasingType>(it->config.easingType));

		// 強度の計算（山なりに変化するように start -> peak -> end を線形補間）
		float intensity = 0.0f;
		if (progress < 0.5f) {
			// 前半：start -> peak (0.0~0.5 を 0.0~1.0 に換算して補間)
			intensity = it->config.startIntensity + (it->config.peakIntensity - it->config.startIntensity) * easeProgress;
			// ※ easingType によっては behavior が変わるため、ここでは簡易的に easeProgress をそのまま使っているが、
			//   山なりを表現するには timer 側でもう少し工夫が必要かもしれない。
			//   とりあえず「開始->ピーク->終了」の3点補間ではなく、「山なりイージング」は easingType 自体(OUT_BOUNCE 等)か
			//   別途計算ロジックが必要。
		} else {
			// 後半：peak -> end
			intensity = it->config.peakIntensity + (it->config.endIntensity - it->config.peakIntensity) * easeProgress;
		}
		
		// とりあえず今回は「0.0->1.0->0.0」のような挙動ではなく、「イージングに従って startからendへ向かう」挙動とする
		// （山なりにしたい場合は easingType に山なりのもの(SIN_WAVEなど)を選ぶか、
		//   ピークが必要なら別途計算式を導入する）
		// ここではプランに合わせて、単純なイージング進捗で start -> end を補間する（山なりは easing か peak を使う想定）
		
		// 1.0をピークとする山なり計算の簡易例:
		// float sinT = std::sin(progress * 3.14159f); 
		// intensity = it->config.startIntensity + (it->config.peakIntensity - it->config.startIntensity) * sinT;
		
		intensity = it->config.startIntensity + (it->config.peakIntensity - it->config.startIntensity) * easeProgress;

		// エフェクト名ごとに強度を加算
		totalIntensities[it->config.effectName] += intensity;

		// 終了判定
		if (it->timer.IsFinished()) {
			it = activeRequests_.erase(it);
		} else {
			++it;
		}
	}

	// 合計強度を各エフェクトに反映
	for (auto& effect : postEffects_) {
		if (totalIntensities.count(effect.name)) {
			effect.postEffect->SetIntensity(totalIntensities[effect.name]);
			// 0より大きければアクティブにする
			effect.postEffect->SetIsActive(totalIntensities[effect.name] > 0.0f);
		} else {
			// リクエストがない場合は 0 に戻す
			// ※ ただし「常時オン」にしたいエフェクト（ShadowMap等）を上書きしてしまわないか注意。
			//   一時エフェクトのみを SetIntensity 経由で管理するように各エフェクトを実装すればOK。
		}
	}
}

//======================================================================
// プリセット名からエフェクトを再生
//======================================================================
void PostEffectManager::PlayEffect(const std::string& presetName) {
	if (presetMap_.count(presetName) == 0) {
		Logger::Log("PostEffectManager::PlayEffect() : Preset not found: " + presetName);
		return;
	}
	PlayEffect(presetMap_[presetName]);
}

//======================================================================
// 設定構造体から直接エフェクトを再生
//======================================================================
void PostEffectManager::PlayEffect(const PostEffectPlayConfig& config) {
	PlayRequest request;
	request.config = config;
	request.timer.Initialize(config.duration, 0.0f);
	activeRequests_.push_back(request);
}

//======================================================================
// JSONからプリセットを一括ロード
//======================================================================
void PostEffectManager::LoadPresets() {
	std::vector<std::string> presetNames = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<PostEffectPlayConfig>();
	for (const auto& name : presetNames) {
		presetMap_[name] = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PostEffectPlayConfig>(name + ".json");
	}
	Logger::Log("PostEffectManager : Loaded " + std::to_string(presetMap_.size()) + " presets.");
}
