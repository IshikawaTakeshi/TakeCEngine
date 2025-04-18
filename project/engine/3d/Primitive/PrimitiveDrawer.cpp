#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ImGuiManager.h"
#include "MatrixMath.h"
#include "Particle/Particle3d.h"
#include <numbers>

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	////PSOの生成
	//pso_ = std::make_unique<PSO>();
	//pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Resources/shaders/Particle.VS.hlsl");
	//pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Resources/shaders/Particle.PS.hlsl");
	////PSOの生成
	//pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO,PSO::BlendState::ADD);
	//rootSignature_ = pso_->GetGraphicRootSignature();

	//wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	//wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	////particleResource
	//srvIndex_ = srvManager_->Allocate();
	//particleResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParticleForGPU));
	//srvManager_->CreateSRVforStructuredBuffer(
	//	1,
	//	sizeof(ParticleForGPU),
	//	particleResource_.Get(),
	//	srvIndex_
	//);
	//particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));
	//particleData_ = new ParticleForGPU();
	////perview
	//perviewResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PerView));
	//perviewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	//perViewData_ = new PerView();
	//TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();
	//TransformMatrixData_->World = MatrixMath::MakeIdentity4x4();
	//TransformMatrixData_->WorldInverseTranspose = MatrixMath::MakeIdentity4x4();

	ringData_ = new RingData();
	CreateRingVertexData();
}

void PrimitiveDrawer::Finalize() {

	if (ringData_) {
		ringData_->primitiveData_.vertexResource_.Reset();
		delete ringData_;
		ringData_ = nullptr;
	}

	//wvpResource_.Reset();

	//rootSignature_.Reset();
	//pso_.reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PrimitiveDrawer::Update() {

	////アフィン行列の更新
	//TransformMatrixData_->World = MatrixMath::MakeAffineMatrix(
	//	transform_.scale,
	//	transform_.rotate,
	//	transform_.translate
	//);
	//TransformMatrixData_->WVP = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
	//TransformMatrixData_->WorldInverseTranspose = MatrixMath::InverseTranspose(TransformMatrixData_->World);

	////perview
	//perViewData_->viewProjection = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
	//perViewData_->billboardMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetRotationMatrix();
}

void PrimitiveDrawer::UpdateImGui() {

	ImGui::Begin("PrimitiveDrawer");
	ImGui::DragFloat3("RingScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("RingRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("RingTranslate", &transform_.translate.x, 0.01f);
	ringData_->material_->UpdateMaterialImGui();
	ImGui::End();
}

void PrimitiveDrawer::GenerateRing(const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color) {

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringDivide_);

	for (uint32_t index = 0; index < ringDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringDivide_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide_);

		ringData_->vertexData_[ringVertexIndex_ + 0].position = { center.x + outerRadius * -sin,     center.y + outerRadius * cos,     center.z, 1.0f }; // outer[i]
		ringData_->vertexData_[ringVertexIndex_ + 1].position = { center.x + innerRadius * -sin,     center.y + innerRadius * cos,     center.z, 1.0f }; // inner[i]
		ringData_->vertexData_[ringVertexIndex_ + 2].position = { center.x + innerRadius * -sinNext, center.y + innerRadius * cosNext, center.z, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex_ + 3].position = ringData_->vertexData_[ringVertexIndex_].position; // outer[i]
		ringData_->vertexData_[ringVertexIndex_ + 4].position = ringData_->vertexData_[ringVertexIndex_ + 2].position; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex_ + 5].position = { center.x + outerRadius * -sinNext, center.y + outerRadius * cosNext, center.z, 1.0f }; // outer[i+1]

		ringData_->vertexData_[ringVertexIndex_ + 0].texcoord = { uv, 0.0f };     // outer[i]
		ringData_->vertexData_[ringVertexIndex_ + 1].texcoord = { uv, 1.0f };     // inner[i]
		ringData_->vertexData_[ringVertexIndex_ + 2].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex_ + 3].texcoord = { uv, 0.0f };     // outer[i]
		ringData_->vertexData_[ringVertexIndex_ + 4].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData_->vertexData_[ringVertexIndex_ + 5].texcoord = { uvNext, 0.0f }; // outer[i+1]

		ringData_->vertexData_[ringVertexIndex_].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 1].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 2].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 3].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 4].normal = { 0.0f, 0.0f, 1.0f };
		ringData_->vertexData_[ringVertexIndex_ + 5].normal = { 0.0f, 0.0f, 1.0f };

		ringVertexIndex_ += 6;
	}

	ringData_->material_->SetMaterialColor(color);

	//particleData_[0].translate = { center.x, center.y, center.z };
	//particleData_[0].rotate = { 0.0f, 0.0f, 0.0f };
	//particleData_[0].scale = { outerRadius, outerRadius, outerRadius };
	//particleData_[0].color = { color.x, color.y, color.z };
	//particleData_[0].lifeTime = 100.0f;
	//particleData_[0].currentTime = 0.0f;
}

void PrimitiveDrawer::DrawParticle(PSO* pso,UINT instanceCount) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//--------------------------------------------------
	//		ringの描画
	//--------------------------------------------------

	//commandList->SetGraphicsRootSignature(rootSignature_.Get());

	//commandList->SetPipelineState(pso_->GetGraphicPipelineState());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//VertexBufferView
	commandList->IASetVertexBuffers(0, 1, &ringData_->primitiveData_.vertexBufferView_);
	// materialResource
	commandList->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gMaterial"), ringData_->material_->GetMaterialResource()->GetGPUVirtualAddress());
	// texture
	srvManager_->SetGraphicsRootDescriptorTable(pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(ringData_->material_->GetTextureFilePath()));
	
	//描画
	commandList->DrawInstanced(ringVertexIndex_, instanceCount, 0, 0);
}

void PrimitiveDrawer::CreateRingVertexData() {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData_->primitiveData_.vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * ringDivide_ * kMaxVertexCount_);
	//bufferview設定
	ringData_->primitiveData_.vertexBufferView_.BufferLocation = ringData_->primitiveData_.vertexResource_->GetGPUVirtualAddress();
	ringData_->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData_->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData_->primitiveData_.vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&ringData_->vertexData_));

	ringData_->material_ = new Material();
	ringData_->material_->Initialize(dxCommon_, "Resources/images/gradationLine.png");
	ringData_->material_->SetEnableLighting(false);
	ringData_->material_->SetEnvCoefficient(0.0f);
}
