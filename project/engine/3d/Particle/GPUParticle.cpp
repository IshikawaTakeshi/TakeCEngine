#include "GPUParticle.h"
#include "ParticleCommon.h"
#include "ModelManager.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"

GPUParticle::~GPUParticle() {

	particleResource_.Reset();
	perViewResource_.Reset();
}

void GPUParticle::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//モデルの読み込み
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	//ParticleResource生成
	particleResource_ = DirectXCommon::CreateBufferResource(
		particleCommon_->GetDirectXCommon()->GetDevice(),sizeof(ParticleForCS) * kNumMaxInstance_);
	
	//PerViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));

	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));

	//SRVリソース
	particleSrvIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForCS),
		particleResource_.Get(),
		0
	);

	//PerViewData初期化
	perViewData_->viewProjection = MatrixMath::MakeIdentity4x4();
	perViewData_->billboardMatrix = MatrixMath::MakeIdentity4x4();

	camera_ = particleCommon_->GetDefaultCamera();

	//初期化
	particleCommon_->DispatchForGPUParticle();
}

void GPUParticle::Update() {


	perViewData_->viewProjection = camera_->GetViewProjectionMatrix();
	perViewData_->billboardMatrix = camera_->GetRotationMatrix();
}

void GPUParticle::Draw() {

	//perViewResource
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, perViewResource_->GetGPUVirtualAddress());

	//particleResource
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(3, particleSrvIndex_);

	model_->DrawForGPUParticle(kNumMaxInstance_);
}

void GPUParticle::DisPatchInitializeParticle() {



	particleCommon_->DispatchForGPUParticle();

	// UAV から Vertex Buffer へのバリア
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	//今回のバリアはTransition
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	uavBarrier.Transition.pResource = particleResource_.Get();
	//遷移前(現在)のResourceState
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	//遷移後のResourceState
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	//TransitionBarrierを張る
	particleCommon_->GetDirectXCommon()->GetCommandList()->ResourceBarrier(1, &uavBarrier);


	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(0, particleSrvIndex_);

	particleCommon_->GetDirectXCommon()->GetCommandList()->Dispatch(1, 1, 1);

	
	//今回のバリアはTransition
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	uavBarrier.Transition.pResource = particleResource_.Get();
	//遷移前(現在)のResourceState
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	//遷移後のResourceState
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	//TransitionBarrierを張る
	particleCommon_->GetDirectXCommon()->GetCommandList()->ResourceBarrier(1, &uavBarrier);
}
