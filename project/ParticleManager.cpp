#include "ParticleManager.h"
#include"ImGuiManager.h"
#include <cassert>

ParticleManager* ParticleManager::GetInstance() {
	static ParticleManager instance;
	return &instance;
}

//================================================================================================
// パーティクルグループの生成
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name, const std::string& filePath) {

	if (particleGroups_.contains(name)) {
		//すでに読み込み済みならreturn
		return;
	}

	//パーティクルグループの生成とファイル読み込み、初期化
	std::unique_ptr<Particle3d> particleGroup = std::make_unique<Particle3d>();
	particleGroup->Initialize(particleCommon,filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

//================================================================================================
// パーティクルグループの更新
//================================================================================================

void ParticleManager::Update() {

	//MEMO:particleGroups_の値をnameとparticleに分解
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
		// ボタンを表示し、
		if (ImGui::TreeNode(name.c_str())) {
			particleGroup->UpdateImGui();
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif // _DEBUG
}

//================================================================================================
// パーティクルグループの描画
//================================================================================================

void ParticleManager::Draw() {

	//MEMO:particleGroups_の値をnameとparticleに分解
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

	//登録済みのグループ名かどうかチェック
	if (!particleGroups_.contains(name)) {
		return;
	}

	//指定されたパーティクルグループに登録
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition, count));
}