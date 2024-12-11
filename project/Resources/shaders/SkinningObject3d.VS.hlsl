#include "Object3d.hlsli"

struct Well {
	float4x4 skeletonSpaceMatrix;
	float4x4 skeletonSpaceInvTransposeMatrix;
};

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 weight : WEIGHT0;
	int4 index : INDEX0;
};

struct Skinned {
	float4 position;
	float3 normal;
};

struct TransformationMatrix {
	float4x4 WVP;
	float4x4 World;
	float4x4 WorldInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

StructuredBuffer<Well> gMatrixPalette : register(t0);

Skinned Skinning(VertexShaderInput input) {
	Skinned skinned;
	//位置の変換
	skinned.position = mul(input.position, gMatrixPalette[input.index.x].skeletonSpaceMatrix) * input.weight.x;
	skinned.position += mul(input.position, gMatrixPalette[input.index.y].skeletonSpaceMatrix) * input.weight.y;
	skinned.position += mul(input.position, gMatrixPalette[input.index.z].skeletonSpaceMatrix) * input.weight.z;
	skinned.position += mul(input.position, gMatrixPalette[input.index.w].skeletonSpaceMatrix) * input.weight.w;
	skinned.position.w = 1.0f;
	
	//法線の変換
	skinned.normal = mul(input.normal, (float3x3)gMatrixPalette[input.index.x].skeletonSpaceInvTransposeMatrix) * input.weight.x;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.y].skeletonSpaceInvTransposeMatrix) * input.weight.y;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.z].skeletonSpaceInvTransposeMatrix) * input.weight.z;
	skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.w].skeletonSpaceInvTransposeMatrix) * input.weight.w;
	//正規化しておく
	skinned.normal = normalize(skinned.normal);
	
	return skinned;
}


VertexShaderOutput main( VertexShaderInput input ) {
	VertexShaderOutput output;
	Skinned skinned = Skinning(input);
	
	//Skinning結果を使って位置を変換
	output.position = mul(skinned.position, gTransformationMatrix.WVP);
	output.texcoord = input.texcoord;
	output.normal = normalize(mul(skinned.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
	output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
	return output;
}