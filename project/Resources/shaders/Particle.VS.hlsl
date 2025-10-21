#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"

struct PerView {
	float4x4 viewProjection;
	float4x4 billboardMatrix;
	bool isBillboard;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);
ConstantBuffer<DirectionalLight> gDirLight : register(b1);

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID) {
	VertexShaderOutput output;
	ParticleForGPU particle = gParticle[instanceId];

	float4x4 worldMatrix;

	if (gPerView.isBillboard) {
    // スケーリング行列
		float4x4 scaleMatrix = {
			float4(particle.scale.x, 0.0f, 0.0f, 0.0f),
        float4(0.0f, particle.scale.y, 0.0f, 0.0f),
        float4(0.0f, 0.0f, particle.scale.z, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f)
		};

    // Z軸回転行列（パーティクル独自の回転）
		float angle = particle.rotate.z;
		float cosR = cos(angle);
		float sinR = sin(angle);

		float4x4 rotZ = {
			float4(cosR, sinR, 0.0f, 0.0f),
        float4(-sinR, cosR, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f)
		};

    // 平行移動行列
		float4x4 translateMatrix = {
			float4(1.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f),
        float4(particle.translate, 1.0f)
		};

    // ワールド行列 = スケール * rotZ * ビルボード * 平行移動
		worldMatrix = mul(scaleMatrix, mul(rotZ, gPerView.billboardMatrix));
		worldMatrix = mul(worldMatrix, translateMatrix);

		
	} else {
		// 通常のワールド行列（スケーリング・Z軸回転・平行移動）

		// スケール
		float4x4 scaleMatrix = {
			float4(particle.scale.x, 0.0f, 0.0f, 0.0f),
			float4(0.0f, particle.scale.y, 0.0f, 0.0f),
			float4(0.0f, 0.0f, particle.scale.z, 0.0f),
			float4(0.0f, 0.0f, 0.0f, 1.0f)
		};

		// Z軸回転
		float angle = particle.rotate.z;
		float cosR = cos(angle);
		float sinR = sin(angle);

		float4x4 rotZ = {
			float4(cosR, sinR, 0.0f, 0.0f),
			float4(-sinR, cosR, 0.0f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.0f),
			float4(0.0f, 0.0f, 0.0f, 1.0f)
		};

		// 平行移動
		float4x4 translateMatrix = {
			float4(1.0f, 0.0f, 0.0f, 0.0f),
			float4(0.0f, 1.0f, 0.0f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.0f),
			float4(particle.translate, 1.0f)
		};

		// 最終ワールド行列 = 平行移動 * 回転 * スケーリング
		worldMatrix = mul(mul(scaleMatrix, rotZ), translateMatrix);
	}

	output.position = mul(input.position, mul(worldMatrix, gPerView.viewProjection));
	output.texcoord = input.texcoord;
	output.color = particle.color;
	
	//===============光蓄積計算===============//
	// View空間での基底ベクトル
	float3x3 hl2Basis = float3x3(
		float3(1, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1)
	);

	// ライト方向（View空間想定）
	float3 lightDir = normalize(-gDirLight.direction); 
	// ライト色×強度
	float3 lightCol = gDirLight.color.rgb * gDirLight.intensity;
	
	// 各基底ベクトルとの内積を重みとする
	float3 weights = saturate(float3(
		dot(lightDir, hl2Basis[0]),
		dot(lightDir, hl2Basis[1]),
		dot(lightDir, hl2Basis[2])
	));

	output.basisColor0 = lightCol * weights.x;
	output.basisColor1 = lightCol * weights.y;
	output.basisColor2 = lightCol * weights.z;

	output.normal = float3(0, 0, -1); // ビルボードの正面方向
	return output;
}