#include "ParticleEmitter.h"
#include "TakeCFrameWork.h"
#include "ImGuiManager.h"

ParticleEmitter::~ParticleEmitter() {

}

void ParticleEmitter::Initialize(const std::string& particleName, Transform transforms, uint32_t count, float frequency) {

	//Emitter初期化
	particleName_ = particleName;
	transforms_.translate = transforms.translate;
	transforms_.rotate = transforms.rotate;
	transforms_.scale = transforms.scale;
	particleCount_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;
}

void ParticleEmitter::Update() {

	//エミッターの更新
	if (!isEmit_) return;
	frequencyTime_ += kDeltaTime_;
	if (frequency_ <= frequencyTime_) {
		TakeCFrameWork::GetParticleManager()->Emit(particleName_, transforms_.translate, particleCount_);
		frequencyTime_ -= frequency_; //余計に過ぎた時間も加味して頻度計算する
	}
}

void ParticleEmitter::UpdateImGui() {
	ImGui::Begin("ParticleEmitter");
	if (ImGui::Button("SpawnParticle")) {
		isEmit_ = !isEmit_;
	}
	ImGui::SliderFloat3("Translate", &transforms_.translate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Rotate", &transforms_.rotate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Scale", &transforms_.scale.x, 0.0f, 10.0f);
	ImGui::End();
}

void ParticleEmitter::Emit() {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, transforms_.translate, particleCount_);
}