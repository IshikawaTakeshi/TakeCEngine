#include "PostEffectFactory.h"
#include "engine/PostEffect/BloomEffect.h"
#include "engine/PostEffect/BoxFilter.h"
#include "engine/PostEffect/Dissolve.h"
#include "engine/PostEffect/DepthBasedOutline.h"
#include "engine/PostEffect/GrayScale.h"
#include "engine/PostEffect/LuminanceBasedOutline.h"
#include "engine/PostEffect/RadialBluer.h"
#include "engine/PostEffect/ShadowMapEffect.h"
#include "engine/PostEffect/Vignette.h"

using namespace TakeC;


PostEffectFactory::PostEffectFactory() {
    Register("BloomEffect", []() { return std::make_unique<BloomEffect>(); });
    Register("BoxFilter", []() { return std::make_unique<BoxFilter>(); });
    Register("DepthBasedOutline", []() { return std::make_unique<DepthBasedOutline>(); });
    Register("Dissolve", []() { return std::make_unique<Dissolve>(); });
    Register("GrayScale", []() { return std::make_unique<GrayScale>(); });
    Register("LuminanceBasedOutline", []() { return std::make_unique<LuminanceBasedOutline>(); });
    Register("RadialBluer", []() { return std::make_unique<RadialBluer>(); });
    Register("ShadowMapEffect", []() { return std::make_unique<ShadowMapEffect>(); });
    Register("Vignette", []() { return std::make_unique<Vignette>(); });
}

//============================================================
//	エフェクトの登録
//============================================================
void PostEffectFactory::Register(const std::string& name, Creator creator) {
	creators_[name] = creator;
}

//============================================================
//	エフェクトの生成
//============================================================
std::unique_ptr<PostEffect> PostEffectFactory::Create(const std::string& name) const {
	auto it = creators_.find(name);
	if (it != creators_.end()) {
		return it->second();
	}
	return nullptr;
}

//============================================================
//	エフェクトが登録されているか確認
//============================================================
bool PostEffectFactory::IsRegistered(const std::string& name) const {
	return creators_.contains(name);
}