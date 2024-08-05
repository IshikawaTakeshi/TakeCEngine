#include "../Include/PipelineStateObject.h"
#include "../Include/Logger.h"
#include "../Include/DirectXCommon.h"
#include "../Include/DirectXShaderCompiler.h"
#include <cassert>

#pragma region PSO

PSO::~PSO() {}

void PSO::Finalize() {

	graphicPipelineState_.Reset();
	pixelShaderBlob_.Reset();
	vertexShaderBlob_.Reset();
	rootSignature_.Reset();
	if (errorBlob_) {
		errorBlob_.Reset();
	}
	signatureBlob_.Reset();

}

void PSO::CreateRootSignature(ID3D12Device* device) {

	HRESULT result = S_FALSE;
	//ルートシグネチャ
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプターレンジ
	descriptorRange_[0].BaseShaderRegister = 0;
	descriptorRange_[0].NumDescriptors = 1;
	descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange_[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

	//ルートパラメータ。複数設定できるので配列。
	//.0
	rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	//.1
	rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters_[1].Descriptor.ShaderRegister = 0;
	//.2
	rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange_; //Tableの中身の配列を指定
	rootParameters_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_); //Tableで利用する数
	//.3 平行光源をShaderで使う
	rootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	rootParameters_[3].Descriptor.ShaderRegister = 1; //レジスタ番号1を使う

	descriptionRootSignature_.pParameters = rootParameters_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParameters_); //配列の長さ

	//Samplerの設定
	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //バイナリフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0~1の範囲外をリピート
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX; //ありったけのMipmapを使う
	staticSamplers_[0].ShaderRegister = 0; //レジスタ番号0を使う
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う

	descriptionRootSignature_.pStaticSamplers = staticSamplers_;
	descriptionRootSignature_.NumStaticSamplers = _countof(staticSamplers_);

	//シリアライズ
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;

	result = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(result)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	rootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_)
	);
	assert(SUCCEEDED(result));
}

void PSO::CreateInputLayout() {

	//position
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//texcoord
	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//normal
	inputElementDescs_[2].SemanticName = "NORMAL";
	inputElementDescs_[2].SemanticIndex = 0;
	inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);
}

void PSO::CreateBlendState() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
}

void PSO::CreateRasterizerState() {
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
}

void PSO::CreatePSO(ID3D12Device* device, DXC* dxc_) {

	HRESULT result = S_FALSE;

	/// ルートシグネチャ初期化
	CreateRootSignature(device);
	/// インプットレイアウト初期化
	CreateInputLayout();
	/// ブレンドステート初期化
	CreateBlendState();
	/// ラスタライザステート初期化
	CreateRasterizerState();

	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Object3D.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Object3D.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

	//Depthの機能を有効化
	depthStencilDesc_.DepthEnable = true;
	//書き込み
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//DepthStencilの設定
	graphicsPipelineStateDesc_.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc_.pRootSignature = rootSignature_.Get(); // RootSignature
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDesc_; // InputLayout

	graphicsPipelineStateDesc_.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc_.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() }; // PixelShader

	graphicsPipelineStateDesc_.BlendState = blendDesc_; // blendState
	graphicsPipelineStateDesc_.RasterizerState = rasterizerDesc_; // rasterizerState

	//書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトロポジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc_.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むのかの設定
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

#pragma endregion