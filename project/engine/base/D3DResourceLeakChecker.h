#pragma once

//=============================================================================
// D3DResourceLeakChecker class
//=============================================================================
/// <summary>
/// 終了時にDirect3Dリソースの解放漏れを検出・報告するクラスです。
/// </summary>
class D3DResourceLeakChecker {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~D3DResourceLeakChecker();
};

