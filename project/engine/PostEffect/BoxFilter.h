#pragma once
#include "PostEffect/PostEffect.h"

class BoxFilter : public PostEffect {
public:
	BoxFilter() = default;
	~BoxFilter() = default;
	void Initialize(
		DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	void UpdateImGui() override;
	
	void Dispatch() override;

private:

	struct BoxFilterInfo {
		bool isActive = true; // フィルターの有効無効
	};
	BoxFilterInfo* filterInfoData_ = nullptr; // フィルター情報データ

	ComPtr<ID3D12Resource> filterInfoResource_; // フィルター情報リソース

};

