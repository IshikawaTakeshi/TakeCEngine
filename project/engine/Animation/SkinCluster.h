#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <span>
#include <array>

#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/base/SrvManager.h"
#include "engine/math/Matrix4x4.h"
#include "engine/Animation/Skeleton.h"

//最大の影響ボーン数
const uint32_t kNumMaxInfluence = 4;

//頂点の影響情報構造体
struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

//GPU用のウェル構造体
struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix; //位置用
	Matrix4x4 skeletonSpaceInvTransposeMatrix; //法線用
};

//==============================================================
//スキンクラスター構造体
//==============================================================
struct SkinCluster {

	//inverseBindPoseMatrices
	std::vector<Matrix4x4> inverseBindPoseMatrices;
	//influenceResource
	ComPtr<ID3D12Resource> influenceResource;
	std::span<VertexInfluence> mappedInfluences;
	//paletteResource
	ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	//skinningInfoResource
	ComPtr<ID3D12Resource> skinningInfoResource;
	SkinningInfo* skinningInfoData;

	//SRVハンドル
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
	uint32_t paletteIndex;
	uint32_t influenceIndex;

	/// <summary>
	/// SkinCluster生成
	/// </summary>
	void Create(const ComPtr<ID3D12Device>& device,TakeC::SrvManager* srvManager,
				Skeleton* skeleton,const ModelData* modelData);

	/// <summary>
	/// SkinCluster更新
	/// </summary>
	void Update(Skeleton* skeleton);
};

