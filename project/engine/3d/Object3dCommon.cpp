#include "Object3dCommon.h"

#include <numbers>
#include <algorithm>
#include "engine/base/DirectXCommon.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include "engine/math/Vector3Math.h"
#include "engine/3d/Light/LightManager.h"

//================================================================================================
// インスタンスの取得
//================================================================================================
Object3dCommon& Object3dCommon::GetInstance() {
	static Object3dCommon instance;
	return instance;
}

//================================================================================================
// 初期化
//================================================================================================
void Object3dCommon::Initialize(TakeC::DirectXCommon* directXCommon,TakeC::LightManager* lightManager) {

	//DirectXCommon取得
	dxCommon_ = directXCommon;
	//LightManager取得
	lightManager_ = lightManager;
	//PSO生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"3d/Object3d.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"3d/Object3d.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("Object3dPSO:graphic");
	pso_->CompileComputeShader(dxCommon_->GetDXC(), L"3d/Skinning.CS.hlsl");
	pso_->CreateComputePSO(dxCommon_->GetDevice());
	pso_->SetComputePipelineName("Object3dPSO:Conpute");

	//加算ブレンド用PSO生成
	addBlendPso_ = std::make_unique<PSO>();
	addBlendPso_->CompileVertexShader(dxCommon_->GetDXC(), L"3d/Object3d.VS.hlsl");
	addBlendPso_->CompilePixelShader(dxCommon_->GetDXC(), L"3d/Object3d.PS.hlsl");
	addBlendPso_->CreateGraphicPSO(dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL, BlendState::ADD);
	pso_->SetGraphicPipelineName("Object3dPSO:graphic:AddBlend");

	//影描画用PSO生成
	shadowPassPso_ = std::make_unique<PSO>();
	shadowPassPso_->CompileVertexShader(dxCommon_->GetDXC(), L"Shadow/ShadowPass.VS.hlsl");
	shadowPassPso_->CompilePixelShader(dxCommon_->GetDXC(), L"Shadow/ShadowPass.PS.hlsl");
	shadowPassPso_->CreateGraphicPSO(dxCommon_->GetDevice(),D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	shadowPassPso_->SetGraphicPipelineName("Object3dPSO:graphic:ShadowPass");

	//RootSignatureの取得
	graphicRootSignature_ = pso_->GetGraphicRootSignature();
	computeRootSignature_ = pso_->GetComputeRootSignature();
	addBlendRootSignature_ = addBlendPso_->GetGraphicRootSignature();

	//影描画用RootSignatureの取得
	shadowPassRootSignature_ = shadowPassPso_->GetGraphicRootSignature();

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
	shadowPassRootSignature_.Reset();
	addBlendRootSignature_.Reset();
	pso_.reset();
	shadowPassPso_.reset();
	addBlendPso_.reset();
	dxCommon_ = nullptr;
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
// 影描画前処理
//================================================================================================
void Object3dCommon::PreDrawShadowPass() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(shadowPassPso_->GetGraphicPipelineState());
	// ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(shadowPassRootSignature_.Get());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
		pso->GetGraphicBindResourceIndex("gCamera"), TakeC::CameraManager::GetInstance().GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

}
