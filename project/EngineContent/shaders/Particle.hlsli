struct VertexShaderOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 color : COLOR0;
	
	float3 basisColor1 : COLOR1;
	float3 basisColor2 : COLOR2;
	float3 basisColor3 : COLOR3;
};

struct ParticleForGPU {
	float3 translate;
	float4 rotate;
	float3 scale;
	float3 velocity;
	float4 color;
	float lifetime;
	float currentTime;
};