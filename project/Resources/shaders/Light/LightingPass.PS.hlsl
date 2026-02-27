#include "../Light/PointLight.hlsli"
#include "../Light/SpotLight.hlsli"
#include "../Light/DirectionalLight.hlsli"
#include "../Light/LightCountData.hlsli"
#include "../GBuffer.hlsli"
#include "../PostEffect/FullScreen.hlsli"
#include "../3d/Object3d.hlsli"
#include "../CameraData.hlsli"

Texture2D<float4> gGBufferAlbedo : register(t0);
Texture2D<float4> gGBufferNormal : register(t1);
Texture2D<float4> gGBufferMaterial : register(t2);
//環境マップ用テクスチャ
TextureCube<float4> gEnvMapTexture : register(t3);

// メインカメラの深度テクスチャ
Texture2D<float> gSceneDepth : register(t4); 
//ポイントライト
StructuredBuffer<PointLight> gPointLight : register(t5);
//スポットライト
StructuredBuffer<SpotLight> gSpotLight : register(t6);

//サンプラー
SamplerState gSampler : register(s0);

//メインカメラ情報
ConstantBuffer<CameraData> gMainCameraInfo : register(b0);
//平行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
//ライト数カウンター
ConstantBuffer<LightCountData> gLightCount : register(b2);


PixelShaderOutput main(VSOutput_FullScreen input)
{
    
    PixelShaderOutput output;
    
    //平行光源の情報
    DirectionalLight dirLightInfo = gDirectionalLight;

    // GBufferから情報を取得
    float3 albedo = gGBufferAlbedo.Sample(gSampler, input.texcoord).rgb;
    float3 normal = DecodeNormal(gGBufferNormal.Sample(gSampler, input.texcoord).rgb);
    float metallic = gGBufferMaterial.Sample(gSampler, input.texcoord).r;
    float roughness = gGBufferMaterial.Sample(gSampler, input.texcoord).g;
    float ao = gGBufferMaterial.Sample(gSampler, input.texcoord).b;
    float depth = gSceneDepth.Sample(gSampler, input.texcoord);
    
    // ワールド座標を復元
    float3 worldPos = ReconstructWorldPos(input.texcoord, depth,gMainCameraInfo.viewProjectionInverse);
    
    // 法線を[-1, 1]の範囲に変換
    normal = normalize(normal);
    
    //------------------------------------------------------------
    // ライティング計算
    //------------------------------------------------------------
    
    //dirctionalLightの計算
    float3 toEye = normalize(gMainCameraInfo.worldPosition - worldPos);
    float3 dirLighting = CalcDirectionalLighting(dirLightInfo, roughness, normal, toEye, albedo);
    
    //pointLightの計算(光の減衰込み)
    float3 pointLighting = CalcPointLighting(
        gPointLight,
        roughness,
        gLightCount.pointLightCount,
        normal, worldPos, toEye, albedo);
    
    //spotLightの計算
    float3 spotLighting = CalcSpotLighting(
        gSpotLight, 
        roughness,
        gLightCount.spotLightCount,
        normal, worldPos, toEye, albedo);
    
    //環境光
    float3 cameraToPosition = normalize(worldPos - gMainCameraInfo.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normal);
    float4 environmentColor = gEnvMapTexture.Sample(gSampler, reflectedVector);
    
    //拡散反射+鏡面反射
    output.color.rgb = dirLighting + pointLighting + spotLighting + environmentColor.rgb * ao;
    //環境マップ
    output.color.rgb += environmentColor.rgb * metallic;
    //アルファ値
    output.color.a = 1.0f;
    
    return output;
}