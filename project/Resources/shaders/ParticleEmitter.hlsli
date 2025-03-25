struct EmitterSphere {
	float3 translate; //エミッターの位置
	float radius; //射出半径
	float frequency; //発生頻度
	float frequencyTime; //経過時間
	uint particleCount; //発生するParticleの数
	uint isEmit; //発生許可
};