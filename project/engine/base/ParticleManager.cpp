#include "ParticleManager.h"
#include"ImGuiManager.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

//================================================================================================
// パ�EチE��クルグループ�E生�E
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name, const std::string& filePath) {

	if (particleGroups_.contains(name)) {
		//すでに読み込み済みならreturn
		return;
	}

	//パ�EチE��クルグループ�E生�Eとファイル読み込み、�E期化
	std::unique_ptr<Particle3d> particleGroup = std::make_unique<Particle3d>();
	particleGroup->Initialize(particleCommon,filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

//================================================================================================
// パ�EチE��クルグループ�E更新
//================================================================================================

void ParticleManager::Update() {

	//MEMO:particleGroups_の値をnameとparticleに刁E��
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Update();
	}
}

//================================================================================================
// ImGuiの更新
//================================================================================================

void ParticleManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("ParticleManager");
	ImGui::Text("ParticleGroup Count : %d", particleGroups_.size());
	ImGui::Separator();
	for (const auto& [name, particleGroup] : particleGroups_) {
		// ボタンを表示し、E
		if (ImGui::TreeNode(name.c_str())) {
			particleGroup->UpdateImGui();
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif // _DEBUG
}

//================================================================================================
// パ�EチE��クルグループ�E描画
//================================================================================================

void ParticleManager::Draw() {

	//MEMO:particleGroups_の値をnameとparticleに刁E��
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Draw();
	}
}

void ParticleManager::Finalize() {
	particleGroups_.clear();
}

//================================================================================================
// パ�EチE��クルの発甁E
//================================================================================================

void ParticleManager::Emit(const std::string& name, const Vector3& emitPosition, uint32_t count) {

	//登録済みのグループ名かどぁE��チェチE��
	if (!particleGroups_.contains(name)) {
		return;
	}

	//持E��されたパ�EチE��クルグループに登録
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition, count));
}