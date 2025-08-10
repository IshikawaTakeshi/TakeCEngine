#include "GPUParticle.h"
#include "CameraManager.h"
#include "ParticleCommon.h"
#include "ModelManager.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "ImGuiManager.h"
#include "TakeCFrameWork.h"
#include "Utility/ResourceBarrier.h"

GPUParticle::~GPUParticle() {
	particleUavResource_.Reset();
	perViewResource_.Reset();
	perFrameResource_.Reset();
	freeListIndexResource_.Reset();
	freeListResource_.Reset();
}

//==================================================================================
//		Initialize
//==================================================================================

void GPUParticle::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//ParticleResource生成
	particleUavResource_ = DirectXCommon::CreateBufferResourceUAV(
		particleCommon_->GetDirectXCommon()->GetDevice(),sizeof(ParticleForCS) * kNumMaxInstance_,
		particleCommon_->GetDirectXCommon()->GetCommandList());
	particleUavResource_->SetName(L"GPUParticle::particleUavResource_");

	//PerViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
	perViewResource_->SetName(L"GPUParticle::perViewResource_");

	//PerFrameResource生成
	perFrameResource_ = DirectXCommon::CreateBufferResource(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerFrame));
	perFrameResource_->SetName(L"GPUParticle::perFrameResource_");

	//freeListIndexResource生成
	freeListIndexResource_ = DirectXCommon::CreateBufferResourceUAV(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(uint32_t),
		particleCommon_->GetDirectXCommon()->GetCommandList());
	freeListIndexResource_->SetName(L"GPUParticle::freeListIndexResource_");

	//freeListResource生成
	freeListResource_ = DirectXCommon::CreateBufferResourceUAV(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(uint32_t) * kNumMaxInstance_,
		particleCommon_->GetDirectXCommon()->GetCommandList());
	freeListResource_->SetName(L"GPUParticle::freeListResource_");

	//Mapping
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	perFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));

	//particleURVリソース
	particleUavIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForCS),
		particleUavResource_.Get(),
		particleUavIndex_
	);
	//freeListIndexリソース
	freeListIndexUavIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
		1,
		sizeof(uint32_t),
		freeListIndexResource_.Get(),
		freeListIndexUavIndex_
	);
	//freeListリソース
	freeListUavIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(uint32_t),
		freeListResource_.Get(),
		freeListUavIndex_
	);

	//PerViewData初期化
	perViewData_->viewProjection = MatrixMath::MakeIdentity4x4();
	perViewData_->billboardMatrix = MatrixMath::MakeIdentity4x4();
	//PerFrameData初期化
	perFrameData_->gameTime = TakeCFrameWork::GetGameTime();
	perFrameData_->deltaTime = TakeCFrameWork::GetDeltaTime();

	camera_ = particleCommon_->GetDefaultCamera();

	//attributeの初期化
	attributeResource_ = DirectXCommon::CreateBufferResource(
		particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleAttributes) * kNumMaxInstance_);
	attributeResource_->SetName(L"GPUParticle::attributeResource_");
	
	//Mapping
	attributeResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleAttributes_));

	//SRVの生成
	attributeSrvIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_, sizeof(ParticleAttributes),
		attributeResource_.Get(), attributeSrvIndex_);

	particleAttributes_ = &particlePreset_.attribute;


	if (particlePreset_.primitiveType == PRIMITIVE_RING) {
		//プリミティブの初期化
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(1.0f, 0.5f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_PLANE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(1.0f, 1.0f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_SPHERE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateSphere(1.0f, filePath);
	} else {
		assert(0 && "未対応の PrimitiveType が指定されました");
	}

	//初期化
	DisPatchInitializeParticle();
}

//==================================================================================
//		Update
//==================================================================================

void GPUParticle::Update() {

	//cameraの情報取得
	camera_ = CameraManager::GetInstance()->GetActiveCamera();

	//CSによる更新処理
	DisPatchUpdateParticle();

	perFrameData_->gameTime = TakeCFrameWork::GetGameTime();

	perViewData_->viewProjection = camera_->GetViewProjectionMatrix();
	perViewData_->billboardMatrix = camera_->GetRotationMatrix();
}

//==================================================================================
//		Draw
//==================================================================================

void GPUParticle::Draw() {

	//perViewResource
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, perViewResource_->GetGPUVirtualAddress());


	//particleResource
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(3, particleUavIndex_);

	TakeCFrameWork::GetPrimitiveDrawer()->DrawParticle(particleCommon_->GetGraphicPSOForGPUParticle(),
		kNumMaxInstance_, particlePreset_.primitiveType, primitiveHandle_);
}

//==================================================================================
//		DisPatchInitializeParticle
//==================================================================================

void GPUParticle::DisPatchInitializeParticle() {

	//ComputePSOの設定
	particleCommon_->DispatchForGPUParticle();

	//MEMO:普段は描画処理前でいいが、今回は描画処理内ではないのでここでDescriptorHeapを設定する
	particleCommon_->GetSrvManager()->SetDescriptorHeap();

	// VERTEX_AND_CONSTANT_BUFFER -> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		particleUavResource_.Get());

	//0.particleUAV
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(0, particleUavIndex_);
	//1.freeList
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(2, freeListUavIndex_);
	//2.freeListIndex
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(1, freeListIndexUavIndex_);

	///=== Dispatch ===///
	particleCommon_->GetDirectXCommon()->GetCommandList()->Dispatch(1, 1, 1);
	
	//UNORDERED_ACCESS -> VERTEX_AND_CONSTANT_BUFFER
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		particleUavResource_.Get());
}

//==================================================================================
//		DisPatchUpdateParticle
//==================================================================================

void GPUParticle::DisPatchUpdateParticle() {

	//ComputePSOの設定
	particleCommon_->DispatchUpdateParticle();
	particleCommon_->GetSrvManager()->SetDescriptorHeap();

	//VERTEX_AND_CONSTANT_BUFFER -> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		particleUavResource_.Get());

	//1.perFrame
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetComputeRootConstantBufferView(0, perFrameResource_->GetGPUVirtualAddress());
	//0.particleUAV
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(1, particleUavIndex_);
	//2.freeList
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(3, freeListUavIndex_);
	//3.freeListIndex
	particleCommon_->GetSrvManager()->SetComputeRootDescriptorTable(2, freeListIndexUavIndex_);

	///=== Dispatch ===///
	particleCommon_->GetDirectXCommon()->GetCommandList()->Dispatch(1, 1, 1);

	//UNORDERED_ACCESS -> VERTEX_AND_CONSTANT_BUFFER
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		particleUavResource_.Get());
}