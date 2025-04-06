#include "GPUParticle.hlsli"

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
}