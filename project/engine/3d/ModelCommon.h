#pragma once

//前方宣言
namespace TakeC {
	class DirectXCommon;
	class SrvManager;
}

//============================================================================
// ModelCommon class
//============================================================================
class ModelCommon {
private:

	// コンストラクタ・デストラクタ・コピー禁止
	ModelCommon() = default;
	~ModelCommon() = default;
	ModelCommon(const ModelCommon&) = delete;
	ModelCommon& operator=(const ModelCommon&) = delete;

public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static ModelCommon& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// DirectXCommonの取得
	/// </summary>
	/// <returns></returns>
	TakeC::DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	/// <summary>
	/// SrvManagerの取得
	/// </summary>
	/// <returns></returns>
	TakeC::SrvManager* GetSrvManager() const { return srvManager_; }


private:

	//DirectXCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;
	//SrvManager
	TakeC::SrvManager* srvManager_ = nullptr;
};