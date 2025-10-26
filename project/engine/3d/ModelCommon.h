#pragma once

//前方宣言
class DirectXCommon;
class SrvManager;

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
	static ModelCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// DirectXCommonの取得
	/// </summary>
	/// <returns></returns>
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	/// <summary>
	/// SrvManagerの取得
	/// </summary>
	/// <returns></returns>
	SrvManager* GetSrvManager() const { return srvManager_; }


private:

	//インスタンス
	static ModelCommon* instance_;
	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	//SrvManager
	SrvManager* srvManager_ = nullptr;
};