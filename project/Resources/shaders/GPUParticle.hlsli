struct VertexShaderOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 color : COLOR0;
};

struct ParticleForCS {
	float3 translate;
	float3 scale;
	float3 velocity;
	float4 color;
	float lifetime;
	float currentTime;
};

struct ParticleAttributes {
	float3 scale;
	float3 color;
	float2 scaleRange;
	float2 rotateRange;
	float2 positionRange;
	float2 velocityRange;
	float2 colorRange;
	float2 lifetimeRange;
	float frequency;
	uint emitCount;
	uint scaleSetting;
	bool isBillboad;
	bool editColor;
	bool isTraslate;
	bool enableFollowEmitter;
};

struct PerView {
	float4x4 viewProjection;
	float4x4 billboardMatrix;
};

static const uint kMaxParticles = 1024000;