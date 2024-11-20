#include "ParticleManager.h"
#include"ImGuiManager.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

//================================================================================================
// 初期化
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name, const std::string& filePath) {

	if (particleGroups_.contains(name)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<Particle3d> particleGroup = std::make_unique<Particle3d>();
	particleGroup->Initialize(particleCommon,filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

//================================================================================================
// 更新処理
//================================================================================================

void ParticleManager::Update() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Update();
	}
}

//================================================================================================
// ImGuiの更新処理
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
// 描画処理
//================================================================================================

void ParticleManager::Draw() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Draw();
	}
}

void ParticleManager::Finalize() {
	particleGroups_.clear();
}

//================================================================================================
// パーティクルの発生
//================================================================================================

void ParticleManager::Emit(const std::string& name, const Vector3& emitPosition, uint32_t count) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}

	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition, count));
}