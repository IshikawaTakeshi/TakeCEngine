#include "SkinCluster.h"
#include "DirectXCommon.h"

void SkinCluster::Create(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Skeleton& skeleton, const ModelData& modelData,
	const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize) {

	//palette用のReosurce確保
	paletteResource = DirectXCommon::CreateBufferResource(device.Get(), sizeof(WellForGPU) * modelData.vertices.size());


	//paletteのsrv作成

	//influence用のResource確保

	//influenceのVBV作成

	//InverseBindPoseMatricesの保存領域の作成

	//ModelDataのSkinClusterの情報を元に、Influenceの中身を書き込む

}
