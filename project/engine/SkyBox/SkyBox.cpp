#include "SkyBox.h"
#include "camera/Camera.h"
#include "camera/CameraManager.h"
#include "math/MatrixMath.h"
#include "3d/Model.h"
#include "base/TakeCFrameWork.h"
#include "base/ModelManager.h"
#include "base/PipelineStateObject.h"

//=============================================================================
// 初期化
//=============================================================================
void SkyBox::Initialize(TakeC::DirectXCommon* directXCommon,const std::string& texturefilePath) {

	dxCommon_ = directXCommon;

	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("SkyBoxPSO");
	//RootSignatureの生成
	rootSignature_ = pso_->GetGraphicRootSignature();

	//モデルの生成
	primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateCube(
		{ {-50.0f,-50.0f,-50.0f},{50.0f,50.0f,50.0f} }, texturefilePath
	);
	
	//TransformationMatrix用のResource生成
	wvpResource_ = TakeC::DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->SetName(L"SkyBox::wvpResource_");
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//CPUで動かす用のTransform
	transform_ = { {100.0f,100.0f,100.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = TakeC::CameraManager::GetInstance().GetActiveCamera();
}

//=============================================================================
// 更新処理
//=============================================================================
void SkyBox::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			TakeC::CameraManager::GetInstance().GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}

	//ワールド行列の逆行列の転置行列を計算
	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);

	//GPUに使うデータを転送
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void SkyBox::UpdateImGui() {

}

//=============================================================================
// 描画処理
//=============================================================================
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

	//プリミティブ描画
	TakeCFrameWork::GetPrimitiveDrawer()->DrawAllObject(
		pso_.get(),
		PRIMITIVE_CUBE,
		primitiveHandle_
	);
}

void SkyBox::SetMaterialColor(const Vector4& color) {

	TakeCFrameWork::GetPrimitiveDrawer()->SetMaterialColor(
		primitiveHandle_,
		PRIMITIVE_CUBE,
		{ color.x,color.y,color.z,1.0f }
	);
}