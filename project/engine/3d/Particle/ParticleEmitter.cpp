#include "ParticleEmitter.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/SrvManager.h"
#include "engine/math/Easing.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/MatrixMath.h"
#include "3d/Particle/GPUParticle.h"
#include "2d/WireFrame.h"

//================================================================================================
// 初期化
//================================================================================================

void ParticleEmitter::Initialize(const std::string& emitterName, EulerTransform transforms, uint32_t count, float frequency) {
	//Emitter初期化
	emitterName_ = emitterName;
	transforms_.translate = transforms.translate;
	transforms_.rotate = transforms.rotate;
	transforms_.scale = transforms.scale;
	emitDirection_ = { 0.0f,0.0f,1.0f };
	particleCount_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;
	isEmit_ = false;
}

void ParticleEmitter::InitializeEmitterSphere(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//PSO生成
	emitParticlePso_ = std::make_unique<PSO>();
	emitParticlePso_->CompileComputeShader(dxCommon_->GetDXC(), L"EmitParticle.CS.hlsl");
	emitParticlePso_->CreateComputePSO(dxCommon_->GetDevice());
	emitParticlePso_->SetComputePipelineName("EmitParticlePSO");
	//RootSignature生成
	emitParticleRootSignature_ = emitParticlePso_->GetComputeRootSignature();

	//EmitterSphereResource生成
	emitterSphereResource_ = 
		TakeC::DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(EmitterSphereInfo));
	emitterSphereResource_->SetName(L"EmitterSphereInfo::emitterSphereResource_");
	//PerFrameResource生成
	perFrameResource_ = 
		TakeC::DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PerFrame));
	perFrameResource_->SetName(L"EmitterSphereInfo::perFrameResource_");

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

//==================================================================================
// 更新処理
//==================================================================================

void ParticleEmitter::Update() {

	ParticlePreset preset = TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset();
	maxInterpolationCount_ = preset.attribute.particlesPerInterpolation; // 最大補間回数

	//transform.rotateによってDirectionを更新
	Matrix4x4 rotateMatrix = MatrixMath::MakeRotateMatrix(transforms_.rotate);
	emitDirection_ = MatrixMath::Transform({ 0.0f,0.0f,1.0f }, rotateMatrix);

	//エミッターの更新
	if (!isEmit_) return;
	frequencyTime_ += TakeCFrameWork::GetDeltaTime();
	if (frequency_ <= frequencyTime_) {

		if (TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.isEmitterTrail == true) {
			// 移動ベクトルと距離
			Vector3 moveVector = transforms_.translate - prevTranslate_;
			float moveDistance = Vector3Math::Length(moveVector);

			// 移動距離に応じた補間回数を計算
			// 例: 1. 0単位ごとに1回補間
			const float interpolationUnit = 1.0f; // 調整可能
			int interpolationCount = static_cast<int>(moveDistance / interpolationUnit);

			// 最大補間回数の制限（パフォーマンス対策）
			interpolationCount = std::min(interpolationCount, maxInterpolationCount_);

			if (interpolationCount > 0) {
				// 等間隔で補間位置から発生
				for (int i = 0; i < interpolationCount; ++i) {
					float t = static_cast<float>(i + 1) / (interpolationCount + 1);
					Vector3 interpolatedPos = prevTranslate_ + moveVector * t;
					Emit(interpolatedPos);
				}
			}
		}
		Emit();
		frequencyTime_ -= frequency_; //余計に過ぎた時間も加味して頻度計算する
		prevTranslate_ = transforms_. translate;
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

//==================================================================================
// ImGuiの更新
//==================================================================================

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

//==================================================================================
// 描画処理
//==================================================================================

void ParticleEmitter::DrawWireFrame() {

	TakeCFrameWork::GetWireFrame()->DrawSphere(transforms_.translate, 0.3f, { 1.0f, 0.0f, 1.0f, 1.0f });
}

//==================================================================================
// パーティクルの発生
//==================================================================================

void ParticleEmitter::Emit() {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, transforms_.translate,emitDirection_, particleCount_);
}

void ParticleEmitter::Emit(const Vector3& position) {
	TakeCFrameWork::GetParticleManager()->Emit(particleName_, position,emitDirection_, particleCount_);
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
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		emitParticlePso_->GetComputeBindResourceIndex("gPerFrame"), perFrameResource_->GetGPUVirtualAddress());
	//1.EmitterSphereInfo
	srvManager_->SetComputeRootDescriptorTable(
		emitParticlePso_->GetComputeBindResourceIndex("gEmitterSphere"), emitterSphereSrvIndex_);
	//2.Particle
	srvManager_->SetComputeRootDescriptorTable(
		emitParticlePso_->GetComputeBindResourceIndex("gParticles"), gpuParticle->GetParticleUavIndex());
	//3.FreeListIndex
	srvManager_->SetComputeRootDescriptorTable(
		emitParticlePso_->GetComputeBindResourceIndex("gFreeListIndex"), gpuParticle->GetFreeListIndexUavIndex());
	//4.FreeList
	srvManager_->SetComputeRootDescriptorTable(
		emitParticlePso_->GetComputeBindResourceIndex("gFreeList"), gpuParticle->GetFreeListUavIndex());
	//particleAttributes
	srvManager_->SetComputeRootDescriptorTable(
		emitParticlePso_->GetComputeBindResourceIndex("gAttributes"), gpuParticle->GetAttributeSrvIndex());
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

//==================================================================================
// パーティクル名の設定
//==================================================================================

void ParticleEmitter::SetParticleName(const std::string& particleName) {
	particleName_ = particleName;
	frequency_ = TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.frequency;
	particleCount_ = TakeCFrameWork::GetParticleManager()->GetParticleGroup(particleName_)->GetPreset().attribute.emitCount;
}