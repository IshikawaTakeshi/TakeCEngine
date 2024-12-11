#include "SkinCluster.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>

void SkinCluster::Create(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device,SrvManager* srvManager,
	const Skeleton& skeleton, const ModelData& modelData) {

	//palette用のReosurce確保
	paletteResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(WellForGPU) * modelData.vertices.size());
	WellForGPU* mappedPaletteData = nullptr;
	paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPaletteData));
	mappedPalette = { mappedPaletteData, modelData.vertices.size() };
	//paletteのSRVのIndexを取得
	uint32_t srvIndex = srvManager->Allocate();
	paletteSrvHandle.first = srvManager->GetSrvDescriptorHandleCPU(srvIndex);
	paletteSrvHandle.second = srvManager->GetSrvDescriptorHandleGPU(srvIndex);

	//paletteのsrv作成
	srvManager->CreateSRVforStructuredBuffer(
		UINT(skeleton.joints.size()),sizeof(WellForGPU),paletteResource.Get(),srvIndex);

	//influence用のResource確保
	influenceResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluenceData = nullptr;
	influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluenceData));
	std::memset(mappedInfluenceData, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	mappedInfluences = { mappedInfluenceData, modelData.vertices.size() };

	//influenceのVBV作成
	influenceBufferView.BufferLocation = influenceResource->GetGPUVirtualAddress();
	influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	//InverseBindPoseMatricesの保存領域の作成
	inverseBindPoseMatrices.resize(skeleton.joints.size());
	std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), []() {
		return MatrixMath::MakeIdentity4x4(); });

	//ModelDataのSkinClusterの情報を元に、Influenceの中身を書き込む
	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton.jointMap.find(jointWeight.first);

		//Jointが見つからなかったらスキップ
		if (it == skeleton.jointMap.end()) {
			continue;
		}

		inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			VertexInfluence& currentInfluence = mappedInfluences[vertexWeight.vertexIndex];
			for (uint32_t i = 0; i < kNumMaxInfluence; i++) {

				//Weightが0.0fの場合、WeightとJointのIndexを書き込む
				if (currentInfluence.weights[i] == 0.0f) {
					currentInfluence.weights[i] = vertexWeight.weight;
					currentInfluence.jointIndices[i] = (*it).second;
					break;
				}
			}
		}
	}
}

void SkinCluster::Update(const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); jointIndex++) {
		assert(jointIndex < inverseBindPoseMatrices.size());
		mappedPalette[jointIndex].skeletonSpaceMatrix =
			inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		mappedPalette[jointIndex].skeletonSpaceInvTransposeMatrix =
			MatrixMath::Transpose(MatrixMath::Inverse(mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}
