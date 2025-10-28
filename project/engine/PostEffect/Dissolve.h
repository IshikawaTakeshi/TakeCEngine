#pragma once
#include "PostEffect/PostEffect.h"

// Dissolve情報構造体
struct DissolveInfo {
	float threshold;
	bool isDissolve;
	float padding[2];
};

//============================================================================
//	Dissolve class
//============================================================================
class Dissolve : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Dissolve() = default;
	~Dissolve() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	/// <param name="CSFilePath"></param>
	/// <param name="inputResource"></param>
	/// <param name="inputSrvIdx"></param>
	/// <param name="outputResource"></param>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;

	/// <summary>
	/// ディスパッチ
	/// </summary>
	void Dispatch() override;

private:

	// Dissolve情報リソース
	ComPtr<ID3D12Resource> dissolveInfoResource_ = nullptr;
	// Dissolve情報データ
	DissolveInfo* dissolveInfoData_ = nullptr;

	// マスクテクスチャのファイルパス
	std::string maskTextureFilePath_ = "";
};