#include "PipelineStateObject.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "DirectXShaderCompiler.h"
#include "ImGuiManager.h"
#include "StringUtility.h"

#include <cassert>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <unordered_map>


PSO::~PSO() {

	pixelShaderBlob_.Reset();
	vertexShaderBlob_.Reset();
	graphicRootSignature_.Reset();
	if (errorBlob_) {
		errorBlob_.Reset();
	}
	signatureBlob_.Reset();
	graphicPipelineState_.Reset();
}

//=============================================================================
// RootSignatureの生成
//=============================================================================

void PSO::CreateRootSignatureFromShaders(
	ID3D12Device* device,
	const std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {

	std::unordered_map<ShaderResourceKey, ShaderResourceInfo, ShaderResourceKeyHash> resources;

	// シェーダーの可視性を取得する関数
	auto GetShaderVisibility = [&](D3D12_SHADER_DESC shaderDesc) {
		UINT shaderVersion = D3D12_SHVER_GET_TYPE(shaderDesc.Version);
		if (shaderVersion == D3D12_SHVER_VERTEX_SHADER) {

			return D3D12_SHADER_VISIBILITY_VERTEX;

		} else if (shaderVersion == D3D12_SHVER_PIXEL_SHADER) {

			return D3D12_SHADER_VISIBILITY_PIXEL;

		} else if (shaderVersion == D3D12_SHVER_COMPUTE_SHADER) {

			return D3D12_SHADER_VISIBILITY_ALL;

		} else {

			return D3D12_SHADER_VISIBILITY_ALL;
		}
	};

	// 各シェーダーをリフレクションしてリソースを収集
	for (size_t shaderIndex = 0; shaderIndex < shaderBlobs.size(); ++shaderIndex) {
		const auto& shaderBlob = shaderBlobs[shaderIndex];
		
		ComPtr<IDxcContainerReflection> containerReflection;
		HRESULT hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&containerReflection));
		if (FAILED(hr)) {
			std::cerr << "Failed to create IDxcContainerReflection." << std::endl;
			return;
		}

		hr = containerReflection->Load(shaderBlob.Get());
		if (FAILED(hr)) {
			std::cerr << "Failed to load shader blob into reflection." << std::endl;
			return;
		}

		UINT32 shaderIdx = 0;
		hr = containerReflection->FindFirstPartKind(DXC_PART_DXIL, &shaderIdx);
		if (FAILED(hr)) {
			std::cerr << "Failed to find DXIL part in shader blob." << std::endl;
			return;
		}

		ComPtr<ID3D12ShaderReflection> shaderReflection;
		hr = containerReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&shaderReflection));
		if (FAILED(hr)) {
			std::cerr << "Failed to get shader reflection." << std::endl;
			return;
		}


		D3D12_SHADER_DESC shaderDesc;
		shaderReflection->GetDesc(&shaderDesc);
	

		D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shaderDesc);

		// シェーダーのバインドリソースを収集
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			shaderReflection->GetResourceBindingDesc(i, &bindDesc);
			
			ShaderResourceKey key = { bindDesc.Type,visibility, bindDesc.BindPoint, bindDesc.Space };
			if (resources.find(key) == resources.end()) {
				resources[key] = { key, bindDesc.Name };
			}
		}
	}

	// ルートシグネチャ記述を構築
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	// ルートシグネチャのパラメータを構築
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;

	for (const auto& resource : resources) {
		const ShaderResourceKey& key = resource.second.key;

		if (key.type == D3D_SIT_CBUFFER) {
			// 定数バッファ
			D3D12_ROOT_PARAMETER rootParam = {};
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParam.ShaderVisibility = key.visibility;
			rootParam.Descriptor.ShaderRegister = key.bindPoint;
			//rootParam.Descriptor.RegisterSpace = key.space;

			rootParameters.push_back(rootParam);
		} else if (key.type == D3D_SIT_TEXTURE || key.type == D3D_SIT_STRUCTURED) {
			// テクスチャやストラクチャードバッファ
			D3D12_DESCRIPTOR_RANGE range = {};
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors = 1;
			range.BaseShaderRegister = key.bindPoint;
			//range.RegisterSpace = key.space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER rootParam = {};
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility = key.visibility;
			rootParam.DescriptorTable.pDescriptorRanges = &range;
			rootParam.DescriptorTable.NumDescriptorRanges = 1;

			rootParameters.push_back(rootParam);
		}
	}

	rootSigDesc.pParameters = rootParameters.data();
	rootSigDesc.NumParameters = static_cast<UINT>(rootParameters.size());

	//Samplerの設定
	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //バイナリフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0~1の範囲外をリピート
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX; //ありったけのMipmapを使う
	staticSamplers_[0].ShaderRegister = 0; //レジスタ番号0を使う
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う

	rootSigDesc.pStaticSamplers = staticSamplers_;
	rootSigDesc.NumStaticSamplers = _countof(staticSamplers_);

	// ルートシグネチャをシリアライズして作成
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob_,&errorBlob_);

	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	hr = device->CreateRootSignature(0,signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(),IID_PPV_ARGS(&graphicRootSignature_));

	if (FAILED(hr)) {
		Logger::Log(StringUtility::ConvertString(L"Failed to create root signature."));
		return;
	}

	Logger::Log(StringUtility::ConvertString(L"Root signature successfully created."));
}

void PSO::CreateRootSignatureForSprite(ID3D12Device* device) {

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
	rootParametersForSprite_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForSprite_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForSprite_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	//.1
	rootParametersForSprite_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParametersForSprite_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParametersForSprite_[1].Descriptor.ShaderRegister = 0;
	//.2
	rootParametersForSprite_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParametersForSprite_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForSprite_[2].DescriptorTable.pDescriptorRanges = descriptorRange_; //Tableの中身の配列を指定
	rootParametersForSprite_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_); //Tableで利用する数

	descriptionRootSignature_.pParameters = rootParametersForSprite_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParametersForSprite_); //配列の長さ

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
	graphicRootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&graphicRootSignature_)
	);
	assert(SUCCEEDED(result));
}

//=============================================================================
// rootSignatureの生成(Object3d)
//=============================================================================

void PSO::CreateRootSignatureForObject3D(ID3D12Device* device) {

	HRESULT result = S_FALSE;
	//ルートシグネチャ
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプターレンジ
	//.0 Texture
	descriptorRangeForObject3d_[0].BaseShaderRegister = 0;
	descriptorRangeForObject3d_[0].NumDescriptors = 1;
	descriptorRangeForObject3d_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForObject3d_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

	//.1 envMap
	descriptorRangeForObject3d_[1].BaseShaderRegister = 1;
	descriptorRangeForObject3d_[1].NumDescriptors = 1;
	descriptorRangeForObject3d_[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForObject3d_[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ。複数設定できるので配列。
	//.0 Material
	rootParametersForObject3d_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForObject3d_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForObject3d_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	//.1 TransformationMatrix
	rootParametersForObject3d_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParametersForObject3d_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParametersForObject3d_[1].Descriptor.ShaderRegister = 0;
	//.2 Texture
	rootParametersForObject3d_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParametersForObject3d_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForObject3d_[2].DescriptorTable.pDescriptorRanges = &descriptorRangeForObject3d_[0]; //Tableの中身の配列を指定
	rootParametersForObject3d_[2].DescriptorTable.NumDescriptorRanges = 1; //Tableで利用する数
	//.3 DirectionalLight
	rootParametersForObject3d_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForObject3d_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	rootParametersForObject3d_[3].Descriptor.ShaderRegister = 1; //レジスタ番号1
	//.4 カメラの情報
	rootParametersForObject3d_[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForObject3d_[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	rootParametersForObject3d_[4].Descriptor.ShaderRegister = 2; //レジスタ番号2
	//.5 PointLightのデータ
	rootParametersForObject3d_[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForObject3d_[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	rootParametersForObject3d_[5].Descriptor.ShaderRegister = 3; //レジスタ番号3
	//.6 SpotLightのデータ
	rootParametersForObject3d_[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForObject3d_[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	rootParametersForObject3d_[6].Descriptor.ShaderRegister = 4; //レジスタ番号4
	//.7 envMap
	rootParametersForObject3d_[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParametersForObject3d_[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForObject3d_[7].DescriptorTable.pDescriptorRanges = &descriptorRangeForObject3d_[1]; //Tableの中身の配列を指定
	rootParametersForObject3d_[7].DescriptorTable.NumDescriptorRanges = 1; //Tableで利用する数

	descriptionRootSignature_.pParameters = rootParametersForObject3d_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParametersForObject3d_); //配列の長さ

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
	graphicRootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&graphicRootSignature_)
	);
	assert(SUCCEEDED(result));
}

//=============================================================================
// graphicRootSignatureの生成(SkinningObject3D)
//=============================================================================

void PSO::CreateGraphicRootSignatureForSkinnedObject3D(ID3D12Device* device) {

	HRESULT result = S_FALSE;
	//ルートシグネチャ
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプターレンジ
	//.0 Texture
	graphicDescriptorRange_[0].BaseShaderRegister = 0;
	graphicDescriptorRange_[0].NumDescriptors = 1;
	graphicDescriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	graphicDescriptorRange_[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

	//.1 envMap
	graphicDescriptorRange_[1].BaseShaderRegister = 1;
	graphicDescriptorRange_[1].NumDescriptors = 1;
	graphicDescriptorRange_[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	graphicDescriptorRange_[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ。複数設定できるので配列。
	//.0 Material
	graphicRootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	graphicRootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	graphicRootParameters_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	//.1 TransformationMatrix
	graphicRootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	graphicRootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	graphicRootParameters_[1].Descriptor.ShaderRegister = 0;
	//.2 Texture
	graphicRootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	graphicRootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	graphicRootParameters_[2].DescriptorTable.pDescriptorRanges = &graphicDescriptorRange_[0]; //Tableの中身の配列を指定
	graphicRootParameters_[2].DescriptorTable.NumDescriptorRanges = 1; //Tableで利用する数
	//.3 DirectionalLight
	graphicRootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	graphicRootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	graphicRootParameters_[3].Descriptor.ShaderRegister = 1; //レジスタ番号1
	//.4 カメラの情報
	graphicRootParameters_[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	graphicRootParameters_[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	graphicRootParameters_[4].Descriptor.ShaderRegister = 2; //レジスタ番号2
	//.5 PointLightのデータ
	graphicRootParameters_[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	graphicRootParameters_[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	graphicRootParameters_[5].Descriptor.ShaderRegister = 3; //レジスタ番号3
	//.6 SpotLightのデータ
	graphicRootParameters_[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	graphicRootParameters_[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixcelShaderで使う
	graphicRootParameters_[6].Descriptor.ShaderRegister = 4; //レジスタ番号4
	//.7 envMap
	graphicRootParameters_[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	graphicRootParameters_[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	graphicRootParameters_[7].DescriptorTable.pDescriptorRanges = &graphicDescriptorRange_[1];
	graphicRootParameters_[7].DescriptorTable.NumDescriptorRanges = 1;
	//.8 outputedVertices
	graphicRootParameters_[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	graphicRootParameters_[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	graphicRootParameters_[8].DescriptorTable.pDescriptorRanges = &graphicDescriptorRange_[0];
	graphicRootParameters_[8].DescriptorTable.NumDescriptorRanges = 1;


	descriptionRootSignature_.pParameters = graphicRootParameters_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(graphicRootParameters_); //配列の長さ

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
	graphicRootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&graphicRootSignature_)
	);
	assert(SUCCEEDED(result));
}

//=============================================================================
// computeRootSignatureの生成(SkinningObject3D)
//=============================================================================


void PSO::CreateComputeRootSignatureForSkinnedObject3D(ID3D12Device* device) {

	HRESULT result = S_FALSE;
	//ルートシグネチャ
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプターレンジ
	//.0 pallete
	computeDescriptorRange_[0].BaseShaderRegister = 0;
	computeDescriptorRange_[0].NumDescriptors = 1;
	computeDescriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	computeDescriptorRange_[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

	//.1 InputVertices
	computeDescriptorRange_[1].BaseShaderRegister = 1;
	computeDescriptorRange_[1].NumDescriptors = 1;
	computeDescriptorRange_[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	computeDescriptorRange_[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//.2 Influence
	computeDescriptorRange_[2].BaseShaderRegister = 2;
	computeDescriptorRange_[2].NumDescriptors = 1;
	computeDescriptorRange_[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	computeDescriptorRange_[2].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//.3 OutputVertices
	computeDescriptorRange_[3].BaseShaderRegister = 0;
	computeDescriptorRange_[3].NumDescriptors = 1;
	computeDescriptorRange_[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	computeDescriptorRange_[3].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ。複数設定できるので配列。
	//.0 SkinningInfo
	computeRootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	computeRootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	computeRootParameters_[0].Descriptor.ShaderRegister = 0;
	//.1 Well
	computeRootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	computeRootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	computeRootParameters_[1].DescriptorTable.pDescriptorRanges = &computeDescriptorRange_[0];
	computeRootParameters_[1].DescriptorTable.NumDescriptorRanges = 1;
	//.2 InputVertices
	computeRootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	computeRootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	computeRootParameters_[2].DescriptorTable.pDescriptorRanges = &computeDescriptorRange_[1];
	computeRootParameters_[2].DescriptorTable.NumDescriptorRanges = 1;
	//.3 Influence
	computeRootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	computeRootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	computeRootParameters_[3].DescriptorTable.pDescriptorRanges = &computeDescriptorRange_[2];
	computeRootParameters_[3].DescriptorTable.NumDescriptorRanges = 1;
	//.4 OutputVertices
	computeRootParameters_[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	computeRootParameters_[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	computeRootParameters_[4].DescriptorTable.pDescriptorRanges = &computeDescriptorRange_[3];
	computeRootParameters_[4].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature_.pParameters = computeRootParameters_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(computeRootParameters_); //配列の長さ

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
	computeRootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature_)
	);
	assert(SUCCEEDED(result));
}

//=============================================================================
// rootSignatureの生成(SkyBox)
//=============================================================================

void PSO::CreateRootSignatureForParticle(ID3D12Device* device) {

	HRESULT result = S_FALSE;
	//デスクリプション
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ディスクリプターレンジ
	//.0 particle/Texture
	descriptorRangeForInstancing_[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing_[0].NumDescriptors = 1;
	descriptorRangeForInstancing_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータ。複数設定できるので配列。
	//.0 Material
	rootParametersForParticle_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParametersForParticle_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForParticle_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	//.1 Particle
	rootParametersForParticle_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParametersForParticle_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParametersForParticle_[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing_;
	rootParametersForParticle_[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing_);
	//.2 Texture
	rootParametersForParticle_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParametersForParticle_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParametersForParticle_[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing_; //Tableの中身の配列を指定
	rootParametersForParticle_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing_); //Tableで利用する数

	descriptionRootSignature_.pParameters = rootParametersForParticle_; //rootParameter配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParametersForParticle_); //配列の長さ

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
	graphicRootSignature_ = nullptr;
	result = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&graphicRootSignature_)
	);
	assert(SUCCEEDED(result));
}

//=============================================================================
// InputLayoutの生成
//=============================================================================

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

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

void PSO::CreateInputLayoutForSkyBox() {

	//position
	inputElementDescsForSkyBox_[0].SemanticName = "POSITION";
	inputElementDescsForSkyBox_[0].SemanticIndex = 0;
	inputElementDescsForSkyBox_[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescsForSkyBox_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	//texcoord
	inputElementDescsForSkyBox_[1].SemanticName = "TEXCOORD";
	inputElementDescsForSkyBox_[1].SemanticIndex = 0;
	inputElementDescsForSkyBox_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescsForSkyBox_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescsForSkyBox_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescsForSkyBox_.size());

}

void PSO::CreateInputLayoutForSkinningObject() {

	//position
	inputElementDescsForSkinningObject_[0].SemanticName = "POSITION";
	inputElementDescsForSkinningObject_[0].SemanticIndex = 0;
	inputElementDescsForSkinningObject_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescsForSkinningObject_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//texcoord
	inputElementDescsForSkinningObject_[1].SemanticName = "TEXCOORD";
	inputElementDescsForSkinningObject_[1].SemanticIndex = 0;
	inputElementDescsForSkinningObject_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescsForSkinningObject_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//normal
	inputElementDescsForSkinningObject_[2].SemanticName = "NORMAL";
	inputElementDescsForSkinningObject_[2].SemanticIndex = 0;
	inputElementDescsForSkinningObject_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescsForSkinningObject_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescsForSkinningObject_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescsForSkinningObject_.size());
}

//=============================================================================
// BlendStateの生成
//=============================================================================

void PSO::CreateBlendStateForObject3d() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].BlendEnable = true;
	blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

}

void PSO::CreateBlendStateForParticle() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].BlendEnable = true;
	blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
}

void PSO::CreateBlendStateForSprite() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

//=============================================================================
// RasterizerStateの生成
//=============================================================================

void PSO::CreateRasterizerState(D3D12_FILL_MODE fillMode) {
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc_.FillMode = fillMode;
}

//=============================================================================
// PSOの生成
//=============================================================================

void PSO::CreatePSOForSprite(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {
	HRESULT result = S_FALSE;

	device_ = device;

	itemCurrentIdx = 0;


	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Sprite.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Sprite.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

	/// ルートシグネチャ初期化
	CreateRootSignatureFromShaders(device, { vertexShaderBlob_, pixelShaderBlob_ });
	//CreateRootSignatureForSprite(device);
	/// インプットレイアウト初期化
	CreateInputLayout();
	/// ブレンドステート初期化
	CreateBlendStateForSprite();
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);

#pragma region SetDepthStencilDesc
	//Depthの機能を有効化
	depthStencilDesc_.DepthEnable = true;
	//書き込み
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

#pragma endregion

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc();

	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::CreatePSOForObject3D(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {

	HRESULT result = S_FALSE;

	device_ = device;

	itemCurrentIdx = 0;

	/// ルートシグネチャ初期化
	CreateRootSignatureForObject3D(device_);
	/// インプットレイアウト初期化
	CreateInputLayout();
	/// ブレンドステート初期化
	CreateBlendStateForObject3d();
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);

	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Object3D.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> shaderReflection;

	////シェーダーのリフレクションを取得
	//result = D3DReflect(vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize(), IID_PPV_ARGS(&shaderReflection));

	////シェーダー情報の取得
	//D3D12_SHADER_DESC shaderDesc;
	//shaderReflection->GetDesc(&shaderDesc);

	//for(UINT i = 0; i < shaderDesc.BoundResources; i++) {
	//	D3D12_SHADER_INPUT_BIND_DESC bindDesc;
	//	shaderReflection->GetResourceBindingDesc(i, &bindDesc);

	//	if(bindDesc.Type == D3D_SIT_CBUFFER) {
	//		if(bindDesc.Name == "TransformationMatrix") {
	//			rootParametersForObject3d_[1].Descriptor.ShaderRegister = bindDesc.BindPoint;
	//		}
	//	}
	//	else if(bindDesc.Type == D3D_SIT_TEXTURE) {
	//		if(bindDesc.Name == "Texture") {
	//			descriptorRangeForObject3d_[0].BaseShaderRegister = bindDesc.BindPoint;
	//		}
	//		else if(bindDesc.Name == "envMap") {
	//			descriptorRangeForObject3d_[1].BaseShaderRegister = bindDesc.BindPoint;
	//		}
	//	}
	//}

#pragma region SetDepthStencilDesc
	//Depthの機能を有効化
	depthStencilDesc_.DepthEnable = true;
	//書き込み
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

#pragma endregion

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc();

	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::CreatePSOForSkinningObject3D(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {

	HRESULT result = S_FALSE;

	device_ = device;

	itemCurrentIdx = 0;

	/// ルートシグネチャ初期化
	CreateGraphicRootSignatureForSkinnedObject3D(device_);
	CreateComputeRootSignatureForSkinnedObject3D(device_);
	/// インプットレイアウト初期化
	CreateInputLayoutForSkinningObject();
	/// ブレンドステート初期化
	CreateBlendStateForObject3d();
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);

	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/SkinningObject3D.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

	computeShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Skinning.CS.hlsl",
		L"cs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(computeShaderBlob_ != nullptr);

#pragma region SetDepthStencilDesc
	//Depthの機能を有効化
	depthStencilDesc_.DepthEnable = true;
	//書き込み
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc();
	//ComputePipelineStateDescの設定
	SetComputePipelineStateDesc();

	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));

	computePipelineState_ = nullptr;
	result = device_->CreateComputePipelineState(&computePipelineStateDesc_,
		IID_PPV_ARGS(&computePipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::CreatePSOForParticle(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {

	HRESULT result = S_FALSE;

	device_ = device;

	itemCurrentIdx = 0;

	/// ルートシグネチャ初期化
	CreateRootSignatureForParticle(device_);
	/// インプットレイアウト初期化
	CreateInputLayout();
	/// ブレンドステート初期化
	CreateBlendStateForParticle();
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);

	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Particle.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/Particle.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

#pragma region SetDepthStencilDesc

	depthStencilDesc_.DepthEnable = true;                           //Depthの機能を有効化
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //書き込みをしない
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //比較関数はLessEqual。近ければ描画される

#pragma endregion

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc();
	
	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::CreatePSOForSkyBox(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {

	HRESULT result = S_FALSE;

	device_ = device;

	itemCurrentIdx = 0;

	/// ルートシグネチャ初期化
	CreateRootSignatureForSprite(device_);
	/// インプットレイアウト初期化
	CreateInputLayoutForSkyBox();
	/// ブレンドステート初期化
	CreateBlendStateForSprite();
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);

	//Shaderをコンパイル
	//VS
	vertexShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/SkyBox.VS.hlsl",
		L"vs_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(vertexShaderBlob_ != nullptr);

	//PS
	pixelShaderBlob_ = dxc_->CompileShader(
		L"Resources/shaders/SkyBox.PS.hlsl",
		L"ps_6_0",
		dxc_->GetDxcUtils().Get(),
		dxc_->GetDxcCompiler().Get(),
		dxc_->GetIncludeHandler().Get()
	);
	assert(pixelShaderBlob_ != nullptr);

#pragma region SetDepthStencilDesc

	depthStencilDesc_.DepthEnable = true;                           //Depthの機能を有効化
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; //書き込みをしない
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //比較関数はLessEqual。近ければ描画される

#pragma endregion

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc();


	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::CreatePSO(PSOType psoType, ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode) {
	switch (psoType) {
	case PSOType::kObject3D:
		CreatePSOForObject3D(device, dxc_, fillMode);
		break;
	case PSOType::kParticle:
		CreatePSOForParticle(device, dxc_, fillMode);
		break;
	case PSOType::kSprite:
		CreatePSOForSprite(device, dxc_, fillMode);
		break;
	case PSOType::kSkinningObject3D:
		CreatePSOForSkinningObject3D(device, dxc_, fillMode);
		
		break;
	case PSOType::kSkyBox:
		CreatePSOForSkyBox(device, dxc_, fillMode);
		break;
	}
}

void PSO::UpdateImGui() {
	ImGui::Text("BlendState");
	UpdateImGuiCombo();
}

bool PSO::UpdateImGuiCombo() {

	//コンボボックスの項目
	std::vector<std::string> items = { "Normal", "Add", "Subtract","Multiply","Screen" };
	//現在の項目
	std::string& currentItem = items[itemCurrentIdx];
	//変更があったかどうか
	bool changed = false;

	//コンボボックスの表示
	if (ImGui::BeginCombo("Lighting", currentItem.c_str())) {
		for (int n = 0; n < items.size(); n++) {
			const bool is_selected = (currentItem == items[n]);
			if (ImGui::Selectable(items[n].c_str(), is_selected)) {
				currentItem = items[itemCurrentIdx];
				itemCurrentIdx = n;
				changed = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		currentItem = items[itemCurrentIdx];
		ImGui::EndCombo();
	}

	//変更があった場合
	if (changed) {

		HRESULT result = S_FALSE;

		switch (itemCurrentIdx) {
		case 0: // Normal
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
			break;
		case 1: // Add
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		case 2: // Subtract
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		case 3: // Multiply
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
			break;

		case 4: // Screen
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		}

		//PSOの再生成をする
		graphicsPipelineStateDesc_.BlendState = blendDesc_; // blendState
		result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
			IID_PPV_ARGS(&graphicPipelineState_));
		assert(SUCCEEDED(result));
	}

	return changed;
}

void PSO::SetGraphicPipelineStateDesc() {

	//DepthStencilの設定
	graphicsPipelineStateDesc_.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// RootSignature
	graphicsPipelineStateDesc_.pRootSignature = graphicRootSignature_.Get(); 

	// InputLayout
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDesc_;

	// VertexShader
	graphicsPipelineStateDesc_.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() }; 

	// PixelShader
	graphicsPipelineStateDesc_.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() }; 

	// blendState
	graphicsPipelineStateDesc_.BlendState = blendDesc_; 

	// rasterizerState
	graphicsPipelineStateDesc_.RasterizerState = rasterizerDesc_; 

	//書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトロポジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//どのように画面に色を打ち込むのかの設定
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
}

void PSO::SetComputePipelineStateDesc() {
	computePipelineStateDesc_.CS = { 
		computeShaderBlob_->GetBufferPointer(),
		computeShaderBlob_->GetBufferSize()
	};

	computePipelineStateDesc_.pRootSignature = computeRootSignature_.Get();

}
