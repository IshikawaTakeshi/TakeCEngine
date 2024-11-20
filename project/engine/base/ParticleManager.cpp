#include "ParticleManager.h"
#include"ImGuiManager.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

//================================================================================================
// 繝代・繝・ぅ繧ｯ繝ｫ繧ｰ繝ｫ繝ｼ繝励・逕滓・
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name, const std::string& filePath) {

	if (particleGroups_.contains(name)) {
		//縺吶〒縺ｫ隱ｭ縺ｿ霎ｼ縺ｿ貂医∩縺ｪ繧詠eturn
		return;
	}

	//繝代・繝・ぅ繧ｯ繝ｫ繧ｰ繝ｫ繝ｼ繝励・逕滓・縺ｨ繝輔ぃ繧､繝ｫ隱ｭ縺ｿ霎ｼ縺ｿ縲∝・譛溷喧
	std::unique_ptr<Particle3d> particleGroup = std::make_unique<Particle3d>();
	particleGroup->Initialize(particleCommon,filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

//================================================================================================
// 繝代・繝・ぅ繧ｯ繝ｫ繧ｰ繝ｫ繝ｼ繝励・譖ｴ譁ｰ
//================================================================================================

void ParticleManager::Update() {

	//MEMO:particleGroups_縺ｮ蛟､繧地ame縺ｨparticle縺ｫ蛻・ｧ｣
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Update();
	}
}

//================================================================================================
// ImGui縺ｮ譖ｴ譁ｰ
//================================================================================================

void ParticleManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("ParticleManager");
	ImGui::Text("ParticleGroup Count : %d", particleGroups_.size());
	ImGui::Separator();
	for (const auto& [name, particleGroup] : particleGroups_) {
		// 繝懊ち繝ｳ繧定｡ｨ遉ｺ縺励・
		if (ImGui::TreeNode(name.c_str())) {
			particleGroup->UpdateImGui();
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif // _DEBUG
}

//================================================================================================
// 繝代・繝・ぅ繧ｯ繝ｫ繧ｰ繝ｫ繝ｼ繝励・謠冗判
//================================================================================================

void ParticleManager::Draw() {

	//MEMO:particleGroups_縺ｮ蛟､繧地ame縺ｨparticle縺ｫ蛻・ｧ｣
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Draw();
	}
}

void ParticleManager::Finalize() {
	particleGroups_.clear();
}

//================================================================================================
// 繝代・繝・ぅ繧ｯ繝ｫ縺ｮ逋ｺ逕・
//================================================================================================

void ParticleManager::Emit(const std::string& name, const Vector3& emitPosition, uint32_t count) {

	//逋ｻ骭ｲ貂医∩縺ｮ繧ｰ繝ｫ繝ｼ繝怜錐縺九←縺・°繝√ぉ繝・け
	if (!particleGroups_.contains(name)) {
		return;
	}

	//謖・ｮ壹＆繧後◆繝代・繝・ぅ繧ｯ繝ｫ繧ｰ繝ｫ繝ｼ繝励↓逋ｻ骭ｲ
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition, count));
}