#include "SkyBox.h"
#include "camera/Camera.h"
#include "camera/CameraManager.h"
#include "base/PipelineStateObject.h"
#include "base/ModelManager.h"
#include "base/TakeCFrameWork.h"
#include "3d/Primitive/PrimitiveDrawer.h"
#include "3d/Model.h"
#include "math/MatrixMath.h"

SkyBox::~SkyBox() {
	pso_.reset();
	dxCommon_ = nullptr;
}

void SkyBox::Initialize(DirectXCommon* directXCommon,const std::string& filename) {

	dxCommon_ = directXCommon;

	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);

	//RootSignatureの生成
	rootSignature_ = pso_->GetGraphicRootSignature();
	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->SetName(L"SkyBox::wvpResource_");
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//CPUで動かす用のTransform
	transform_ = { {50.0f,50.0f,50.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = CameraManager::GetInstance()->GetActiveCamera();

	//プリミティブタイプ設定
	primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateBox({500.0f,500.0f,500.0f}, filename);
}

void SkyBox::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}

	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

void SkyBox::Draw() {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ルートシグネチャ設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	commandList->SetPipelineState(pso_->GetGraphicPipelineState());


	//TransformationMatrix
	commandList->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());

	TakeCFrameWork::GetPrimitiveDrawer()->DrawSkyBox(pso_.get(), primitiveHandle_);
}
