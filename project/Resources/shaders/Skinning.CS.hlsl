

struct Vertex {
	float4 position;
	float2 texcoord;
	float3 normal;
};

struct VertexInfluence {
	float4 weight;
	int4 index;
};

struct SkinningInfomation {
	uint numVertices;
};

struct Well {
	float4x4 skeletonSpaceMatrix;
	float4x4 skeletonSpaceInvTransposeMatrix;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluences : register(t2);

RWStructuredBuffer<Vertex> gOutputVertices : register(u0);

ConstantBuffer<SkinningInfomation> gSkinningInfomation : register(b0);


[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint vertexIndex = DTid.x;
	if ( vertexIndex >= gSkinningInfomation.numVertices ) {
		return;
	}
		//Skinning計算
		//入力頂点
	Vertex input = gInputVertices[vertexIndex];
	VertexInfluence influence = gInfluences[vertexIndex];
		
		//Skinning後の頂点の計算
	Vertex skinned;
	skinned.texcoord = input.texcoord;
		
		//位置の変換
	skinned.position = mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
	skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
	skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
	skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
	skinned.position.w = 1.0f;
	
	//法線の変換
	skinned.normal = mul(input.normal, (float3x3)gMatrixPalette[influence.index.x].skeletonSpaceInvTransposeMatrix) * influence.weight.x;
	skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[influence.index.y].skeletonSpaceInvTransposeMatrix) * influence.weight.y;
	skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[influence.index.z].skeletonSpaceInvTransposeMatrix) * influence.weight.z;
	skinned.normal += mul(input.normal, (float3x3)gMatrixPalette[influence.index.w].skeletonSpaceInvTransposeMatrix) * influence.weight.w;
	//正規化しておく
	skinned.normal = normalize(skinned.normal);
		
	//頂点データの書き込み
	gOutputVertices[vertexIndex] = skinned;
	
}