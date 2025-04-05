#include "GPUParticle.hlsli"

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeCounter : register(u1);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	uint particleIndex = DTid.x;
	
	//particleIndexが最大数を超えた場合は何もしない
	if (particleIndex > kMaxParticles) {
		return;
	}
	
	//カウンター初期化
	if(particleIndex == 0) {
		gFreeCounter[0] = 0;
	}
	//カウント分Particleを射出
	gParticles[particleIndex].translate = float3(-3.0f, 0.0f, 0.0f);
	gParticles[particleIndex].scale = float3(1.0f, 1.0f, 1.0f);
	gParticles[particleIndex].color = float4(1.0f, 1.0f, 1.0f, 1.0f);
}