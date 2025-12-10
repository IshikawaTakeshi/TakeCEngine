#pragma once
#include "base/DirectXCommon.h"
#include <unordered_map>

//============================================================
// ResourceBarrier class
//============================================================
class ResourceBarrier {
private:

	/// コンストラクタ・デストラクタ・コピー禁止
	ResourceBarrier() = default;
	~ResourceBarrier() = default;
	ResourceBarrier(const ResourceBarrier&) = delete;
	ResourceBarrier& operator=(const ResourceBarrier&) = delete;

public:

	//=========================================================
	/// functions
	//=========================================================
	
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static ResourceBarrier& GetInstance();

	/// <summary>
	/// リソースバリアの初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// リソースの状態遷移
	/// </summary>
	/// <param name="beforeState"></param>
	/// <param name="afterState"></param>
	/// <param name="resource"></param>
	void Transition(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState,ID3D12Resource* resource);


private:

	//dxCommon
	DirectXCommon* dxCommon_ = nullptr;
	
	//リソースごとの状態を管理するためのマップ
	std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> resourceStates_;
};