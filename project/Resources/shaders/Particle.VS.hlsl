#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"

struct PerView {
	float4x4 viewProjection;
	float4x4 billboardMatrix;
	bool isBillboard;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<DirectionalLight> gDirLight : register(b0);
ConstantBuffer<PerView> gPerView : register(b1);

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

//----------------------------------------------------
// HL2 basis ベクトル（ビュー空間基準）
//----------------------------------------------------
static const float3 HL2Basis0 = float3(1, 0, 0);
static const float3 HL2Basis1 = float3(0, 1, 0);
static const float3 HL2Basis2 = float3(0, 0, 1);

//====================================================
// 頂点シェーダー
//====================================================

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID) {
	VertexShaderOutput output;
	ParticleForGPU particle = gParticle[instanceId];
	DirectionalLight dirLightInfo = gDirLight;
	
	 //---------------------------------------------------
	// ワールド行列計算
	//----------------------------------------------------

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
	
	 //----------------------------------------------------
    // 法線処理（ビルボード時はカメラ正面固定）
    //----------------------------------------------------
	float3 normalWS = (gPerView.isBillboard) ? float3(0, 0, 1)
        : normalize(mul(input.normal, (float3x3)worldMatrix));

	output.normal = normalWS;
	
	 //----------------------------------------------------
    // HL2-basisライティング計算
    //----------------------------------------------------
	output.basisColor1 = float3(0, 0, 0);
	output.basisColor2 = float3(0, 0, 0);
	output.basisColor3 = float3(0, 0, 0);

    // 方向光情報を取得
	float3 lightDir = normalize(-dirLightInfo.direction.xyz);
	float3 lightCol = dirLightInfo.color.rgb * dirLightInfo.intensity;

    // 入射角に基づく減衰（Lambert的）
	float NdotL = saturate(dot(normalWS, lightDir));
	lightCol *= NdotL;

    // basis方向ごとの寄与
	float3 weights = saturate(float3(
        dot(lightDir, HL2Basis0),
        dot(lightDir, HL2Basis1),
        dot(lightDir, HL2Basis2)
    ));

	output.basisColor1 += lightCol * weights.x;
	output.basisColor2 += lightCol * weights.y;
	output.basisColor3 += lightCol * weights.z;
	
	return output;
}