#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

// 前方宣言
class PSO;
class DirectXCommon;

//============================================================================
// SpriteCommon class
//============================================================================
class SpriteCommon {
private:

	//コンストラクタ・デストラクタ・コピー禁止
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

public:

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	//========================================================================
	// functions
	//========================================================================
	static SpriteCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void PreDraw();
	
public:

	//=========================================================================
	// accessors
	//=========================================================================

	//----- getter ---------------------------

	//DirectXCommonの取得
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }


	//----- setter ---------------------------

	//DirectXCommonの設定
	void SetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }



private:

	/////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	/////////////////////////////////////////////////////////////////////////////////////

	//シングルトンインスタンス
	static SpriteCommon* instance_;

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//PipelineStateObject
	std::unique_ptr<PSO> pso_ = nullptr;

};