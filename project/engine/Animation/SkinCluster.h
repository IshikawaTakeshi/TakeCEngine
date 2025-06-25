#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <span>
#include <array>

#include "ResourceDataStructure.h"
#include "Skeleton.h"
#include "SrvManager.h"
#include "Animation/VertexInfluence.h"

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix; //位置用
	Matrix4x4 skeletonSpaceInvTransposeMatrix; //法線用
};

struct ModelData;
struct SkinCluster {

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

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
	uint32_t paletteIndex;
	uint32_t influenceIndex;

	void Create(const ComPtr<ID3D12Device>& device,SrvManager* srvManager,
				Skeleton* skeleton,const ModelData* modelData);

	void Update(Skeleton* skeleton);
};

