#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "Sprite.h"
#include "Mesh/Mesh.h"
#include "base/SrvManager.h"
#include "TextureManager.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>

using namespace TakeC;

//====================================================================
// インスタンスの取得
//====================================================================
SpriteCommon& SpriteCommon::GetInstance() {	
	static SpriteCommon instance;
	return instance;
}

//====================================================================
// 初期化
//====================================================================
void SpriteCommon::Initialize(DirectXCommon* directXCommon, SrvManager* srvManager) {

	dxCommon_ = directXCommon;
	srvManager_ = srvManager;

	// 1. 通常（非インスタンス）描画用のPSO初期化
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Sprite/Sprite.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Sprite/Sprite.PS.hlsl");
	pso_->CreateGraphicPSO(
		dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		BlendState::SPRITE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		false
	);
	pso_->SetGraphicPipelineName("SpritePSO");
	rootSignature_ = pso_->GetGraphicRootSignature();

	// 2. インスタンス描画用のPSO初期化
	instancedPSO_ = std::make_unique<PSO>();
	instancedPSO_->CompileVertexShader(dxCommon_->GetDXC(), L"Sprite/SpriteInstanced.VS.hlsl");
	instancedPSO_->CompilePixelShader(dxCommon_->GetDXC(), L"Sprite/SpriteInstanced.PS.hlsl");
	instancedPSO_->CreateGraphicPSO(
		dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		BlendState::SPRITE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		false
	);
	instancedPSO_->SetGraphicPipelineName("SpriteInstancedPSO");
	instancedRootSignature_ = instancedPSO_->GetGraphicRootSignature();

	// 3. 共有ユニットクアッドMeshの初期化 (アンカーポイントはシェーダー側で調整するため 0,0 固定)
	sharedQuadMesh_ = std::make_unique<Mesh>();
	sharedQuadMesh_->InitializeMesh(dxCommon_, "white1x1.png");
	sharedQuadMesh_->InitializeVertexResourceSprite(dxCommon_->GetDevice(), { 0.0f, 0.0f });
	sharedQuadMesh_->InitializeIndexResourceSprite(dxCommon_->GetDevice());

	// 4. インスタンス化StructuredBufferの生成
	instanceResource_ = TakeC::DirectXCommon::CreateBufferResource(
		dxCommon_->GetDevice(), sizeof(SpriteInstanceData) * kMaxSpriteInstances
	);
	instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedInstanceData_));
	instanceResource_->SetName(L"SpriteCommon::instanceResource_");

	// 5. SRVの割り当てと生成
	instanceSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(
		kMaxSpriteInstances, sizeof(SpriteInstanceData), instanceResource_.Get(), instanceSrvIndex_
	);

	// 6. バッチオフセット用ConstantBufferの生成 (256バイトアライメント x 最大バッチ数)
	batchConfigResource_ = TakeC::DirectXCommon::CreateBufferResource(
		dxCommon_->GetDevice(), 256 * kMaxBatches
	);
	batchConfigResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedBatchConfigData_));
	batchConfigResource_->SetName(L"SpriteCommon::batchConfigResource_");
}

//====================================================================
// 終了処理
//====================================================================
void SpriteCommon::Finalize() {
	instancedRootSignature_.Reset();
	instancedPSO_.reset();
	sharedQuadMesh_.reset();
	if (instanceResource_) {
		instanceResource_->Unmap(0, nullptr);
		instanceResource_.Reset();
	}
	mappedInstanceData_ = nullptr;
	
	if (batchConfigResource_) {
		batchConfigResource_->Unmap(0, nullptr);
		batchConfigResource_.Reset();
	}
	mappedBatchConfigData_ = nullptr;

	srvManager_ = nullptr;

	rootSignature_.Reset();
	pso_.reset();
	dxCommon_ = nullptr;
}

//====================================================================
// 共通描画設定
//====================================================================
void SpriteCommon::PreDraw() {
	// 互換性のためのレガシー設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//====================================================================
// フレーム開始処理
//====================================================================
void SpriteCommon::BeginFrame() {
	frameInstanceCursor_ = 0;
	frameBatchCursor_ = 0;
	drawRequests_.clear();
}

//====================================================================
// 描画リクエストの登録（遅延描画用）
//====================================================================
void SpriteCommon::RegisterDrawRequest(Sprite* sprite) {
	if (sprite) {
		drawRequests_.push_back(sprite);
	}
}

//====================================================================
// 登録されたスプライトを一括ソートして描画実行
//====================================================================
void SpriteCommon::ExecuteDraws() {
	if (drawRequests_.empty()) {
		return;
	}

	// 安定ソートでレイヤー順に並べる（値が小さいほど奥＝先に描画、値が大きいほど手前＝後に描画）
	std::stable_sort(drawRequests_.begin(), drawRequests_.end(), [](const Sprite* a, const Sprite* b) {
		return a->GetLayer() < b->GetLayer();
	});

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 1. 共有ユニットクアッドの頂点・インデックスバッファをバインド
	sharedQuadMesh_->SetVertexBuffers(commandList, 0);
	commandList->IASetIndexBuffer(&sharedQuadMesh_->GetIndexBufferView());

	// 2. インスタンス記述子ヒープの設定
	srvManager_->SetDescriptorHeap();

	// 3. 全スプライトの走査と自動バッチング
	uint32_t currentInstanceIndex = frameInstanceCursor_;
	uint32_t batchIndex = frameBatchCursor_;
	size_t i = 0;

	while (i < drawRequests_.size()) {
		Sprite* startSprite = drawRequests_[i];
		if (!startSprite->IsActive()) {
			i++;
			continue;
		}

		// 安全チェック：最大バッチ数を超えたら終了
		if (batchIndex >= kMaxBatches) {
			break;
		}

		const std::string& texturePath = startSprite->GetConfig().textureFilePath_;

		// 同じテクスチャを使用する連続したスプライトを1つのバッチとしてスキャン
		size_t batchSize = 0;
		while (i + batchSize < drawRequests_.size()) {
			Sprite* currentSprite = drawRequests_[i + batchSize];
			if (!currentSprite->IsActive()) {
				batchSize++;
				continue;
			}
			if (currentSprite->GetConfig().textureFilePath_ != texturePath) {
				break;
			}
			batchSize++;
		}

		// 有効なインスタンスデータの書き込み
		uint32_t activeCount = 0;
		uint32_t batchStartIndex = currentInstanceIndex;

		for (size_t k = 0; k < batchSize; ++k) {
			Sprite* sprite = drawRequests_[i + k];
			if (!sprite->IsActive()) {
				continue;
			}

			// 容量制限チェック（最大数を超えたらそれ以上のコピーを打ち切り描画）
			if (currentInstanceIndex >= kMaxSpriteInstances) {
				break;
			}

			// インスタンスデータの設定
			SpriteInstanceData& dest = mappedInstanceData_[currentInstanceIndex];
			dest.WVP = sprite->GetWVPMatrix();
			dest.World = sprite->GetWorldMatrix();
			// 2D座標変換に伴う法線用の逆転置行列
			dest.WorldInverseTranspose = MatrixMath::Transpose(MatrixMath::Inverse(sprite->GetWorldMatrix()));
			dest.color = sprite->GetMaterialColor();
			dest.anchorPoint = sprite->GetConfig().anchorPoint_;
			dest.size = sprite->GetConfig().size_;
			dest.isFlipX = sprite->GetIsFlipX() ? 1 : 0;
			dest.isFlipY = sprite->GetIsFlipY() ? 1 : 0;

			// 切り出し範囲UVの計算
			const DirectX::TexMetadata& metadata = TakeC::TextureManager::GetInstance().GetMetadata(texturePath);
			dest.uvLeftTop.x = sprite->GetConfig().textureLeftTop_.x / metadata.width;
			dest.uvLeftTop.y = sprite->GetConfig().textureLeftTop_.y / metadata.height;
			dest.uvSize.x = sprite->GetConfig().textureSize_.x / metadata.width;
			dest.uvSize.y = sprite->GetConfig().textureSize_.y / metadata.height;

			currentInstanceIndex++;
			activeCount++;
		}

		if (activeCount > 0) {
			// バッチごとの描画状態を設定
			commandList->SetGraphicsRootSignature(instancedRootSignature_.Get());
			commandList->SetPipelineState(instancedPSO_->GetGraphicPipelineState());

			// テクスチャのバインド (register(t0))
			srvManager_->SetGraphicsRootDescriptorTable(
				instancedPSO_->GetGraphicBindResourceIndex("gTexture"),
				TakeC::TextureManager::GetInstance().GetSrvIndex(texturePath)
			);

			// インスタンスバッファのバインド (register(t1))
			srvManager_->SetGraphicsRootDescriptorTable(
				instancedPSO_->GetGraphicBindResourceIndex("gInstances"),
				instanceSrvIndex_
			);

			// バッチオフセット定数バッファの書き込みとバインド (register(b0))
			// 256バイトアライメント（uint32_t 64個分）
			uint32_t offsetIndex = batchIndex * 64;
			mappedBatchConfigData_[offsetIndex] = batchStartIndex;

			commandList->SetGraphicsRootConstantBufferView(
				instancedPSO_->GetGraphicBindResourceIndex("gBatchConfig"),
				batchConfigResource_->GetGPUVirtualAddress() + batchIndex * 256
			);

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// DrawIndexedInstanced による一括描画を実行
			// インスタンスIDのオフセットはシェーダー側で gBatchConfig.startIndex を加算して適用するため、
			// 第5引数は 0 固定にします。
			commandList->DrawIndexedInstanced(
				6,                  // 1インスタンスあたりのインデックス数
				activeCount,        // 描画インスタンス数
				0,                  // 開始インデックス位置
				0,                  // 開始頂点位置
				0                   // 開始インスタンス位置 (0固定)
			);

			batchIndex++;
		}

		i += batchSize;
	}

	frameInstanceCursor_ = currentInstanceIndex;
	frameBatchCursor_ = batchIndex;

	// キューをクリア
	drawRequests_.clear();
}
