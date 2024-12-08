#include "ParticleEmitter.h"
#include "TakeCFrameWork.h"
#include "ImGuiManager.h"
//#include "ModelManager.h"

ParticleEmitter::~ParticleEmitter() {

}

void ParticleEmitter::Initialize(const std::string& emitterName, Transform transforms, uint32_t count, float frequency) {

	
	//Emitter初期化
	emitterName_ = emitterName;
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
		Emit();
		frequencyTime_ -= frequency_; //余計に過ぎた時間も加味して頻度計算する
	}
}

void ParticleEmitter::UpdateImGui() {

	ImGui::Begin("ParticleEmitter");
	if (ImGui::TreeNode(emitterName_.c_str())) {
		ImGui::Checkbox("Emit", &isEmit_);
		ImGui::SliderFloat3("Translate", &transforms_.translate.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Rotate", &transforms_.rotate.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Scale", &transforms_.scale.x, 0.0f, 10.0f);
		int count = particleCount_;
		ImGui::SliderInt("ParticleCount", &count, 0, 100);
		particleCount_ = count;
		ImGui::SliderFloat("Frequency", &frequency_, 0.0f, 1.0f);

		ImGui::TreePop();
	}
	ImGui::End();
}

//void ParticleEmitter::DrawEmitRange() {
//	//model_->Draw();
//}

void ParticleEmitter::Emit() {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, transforms_.translate, particleCount_);
}