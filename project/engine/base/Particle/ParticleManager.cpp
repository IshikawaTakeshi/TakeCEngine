#include "ParticleManager.h"
#include "base/ImGuiManager.h"
#include "base/TakeCFrameWork.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include <cassert>

//================================================================================================
// 初期化
//================================================================================================

void ParticleManager::Initialize(ParticleCommon* particleCommon) {
	emitterAllocater_ = std::make_unique<ParticleEmitterAllocator>();
	particleCommon_ = particleCommon;
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

void ParticleManager::Finalize() {
	particleGroups_.clear();
}

void ParticleManager::UpdatePrimitiveType(const std::string& groupName, PrimitiveType type,const Vector3& param) {

	if (!particleGroups_.contains(groupName)) {
		assert(false && "ParticleGroup not found! Please check the name.");
		return;
	}

	uint32_t newHandle = 0;
	//指定されたグループのプリミティブタイプを更新
	switch (type) {
	case PRIMITIVE_RING:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(
			param.x,param.y,particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	case PRIMITIVE_PLANE:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(
			param.x, param.y, particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	case PRIMITIVE_SPHERE:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GenerateSphere(
			param.x, particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	case PRIMITIVE_CONE:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GenerateCone(
			param.x, param.y,16, particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	default:
		break;
	}
	particleGroups_.at(groupName)->SetPrimitiveHandle(newHandle);
}

//================================================================================================
// パーティクルグループの生成
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon, const std::string& name,
		const std::string& filePath,PrimitiveType primitiveType) {

	if (particleGroups_.contains(name)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(primitiveType);
	particleGroup->Initialize(particleCommon, filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon, const std::string& presetJson) {
	ParticlePreset preset = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ParticlePreset>(presetJson);
	if(particleGroups_.contains(preset.presetName)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(preset.primitiveType);
	particleGroup->Initialize(particleCommon, preset.textureFilePath);
	particleGroup->SetPreset(preset);
	particleGroups_.insert(std::make_pair(preset.presetName, std::move(particleGroup)));
}


//================================================================================================
// パーティクルの発生
//================================================================================================

void ParticleManager::Emit(const std::string& name, const Vector3& emitPosition,const Vector3& direction, uint32_t count) {

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
uint32_t ParticleManager::EmitterAllocate() {
	return emitterAllocater_->Allocate();
}

//================================================================================================
// パーティクルグループの開放
//================================================================================================
void ParticleManager::ClearParticleGroups() {
	particleGroups_.clear();
}

//================================================================================================
// 出現しているパーティクルのクリア
//================================================================================================
void ParticleManager::ClearParticles() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->SpliceParticles(std::list<Particle>{});
	}
}

BaseParticleGroup* ParticleManager::GetParticleGroup(const std::string& name) {

	if (particleGroups_.contains(name)) {
		return particleGroups_.at(name).get();
	}

	assert(false && "ParticleGroup not found! Please check the name.");
	return nullptr;
}

void ParticleManager::SetPreset(const std::string& name, const ParticlePreset& preset) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}
	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SetPreset(preset);
}
