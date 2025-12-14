#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/PipelineStateObject.h"
#include "PostEffect/PostEffect.h"
#include "PostEffect/PostEffectFactory.h"
#include <string>
#include <vector>

//中間リソースの種類列挙型
enum IntermediateResourceType {
	FRONT,
	BACK,
};

//名前付きPostEffect構造体
struct NamedPostEffect {
	std::string name;
	std::unique_ptr<PostEffect> postEffect;
};

//=============================================================================
//	PostEffectManager class
//=============================================================================

//前方宣言
class RenderTexture;

namespace TakeC {
	class PostEffectManager {
	public:

		//=======================================================================
		/// functions
		//=======================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		PostEffectManager() = default;
		~PostEffectManager() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,RenderTexture* renderTexture);

		/// <summary>
		/// ImGui更新処理
		/// </summary>
		void UpdateImGui();

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		/// <summary>
		///	描画処理
		/// </summary>
		void Draw(PSO* pso);

		/// <summary>
		/// 全PostEffectのCSによる処理
		/// </summary>
		void AllDispatch();

		/// <summary>
		/// エフェクトの適用
		/// </summary>
		/// <param name="name"></param>
		void ApplyEffect(const std::string& name);

		/// <summary>
		/// エフェクトの初期化・追加
		/// </summary>
		/// <param name="name"></param>
		/// <param name="csFilePath"></param>
		void InitializeEffect(const std::string& name, const std::wstring& csFilePath);

	public:

		//=======================================================================
		// accessors
		//=======================================================================

		//----- getter ---------------------------

		// 最終出力SRVインデックスの取得
		uint32_t GetFinalOutputSrvIndex() const;

		//----- setter ---------------------------

		void SetPostEffectFactory(PostEffectFactory* factory) {
			postEffectFactory_ = factory;
		}

		void SetLightCameraRenderTexture(RenderTexture* renderTexture) {
			lightCameraRenderTexture_ = renderTexture;
		}

	private:

		TakeC::DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
		TakeC::SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ
		PostEffectFactory* postEffectFactory_; //PostEffectFactoryのポインタ

		RenderTexture* renderTexture_ = nullptr; //RenderTextureのポインタ
		RenderTexture* lightCameraRenderTexture_ = nullptr; //ライトカメラ用RenderTextureのポインタ

		//srv/uavとして利用する中間リソース
		ComPtr<ID3D12Resource> intermediateResource_[2];
		uint32_t srvIndex_[2] = {};
		uint32_t uavIndex_[2] = {};

		//最終出力用テクスチャリソース
		ComPtr<ID3D12Resource> renderTextureResource_;
		uint32_t renderTextureSrvIndex_ = 0;
		uint32_t finalOutputSrvIndex_ = 0;

		// Ping-Pongバッファの切り替えフラグ
		bool currentWriteBufferIsA_ = true;

		//postEffectのコンテナ
		std::vector<NamedPostEffect> postEffects_;
	};
}