
struct Well {
	float4x4 skeletonSpaceMatrix;
	float4x4 skeletonSpaceInvTransposeMatrix;
};

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

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);

StructuredBuffer<VertexInfluence> gVertexInfluences : register(t2);

RWStructuredBuffer<Vertex> gSkinnedVertices : register(u0);

ConstantBuffer<SkinningInfomation> gVertexInformation : register(b0);




[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint vertexIndex = DTid.x;
	if (vertexIndex < gVertexInformation.numVertices) {
	
		Vertex input = gInputVertices[vertexIndex];
	VertexInfluence influence = gVertexInfluences[vertexIndex];
		
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
		skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInvTransposeMatrix) * influence.weight.x;
		skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInvTransposeMatrix) * influence.weight.y;
		skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInvTransposeMatrix) * influence.weight.z;
		skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInvTransposeMatrix) * influence.weight.w;
	//正規化しておく
		skinned.normal = normalize(skinned.normal);
		
		gSkinnedVertices[vertexIndex] = skinned;
	}
}