#include "ParticleManager.h"
#include "base/ImGuiManager.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/Base/TakeCFrameWork.h"
#include <cassert>

using namespace TakeC;

//================================================================================================
// 初期化
//================================================================================================

void TakeC::ParticleManager::Initialize(ParticleCommon* particleCommon,PrimitiveDrawer* primitiveDrawer) {
	emitterAllocator_ = std::make_unique<ParticleEmitterAllocator>();
	particleCommon_ = particleCommon;
	primitiveDrawer_ = primitiveDrawer;
}

//================================================================================================
// 更新処理
//================================================================================================

void TakeC::ParticleManager::Update() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Update();
	}
}

//================================================================================================
// ImGuiの更新処理
//================================================================================================

void TakeC::ParticleManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("ParticleManager");
	ImGui::Text("ParticleGroup Count : %d", particleGroups_.size());
	ImGui::Separator();
	for (const auto& [name, particleGroup] : particleGroups_) {

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

void TakeC::ParticleManager::Draw() {
	std::unordered_map<BlendState, std::vector<PrimitiveParticle*>> blendGroups;
	for (auto& [name, group] : particleGroups_) {
		BlendState state = group->GetPreset().blendState; // BlendMode取得
		blendGroups[state].push_back(group.get());
	}

	//各ブレンドモードごとに描画
	for (auto& [blendState, groups] : blendGroups) {

		if (blendState == BlendState::COUNT) {
			return;
		}
		
		particleCommon_->PreDraw(blendState);

		for (auto group : groups) {
			group->Draw();
		}
		
	}
}

//================================================================================================
// 終了処理
//================================================================================================

void TakeC::ParticleManager::Finalize() {
	particleGroups_.clear();
}

//================================================================================================
// パーティクルグループの生成
//================================================================================================

void TakeC::ParticleManager::CreateParticleGroup(const std::string& name,const std::string& filePath,PrimitiveType primitiveType) {

	if (particleGroups_.contains(name)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(primitiveType);
	particleGroup->Initialize(particleCommon_, filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

void TakeC::ParticleManager::CreateParticleGroup(const std::string& presetJson) {
	ParticlePreset preset = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>(presetJson);
	if(particleGroups_.contains(preset.presetName)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(preset.primitiveType);
	particleGroup->Initialize(particleCommon_, preset.textureFilePath);
	particleGroup->SetPreset(preset);
	particleGroup->GeneratePrimitive();
	particleGroups_.insert(std::make_pair(preset.presetName, std::move(particleGroup)));
}


//================================================================================================
// パーティクルの発生
//================================================================================================

void TakeC::ParticleManager::Emit(const std::string& name, const Vector3& emitPosition,const Vector3& direction, uint32_t count) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}

	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition,direction, count));
}

//================================================================================================
// エミッター用のハンドルの割り当て
//================================================================================================
uint32_t TakeC::ParticleManager::EmitterAllocate() {
	return emitterAllocator_->Allocate();
}

//================================================================================================
// パーティクルグループの開放
//================================================================================================
void TakeC::ParticleManager::ClearParticleGroups() {
	particleGroups_.clear();
}

//================================================================================================
// 出現しているパーティクルのクリア
//================================================================================================
void TakeC::ParticleManager::ClearParticles() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->EraseParticle();
	}
}

//================================================================================================
// 全プリセットの読み込み
//================================================================================================
BaseParticleGroup* TakeC::ParticleManager::GetParticleGroup(const std::string& name) {

	if (particleGroups_.contains(name)) {
		return particleGroups_.at(name).get();
	}

	assert(false && "ParticleGroup not found! Please check the name.");
	return nullptr;
}

//================================================================================================
// プリセットの設定
//================================================================================================
void TakeC::ParticleManager::SetPreset(const std::string& name, const ParticlePreset& preset) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}
	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SetPreset(preset);
}

//================================================================================================
// 全プリセットの読み込み
//================================================================================================
void TakeC::ParticleManager::LoadAllPresets() {
	auto jsonLoader = TakeCFrameWork::GetJsonLoader();
	auto presetFiles = jsonLoader->GetJsonDataList<ParticlePreset>();
	for (const auto& presetFile : presetFiles) {
		CreateParticleGroup(presetFile + ".json");
	}
}