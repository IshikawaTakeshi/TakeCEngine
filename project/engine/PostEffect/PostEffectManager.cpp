#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "engine/base/ImGuiManager.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/BloomEffect.h"
#include "PostEffect/DepthBasedOutline.h"
#include "PostEffect/RenderTexture.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Math/Easing.h"

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

	// 一時エフェクト再生リクエストのImGui
	if (ImGui::CollapsingHeader("PlayEffect")) {
		int currentPresetIndex_ = -1;
		bool isSelected = false;
		isSelected = ImGuiManager::ComboBoxString("Preset", presetNames_, currentPresetIndex_);

		if (isSelected) {
			PlayEffect(presetNames_[currentPresetIndex_]);
		}
	}

	// 一時エフェクトの新規作成・編集用ImGui
	if (ImGui::CollapsingHeader("Create Temporary Effect Preset")) {
		ImGui::InputText("Preset Name", newPresetName_, sizeof(newPresetName_));

		// 登録済みポストエフェクト一覧からEffectNameを選択
		std::vector<std::string> effectNameStrs;
		for (const auto& pe : postEffects_) { effectNameStrs.push_back(pe.name); }
		int currentEffectIdx = -1;
		for (size_t i = 0; i < effectNameStrs.size(); ++i) {
			if (effectNameStrs[i] == editConfig_.effectName) {
				currentEffectIdx = static_cast<int>(i);
				break;
			}
		}
		if (ImGuiManager::ComboBoxString("Effect Name", effectNameStrs, currentEffectIdx) && currentEffectIdx >= 0) {
			editConfig_.effectName = effectNameStrs[currentEffectIdx];
		}

		// パラメータ調整
		ImGui::DragFloat("Duration", &editConfig_.duration, 0.01f, 0.0f, 10.0f);	
		ImGuiManager::ComboBoxEnum("Easing Type", editConfig_.easingType);
		ImGui::DragFloat("Start Intensity", &editConfig_.startIntensity, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat("Peak Intensity",  &editConfig_.peakIntensity,  0.01f, 0.0f, 5.0f);
		ImGui::DragFloat("End Intensity",   &editConfig_.endIntensity,   0.01f, 0.0f, 5.0f);
		ImGui::SliderFloat("Peak Time Rate", &editConfig_.peakTimeRate, 0.0f, 1.0f);

		// ボタン類
		if (ImGui::Button("Test Play")) {
			PlayEffect(editConfig_); // 作成中のパラメータで再生テスト
		}
		ImGui::SameLine();
		if (ImGui::Button("Save Preset")) {
			editConfig_.presetName = newPresetName_;

			// 現在のエフェクト本体から固有パラメータをJSONとして吸い出す
			PostEffect* target = FindEffect(editConfig_.effectName);
			if (target) {
				editConfig_.specificParams = target->GetSpecificParams();
			}

			RegisterPreset(std::string(newPresetName_), editConfig_);
			TakeCFrameWork::GetJsonLoader()->SaveJsonData<PostEffectPlayConfig>(std::string(newPresetName_) + ".json", editConfig_);
			LoadPresets(); // リストを更新
		}
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
		float peak = it->config.peakTimeRate;
		
		float t = 0.0f;
		float startVal = 0.0f;
		float endVal = 0.0f;

		if (progress < peak) {
			// 前半：start -> peak
			t = (peak > 0.0f) ? (progress / peak) : 1.0f;
			startVal = it->config.startIntensity;
			endVal = it->config.peakIntensity;
		} else {
			// 後半：peak -> end
			t = (peak < 1.0f) ? ((progress - peak) / (1.0f - peak)) : 1.0f;
			startVal = it->config.peakIntensity;
			endVal = it->config.endIntensity;
		}

		// イージング適用
		float easedT = Easing::Ease[it->config.easingType](t);
		float intensity = startVal + (endVal - startVal) * easedT;

		// エフェクト名ごとに強度を加算
		totalIntensities[it->config.effectName] = intensity;

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

	// 各固有パラメータの適用（同一フレームで複数のplayがある場合、最新のリクエストのものを優先する）
	for (auto& req : activeRequests_) {
		if (!req.config.specificParams.empty() && !req.config.specificParams.is_null()) {
			PostEffect* effect = FindEffect(req.config.effectName);
			if (effect) {
				effect->ApplySpecificParams(req.config.specificParams);
			}
		}
	}
}

//======================================================================
// プリセット名からエフェクトを再生
//======================================================================
void PostEffectManager::PlayEffect(const std::string& presetName, std::optional<float> durationOverride) {
	if (presetMap_.count(presetName) == 0) {
		Logger::Log("PostEffectManager::PlayEffect() : Preset not found: " + presetName);
		return;
	}
	
	PostEffectPlayConfig config = presetMap_[presetName];
	if (durationOverride.has_value()) {
		config.duration = durationOverride.value();
	}
	PlayEffect(config);
}

//======================================================================
// 設定構造体から直接エフェクトを再生
//======================================================================
void PostEffectManager::PlayEffect(const PostEffectPlayConfig& config) {
	// 同じ対象エフェクト(effectName)の再生リクエストが存在するか検索
	for (auto& req : activeRequests_) {
		if (req.config.effectName == config.effectName) {
			// 存在する場合は上書きしてタイマーをリセット（連打による強度無限増加を防ぐ）
			req.config = config;
			req.timer.Initialize(config.duration, 0.0f);
			return;
		}
	}

	// なければ新規追加
	PlayRequest request;
	request.config = config;
	request.timer.Initialize(config.duration, 0.0f);
	activeRequests_.push_back(request);
}

void TakeC::PostEffectManager::StopAllEffects() {
	activeRequests_.clear();
	for (auto& effect : postEffects_) {
		effect.postEffect->SetIntensity(0.0f);
		effect.postEffect->SetIsActive(false);
	}
}

//======================================================================
// プリセットを動的に登録
//======================================================================
void PostEffectManager::RegisterPreset(const std::string& name, const PostEffectPlayConfig& config) {
	presetMap_[name] = config;
}

//======================================================================
// プリセットを登録解除
//======================================================================
void PostEffectManager::UnregisterPreset(const std::string& name) {
	presetMap_.erase(name);
}

//======================================================================
// JSONからプリセットを一括ロード
//======================================================================
void PostEffectManager::LoadPresets() {
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<PostEffectPlayConfig>();
	for (const auto& name : presetNames_) {
		presetMap_[name] = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PostEffectPlayConfig>(name + ".json");
	}
	Logger::Log("PostEffectManager : Loaded " + std::to_string(presetMap_.size()) + " presets.");
}
