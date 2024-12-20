#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

class PSO;
class DirectXCommon;
class SpriteCommon {
public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			エイリアステンプレート
	/////////////////////////////////////////////////////////////////////////////////////

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	/////////////////////////////////////////////////////////////////////////////////////
	///			publicメンバ関数
	/////////////////////////////////////////////////////////////////////////////////////

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

	/// <summary>
	/// 描画後処理
	/// </summary>
	//void PostDraw();
	
public:

	/////////////////////////////////////////////////////////////////////////////////////
	///			getter
	/////////////////////////////////////////////////////////////////////////////////////

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

public:

	/////////////////////////////////////////////////////////////////////////////////////
	///			setter
	/////////////////////////////////////////////////////////////////////////////////////

	void SetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }

private:
	
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;


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

