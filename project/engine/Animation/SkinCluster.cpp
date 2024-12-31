#include "SkinCluster.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>

void SkinCluster::Create(
	const ComPtr<ID3D12Device>& device,SrvManager* srvManager,
	Skeleton* skeleton, const ModelData& modelData) {

	//palette用のReosurce確保
	//MEMO:sizeInBytesはWellForGPUのサイズ×ジョイント数
	paletteResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(WellForGPU) * skeleton->GetJoints().size());
	WellForGPU* mappedPaletteData = nullptr;
	paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPaletteData));
	mappedPalette = { mappedPaletteData, skeleton->GetJoints().size() };
	//paletteのSRVのIndexを取得
	paletteResourceIndex = srvManager->Allocate();
	paletteSrvHandle.first = srvManager->GetSrvDescriptorHandleCPU(paletteResourceIndex);
	paletteSrvHandle.second = srvManager->GetSrvDescriptorHandleGPU(paletteResourceIndex);

	//paletteのsrv作成
	srvManager->CreateSRVforStructuredBuffer(
		UINT(skeleton->GetJoints().size()),sizeof(WellForGPU),paletteResource.Get(), paletteResourceIndex);

	//influence用のResource確保
	//VertexInfluence * std::vector<VertexData>
	influenceResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluenceData = nullptr;
	influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluenceData));
	std::memset(mappedInfluenceData, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	mappedInfluences = { mappedInfluenceData, modelData.vertices.size() };

	//influenceのSRV作成
	//influenceBufferView.BufferLocation = influenceResource->GetGPUVirtualAddress();
	//influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	//influenceBufferView.StrideInBytes = sizeof(VertexInfluence);
	influenceResourceIndex = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(
		UINT(modelData.vertices.size()),sizeof(VertexInfluence),influenceResource.Get(),influenceResourceIndex);

	

	//InverseBindPoseMatricesの保存領域の作成
	inverseBindPoseMatrices.resize(skeleton->GetJoints().size());
	std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), []() {
		return MatrixMath::MakeIdentity4x4(); });

	//ModelDataのSkinClusterの情報を元に、Influenceの中身を書き込む
	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton->GetJointMap().find(jointWeight.first);

		//Jointが見つからなかったらスキップ
		if (it == skeleton->GetJointMap().end()) {
			continue;
		}

		inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = mappedInfluences[vertexWeight.vertexIndex];
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

void SkinCluster::Update(Skeleton* skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton->GetJoints().size(); ++jointIndex) {
		assert(jointIndex < inverseBindPoseMatrices.size());

		mappedPalette[jointIndex].skeletonSpaceMatrix =
			inverseBindPoseMatrices[jointIndex] * skeleton->GetJoints()[jointIndex].skeletonSpaceMatrix;
		mappedPalette[jointIndex].skeletonSpaceInvTransposeMatrix =
			MatrixMath::Transpose(MatrixMath::Inverse(mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}
