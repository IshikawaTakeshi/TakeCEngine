#include "SkinCluster.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "3d/Model.h"
#include <algorithm>
#include <cassert>

void SkinCluster::Create(
	const ComPtr<ID3D12Device>& device,SrvManager* srvManager,
	Skeleton* skeleton, const ModelData* modelData) {

	//palette用のReosurce確保
	//MEMO:sizeInBytesはWellForGPUのサイズ×ジョイント数
	paletteResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(WellForGPU) * skeleton->GetJoints().size());
	paletteResource->SetName(L"SkinCluster::paletteResource");
	
	//paletteのSRVのIndexを取得
	paletteIndex = srvManager->Allocate();
	paletteSrvHandle.first = srvManager->GetSrvDescriptorHandleCPU(paletteIndex);
	paletteSrvHandle.second = srvManager->GetSrvDescriptorHandleGPU(paletteIndex);
	//paletteのsrv作成
	srvManager->CreateSRVforStructuredBuffer(
		UINT(skeleton->GetJoints().size()),sizeof(WellForGPU),paletteResource.Get(), paletteIndex);

	WellForGPU* mappedPaletteData = nullptr;
	paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPaletteData));
	mappedPalette = { mappedPaletteData, skeleton->GetJoints().size() };

	//influence用のResource確保
	//VertexInfluence * std::vector<VertexData>
	influenceResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(VertexInfluence) * modelData->influences.size());
	influenceResource->SetName(L"SkinCluster::influenceResource");
	
	//influenceのSRV作成
	influenceIndex = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(
		UINT(modelData->mesh.GetAllVertices().size()), sizeof(VertexInfluence), influenceResource.Get(), influenceIndex);

	VertexInfluence* mappedInfluenceData = nullptr;
	influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluenceData));
	std::memset(mappedInfluenceData, 0, sizeof(VertexInfluence) * modelData->influences.size());
	mappedInfluences = { mappedInfluenceData, modelData->influences.size() };
	
	//mappedInfluencesにmodeldataのinfluencesをコピー
	for (size_t i = 0; i < modelData->influences.size(); ++i) {
		mappedInfluences[i] = modelData->influences[i];
	}

	//skinningInfoResourceの作成
	skinningInfoResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(SkinningInfo));
	skinningInfoResource->Map(0, nullptr, reinterpret_cast<void**>(&skinningInfoData));
	*skinningInfoData = modelData->skinningInfoData;

	//InverseBindPoseMatricesの保存領域の作成
	inverseBindPoseMatrices.resize(skeleton->GetJoints().size());
	std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), []() {
		return MatrixMath::MakeIdentity4x4(); });

	//ModelDataのSkinClusterの情報を元に、Influenceの中身を書き込む
	for (const auto& jointWeight : modelData->skinClusterData) {
		auto it = skeleton->GetJointMap().find(jointWeight.first);
		if (it == skeleton->GetJointMap().end()) continue;
		inverseBindPoseMatrices[it->second] = jointWeight.second.inverseBindPoseMatrix;
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
