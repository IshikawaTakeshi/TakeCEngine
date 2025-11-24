#include "Object3dCommon.h"

#include <numbers>
#include <algorithm>
#include "engine/base/DirectXCommon.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include "engine/math/Vector3Math.h"
#include "engine/3d/Light/LightManager.h"

//シングルトンインスタンスの初期化
Object3dCommon* Object3dCommon::instance_ = nullptr;

//================================================================================================
// インスタンスの取得
//================================================================================================
Object3dCommon* Object3dCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Object3dCommon();
	}
	return instance_;
	
}

//================================================================================================
// 初期化
//================================================================================================
void Object3dCommon::Initialize(DirectXCommon* directXCommon,LightManager* lightManager) {

	//DirectXCommon取得
	dxCommon_ = directXCommon;
	//LightManager取得
	lightManager_ = lightManager;
	//PSO生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Object3d.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Object3d.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("Object3dPSO:graphic");
	pso_->CompileComputeShader(dxCommon_->GetDXC(), L"Skinning.CS.hlsl");
	pso_->CreateComputePSO(dxCommon_->GetDevice());
	pso_->SetComputePipelineName("Object3dPSO:Conpute");
	//加算ブレンド用PSO生成
	addBlendPso_ = std::make_unique<PSO>();
	addBlendPso_->CompileVertexShader(dxCommon_->GetDXC(), L"Object3d.VS.hlsl");
	addBlendPso_->CompilePixelShader(dxCommon_->GetDXC(), L"Object3d.PS.hlsl");
	addBlendPso_->CreateGraphicPSO(dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL, BlendState::ADD);
	pso_->SetGraphicPipelineName("Object3dPSO:graphic:AddBlend");
	//RootSignatureの取得
	graphicRootSignature_ = pso_->GetGraphicRootSignature();
	computeRootSignature_ = pso_->GetComputeRootSignature();
	addBlendRootSignature_ = addBlendPso_->GetGraphicRootSignature();

	//TODO:ライト関連の初期化処理を別のクラスに移動させる
#pragma region "Lighting"

	directionalLightData_ = std::make_unique<DirectionalLightData>();
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-0.05f,-0.95f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;

#pragma endregion
}

//================================================================================================
// ImGuiの更新
//================================================================================================
void Object3dCommon::UpdateImGui() {
	lightManager_->UpdateImGui();
}

//================================================================================================
// 終了・開放処理
//================================================================================================
void Object3dCommon::Finalize() {

	graphicRootSignature_.Reset();
	computeRootSignature_.Reset();
	pso_.reset();
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}

//================================================================================================
// 描画前処理
//================================================================================================
void Object3dCommon::PreDraw() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	// ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignature_.Get());
	
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// カメラ情報設定
	SetCBufferViewCamera(pso_.get());
	// ライトリソース設定
	lightManager_->SetLightResources(pso_.get());
}

//================================================================================================
// 描画前処理(加算ブレンド)
//================================================================================================
void Object3dCommon::PreDrawAddBlend() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(addBlendPso_->GetGraphicPipelineState());
	// ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(addBlendRootSignature_.Get());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// カメラ情報設定
	SetCBufferViewCamera(addBlendPso_.get());
	// ライトリソース設定
	lightManager_->SetLightResources(addBlendPso_.get());
}

//================================================================================================
// コンピュートシェーダーのディスパッチ前処理
//================================================================================================
void Object3dCommon::Dispatch() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetComputePipelineState());
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
}

ID3D12Resource* Object3dCommon::GetDirectionalLightResource() const {
	return lightManager_->GetDirectionalLightResource();
}

//================================================================================================
// カメラ情報用CBufferView設定
//================================================================================================
void Object3dCommon::SetCBufferViewCamera(PSO* pso) {
	//カメラ情報のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gCamera"), CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

}