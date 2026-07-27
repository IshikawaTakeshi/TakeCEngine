#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"
#include "Light/PointLight.hlsli"
#include "Light/SpotLight.hlsli"
#include "Light/LightCountData.hlsli"
struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
    bool isBillboard;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
StructuredBuffer<PointLight> gPointLight : register(t1);
StructuredBuffer<SpotLight> gSpotLight : register(t2);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b0);
ConstantBuffer<PerView> gPerView : register(b1);
ConstantBuffer<LightCountData> gLightCount : register(b2);

struct VertexShaderInput
{
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

//----------------------------------------------------
// 回転行列生成関数
//----------------------------------------------------
float4x4 MakeRotationMatrix(float4 q)
{
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;

    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    // 3x3 回転成分を組み立て
    // （右手/左手・row/column の違いで転置が必要な場合があります）
    float4x4 m =
    {
        float4(1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f),
        float4(2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f),
        float4(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f),
        float4(0.0f, 0.0f, 0.0f, 1.0f)
    };

    return m;
}

//====================================================
// 頂点シェーダー
//====================================================

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    ParticleForGPU particle = gParticle[instanceId];
    DirectionalLight dirLightInfo = gDirectionalLight;
	
	//---------------------------------------------------
	// ワールド行列計算
	//----------------------------------------------------

    float4x4 worldMatrix;

    if (gPerView.isBillboard)
    {
		// スケーリング行列
        float4x4 scaleMatrix =
        {
            float4(particle.scale.x, 0.0f, 0.0f, 0.0f),
			float4(0.0f, particle.scale.y, 0.0f, 0.0f),
			float4(0.0f, 0.0f, particle.scale.z, 0.0f),
			float4(0.0f, 0.0f, 0.0f, 1.0f)
        };

		// Z軸回転行列（パーティクル独自の回転）
        float angle = particle.rotate.z;
        float cosR = cos(angle);
        float sinR = sin(angle);

        float4x4 rotZ =
        {
            float4(cosR, sinR, 0.0f, 0.0f),
			float4(-sinR, cosR, 0.0f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.0f),
			float4(0.0f, 0.0f, 0.0f, 1.0f)
        };

		// 平行移動行列
        float4x4 translateMatrix =
        {
            float4(1.0f, 0.0f, 0.0f, 0.0f),
			float4(0.0f, 1.0f, 0.0f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.0f),
			float4(particle.translate, 1.0f)
        };

		// ワールド行列 = スケール * rotZ * ビルボード * 平行移動
        worldMatrix = mul(mul(mul(scaleMatrix, rotZ), gPerView.billboardMatrix), translateMatrix);
		
    }
    else
    {
		// 通常のワールド行列（3軸回転を適用）

		// スケール
        float4x4 scaleMatrix =
        {
            float4(particle.scale.x, 0.0f, 0.0f, 0.0f),
			float4(0.0f, particle.scale.y, 0.0f, 0.0f),
			float4(0.0f, 0.0f, particle.scale.z, 0.0f),
			float4(0.0f, 0.0f, 0.0f, 1.0f)
        };

		// 3軸回転行列を生成
        float4x4 rotationMatrix = MakeRotationMatrix(particle.rotate);

		// 平行移動
        float4x4 translateMatrix =
        {
            float4(1.0f, 0.0f, 0.0f, 0.0f),
			float4(0.0f, 1.0f, 0.0f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.0f),
			float4(particle.translate, 1.0f)
        };

		// 最終ワールド行列 = スケーリング * 回転 * 平行移動
        worldMatrix = mul(mul(scaleMatrix, rotationMatrix), translateMatrix);
    }
	
	//----------------------------------------------------
	// 最終姿勢の頂点位置計算
	//----------------------------------------------------
	
    float4 worldPos4 = mul(input.position, worldMatrix);
    float3 worldPos = worldPos4.xyz;
	

    output.position = mul(worldPos4, gPerView.viewProjection);
    output.texcoord = input.texcoord;
    output.color = particle.color;
	
	//----------------------------------------------------
	// 法線処理（ビルボード時はカメラ正面固定）
	//-----------------------------------------------------
    float3 normalWS = (gPerView.isBillboard)
		? float3(0, 0, 1)
		: normalize(mul(input.normal, (float3x3) worldMatrix));

    output.normal = normalWS;
	
	//----------------------------------------------------
	// HL2-basisライティング計算
	//-----------------------------------------------------
    output.basisColor1 = float3(0, 0, 0);
    output.basisColor2 = float3(0, 0, 0);
    output.basisColor3 = float3(0, 0, 0);
	
	//-----------------------------------------------------
	// directional light計算
	//-----------------------------------------------------

	//光情報を計算
    float3 lightCol = CalcDirectionalLightingSimple(dirLightInfo, normalWS);

	// basis方向ごとの寄与
    float3 weights = saturate(float3(
		dot(lightCol, HL2Basis0),
		dot(lightCol, HL2Basis1),
		dot(lightCol, HL2Basis2)
	));

    output.basisColor1 += lightCol * weights.x;
    output.basisColor2 += lightCol * weights.y;
    output.basisColor3 += lightCol * weights.z;
	
	//-----------------------------------------------------
	// point light計算
	//-----------------------------------------------------
	
    float3 lightColPoint = CalcPointLightingSimple(
		gPointLight,
		gLightCount.pointLightCount,
		normalWS,
		worldPos);
	
	// basis方向ごとの寄与
    float3 weightsPoint = saturate(float3(
		dot(lightColPoint, HL2Basis0),
		dot(lightColPoint, HL2Basis1),
		dot(lightColPoint, HL2Basis2)
	));
	
    output.basisColor1 += lightColPoint * weightsPoint.x;
    output.basisColor2 += lightColPoint * weightsPoint.y;
    output.basisColor3 += lightColPoint * weightsPoint.z;
	
	//-----------------------------------------------------
	// spot light計算
	//-----------------------------------------------------
	
    float3 lightColSpot = CalcSpotLightingSimple(
		gSpotLight,
		gLightCount.spotLightCount,
		normalWS,
		worldPos);
	
	// basis方向ごとの寄与
    float3 weightsSpot = saturate(float3(
		dot(lightColSpot, HL2Basis0),
		dot(lightColSpot, HL2Basis1),
		dot(lightColSpot, HL2Basis2)
	));
	
    output.basisColor1 += lightColSpot * weightsSpot.x;
    output.basisColor2 += lightColSpot * weightsSpot.y;
    output.basisColor3 += lightColSpot * weightsSpot.z;
	
    return output;
}