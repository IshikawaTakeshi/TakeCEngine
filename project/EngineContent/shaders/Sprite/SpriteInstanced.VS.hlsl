#include "SpriteInstanced.hlsli"

struct SpriteInstanceData {
	float4x4 WVP;
	float4x4 World;
	float4x4 WorldInverseTranspose;
	float4 color;
	float2 uvLeftTop;
	float2 uvSize;
	float2 anchorPoint;
	float2 size;
	int isFlipX;
	int isFlipY;
};

struct SpriteBatchConfig {
	uint startIndex;
};

ConstantBuffer<SpriteBatchConfig> gBatchConfig : register(b0);
StructuredBuffer<SpriteInstanceData> gInstances : register(t1);

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID) {
	VertexShaderOutput output;
	
	// StartInstanceLocation の代わりに gBatchConfig.startIndex を使用して正しいインデックスを取得
	uint actualInstanceId = instanceId + gBatchConfig.startIndex;
	SpriteInstanceData instance = gInstances[actualInstanceId];

	// 1. テクスチャ反転の処理
	float2 localUV = input.texcoord;
	if (instance.isFlipX != 0) {
		localUV.x = 1.0f - localUV.x;
	}
	if (instance.isFlipY != 0) {
		localUV.y = 1.0f - localUV.y;
	}

	// 2. ユニットクアッド頂点(0.0f〜1.0f)にアンカーポイントを適用してローカル座標を算出
	// WVP行列にサイズ(スケール)がすでに適用されているため、ここではサイズ倍しません。
	float2 localPos = input.position.xy - instance.anchorPoint;

	// 3. 座標および法線変換
	output.position = mul(float4(localPos, 0.0f, 1.0f), instance.WVP);
	output.texcoord = instance.uvLeftTop + localUV * instance.uvSize;
	output.normal = normalize(mul(input.normal, (float3x3) instance.WorldInverseTranspose));
	output.worldPosition = mul(float4(localPos, 0.0f, 1.0f), instance.World).xyz;
	output.color = instance.color;

	return output;
}
