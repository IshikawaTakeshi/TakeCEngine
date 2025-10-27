#include "CollisionManager.h"
#include "Collider.h"
#include "Vector3Math.h"
#include "DirectXCommon.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"

CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CollisionManager();
	}
	return instance_;
}

//=============================================================================
// 初期化
//=============================================================================

void CollisionManager::Initialize(DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_WIREFRAME, D3D12_DEPTH_WRITE_MASK_ZERO);
	pso_->SetGraphicPipelineName("CollisionPSO");
	rootSignature_ = pso_->GetGraphicRootSignature();
}

//=============================================================================
// 描画前処理
//=============================================================================

void CollisionManager::PreDraw() {
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
}

//=============================================================================
// シングルトンインスタンスの解放処理
//=============================================================================

void CollisionManager::Finalize() {
	pso_.reset();
	rootSignature_.Reset();
	delete instance_;
	instance_ = nullptr;
}

//=============================================================================
// ゲームキャラクターの登録・解放・衝突判定
//=============================================================================
void CollisionManager::RegisterGameCharacter(GameCharacter* gameCharacter) {
	colliders_.push_back(gameCharacter->GetCollider());
	gameCharacters_.push_back(gameCharacter);
}

//=============================================================================
// ゲームキャラクターの全解放
//=============================================================================
void CollisionManager::ClearGameCharacter() {
	colliders_.clear();
	gameCharacters_.clear();
}

//=============================================================================
// ゲームキャラクター同士の全衝突判定
//=============================================================================
void CollisionManager::CheckAllCollisionsForGameCharacter() {

	//リスト内のペアを総当たり
	std::list<GameCharacter*>::iterator itrA = gameCharacters_.begin();
	for (; itrA != gameCharacters_.end(); ++itrA) {
		//イテレータAからコライダーAを取得する
		GameCharacter* gameCharacterA = *itrA;
		//イテレータA+1からコライダーBを取得する(重複判定の回避)
		std::list<GameCharacter*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != gameCharacters_.end(); ++itrB) {
			//イテレータBからコライダーBを取得する
			GameCharacter* gameCharacterB = *itrB;
			//コライダーAとBの衝突判定
			CheckCollisionPairForGameCharacter(gameCharacterA, gameCharacterB);
		}
	}
}

//=============================================================================
// ゲームキャラクター同士の衝突判定
//=============================================================================
void CollisionManager::CheckCollisionPairForGameCharacter(GameCharacter* gameCharacterA, GameCharacter* gameCharacterB) {

	//コライダーAとB
	Collider* colliderA = gameCharacterA->GetCollider();
	Collider* colliderB = gameCharacterB->GetCollider();

	//コライダーがnullptrだったらreturn
	if (!colliderA || !colliderB) return;

	if( gameCharacterA->GetCharacterType() == gameCharacterB->GetCharacterType()) {
		//同じキャラクタータイプ同士の衝突は無視
		return;
	}

	//コライダーAとBの衝突判定
	if (colliderA->CheckCollision(colliderB)) {
		
		//コライダーAの衝突処理
		gameCharacterA->OnCollisionAction(gameCharacterB);
		//コライダーBの衝突処理
		gameCharacterB->OnCollisionAction(gameCharacterA);
	}
}

//=============================================================================
// レイキャスト処理
//=============================================================================
bool CollisionManager::RayCast(const Ray& ray, RayCastHit& outHit,uint32_t layerMask) {

	bool result = false;
	float closestDistance = ray.distance;
	RayCastHit tempHit;
	for (auto* collider : colliders_) {
		// レイヤーマスクによる絞り込み
		if (!(static_cast<uint32_t>(collider->GetCollisionLayerID()) & layerMask)) continue;

		// レイとコライダーの交差判定
		if (collider->Intersects(ray, tempHit)) {
			// 最も近いヒットを記録
			if (tempHit.distance < closestDistance) {
				closestDistance = tempHit.distance;
				outHit = tempHit;
				result = true;
			}
		}
	}
	return result;
}
