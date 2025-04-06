#include "ParticleEmitter.h"
#include "TakeCFrameWork.h"
#include "ImGuiManager.h"
#include "SrvManager.h"
#include "3d/Particle/GPUParticle.h"

ParticleEmitter::~ParticleEmitter() {
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	emitParticlePso_.reset();
	emitParticleRootSignature_.Reset();
	emitterSphereResource_.Reset();
}

//================================================================================================
// 初期化
//================================================================================================

void ParticleEmitter::Initialize(const std::string& emitterName, EulerTransform transforms, uint32_t count, float frequency) {

	
	//Emitter初期化
	emitterName_ = emitterName;
	transforms_.translate = transforms.translate;
	transforms_.rotate = transforms.rotate;
	transforms_.scale = transforms.scale;
	particleCount_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;
}

void ParticleEmitter::InitializeEmitterSphere(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//PSO生成
	emitParticlePso_ = std::make_unique<PSO>();
	emitParticlePso_->CompileComputeShader(dxCommon_->GetDXC(), L"Resources/shaders/EmitParticle.CS.hlsl");
	emitParticlePso_->CreateComputePSO(dxCommon_->GetDevice());

	//RootSignature生成
	emitParticleRootSignature_ = emitParticlePso_->GetComputeRootSignature();

	//EmitterSphereResource生成
	emitterSphereResource_ = 
		DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(EmitterSphereInfo));
	//PerFrameResource生成
	perFrameResource_ = 
		DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PerFrame));

	//Mapping
	emitterSphereResource_->Map(0, nullptr, reinterpret_cast<void**>(&emitterSphereInfo_));
	perFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));

	//SRV生成
	emitterSphereSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(
		1,
		sizeof(EmitterSphereInfo),
		emitterSphereResource_.Get(),
		emitterSphereSrvIndex_
	);

	//EmitterSphereInfo初期化
	emitterSphereInfo_->particleCount = 10;
	emitterSphereInfo_->frequency = 0.1f;
	emitterSphereInfo_->translate = Vector3(0.0f, 0.0f, 0.0f);
	emitterSphereInfo_->radius = 1.0f;
	emitterSphereInfo_->isEmit = 1;

	//PerFrameData初期化
	perFrameData_->gameTime = TakeCFrameWork::GetGameTime();
	perFrameData_->deltaTime = TakeCFrameWork::GetDeltaTime();
}

void ParticleEmitter::Update() {

	//エミッターの更新
	if (!isEmit_) return;
	frequencyTime_ += TakeCFrameWork::GetDeltaTime();
	if (frequency_ <= frequencyTime_) {
		Emit();
		frequencyTime_ -= frequency_; //余計に過ぎた時間も加味して頻度計算する
	}
}

void ParticleEmitter::UpdateForGPU() {

	//時間の取得及び更新
	perFrameData_->gameTime = TakeCFrameWork::GetGameTime();

	emitterSphereInfo_->frequencyTime += perFrameData_->deltaTime;
	if(emitterSphereInfo_->frequency <= emitterSphereInfo_->frequencyTime){
		emitterSphereInfo_->isEmit = 1;
		emitterSphereInfo_->frequencyTime -= emitterSphereInfo_->frequency;
	}
	else {
		emitterSphereInfo_->isEmit = 0;
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

void ParticleEmitter::EmitParticle(GPUParticle* gpuParticle) {

	//PSOの設定
	dxCommon_->GetCommandList()->SetPipelineState(emitParticlePso_->GetComputePipelineState());
	dxCommon_->GetCommandList()->SetComputeRootSignature(emitParticleRootSignature_.Get());

	//DescriptorHeapの設定
	srvManager_->SetDescriptorHeap();

	//TransitionBarrierを張る
	D3D12_RESOURCE_BARRIER uavBarrier = {};

	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	uavBarrier.Transition.pResource = gpuParticle->GetParticleUavResource();
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &uavBarrier);


	//Resourceの設定
	//0.PerFrame
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(0, perFrameResource_->GetGPUVirtualAddress());
	//1.EmitterSphereInfo
	srvManager_->SetComputeRootDescriptorTable(1, emitterSphereSrvIndex_);
	//2.Particle
	srvManager_->SetComputeRootDescriptorTable(2, gpuParticle->GetParticleUavIndex());
	//3.FreeCounter
	srvManager_->SetComputeRootDescriptorTable(3, gpuParticle->GetFreeCounterIndex());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(1, 1, 1);

	//TransitionBarrierを張る
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	uavBarrier.Transition.pResource = gpuParticle->GetParticleUavResource();
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &uavBarrier);
}