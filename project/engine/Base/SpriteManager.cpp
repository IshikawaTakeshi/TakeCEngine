#include "SpriteManager.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/base/ImGuiManager.h"


using namespace TakeC;

//============================================================================
// 初期化
//============================================================================
void SpriteManager::Initialize(SpriteCommon* spriteCommon) {
	spriteCommon_ = spriteCommon;
	Clear();
}

//============================================================================
// 更新処理 (全スプライト)
//============================================================================
void SpriteManager::Update() {
	for (auto& sprite : sprites_) {
		sprite->Update();
	}
}

//============================================================================
// 描画処理 (全スプライト)
//============================================================================
void SpriteManager::Draw() {
	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
}

//============================================================================
// ImGuiの更新
//============================================================================
void SpriteManager::UpdateImGui(const std::string& groupName) {
	groupName;
#ifdef _DEBUG
	if (ImGui::TreeNode(groupName.c_str())) {
		int i = 0;
		for (auto& sprite : sprites_) {
			// 名前付きの場合は名前を表示、なければ連番
			std::string spriteLabel = "Sprite_" + std::to_string(i);

			// マップを逆引きして名前を探す（デバッグ用なので簡易的）
			for (const auto& pair : namedSprites_) {
				if (pair.second == sprite.get()) {
					spriteLabel = pair.first;
					break;
				}
			}
			groupName;
			sprite->UpdateImGui(spriteLabel);
			i++;
		}
		ImGui::TreePop();
	}
#endif
}

//============================================================================
// スプライトの生成と登録 (テクスチャ指定)
//============================================================================
Sprite* SpriteManager::Create(const std::string& filePath,
	const Vector2& position,
	const std::string& name) {
	if (!spriteCommon_)
		return nullptr;

	auto newSprite = std::make_unique<Sprite>();
	newSprite->Initialize(spriteCommon_, filePath);

	newSprite->SetTranslate(position);
	newSprite->AdjustTextureSize();

	// 名前を設定
	if (!name.empty()) {
		newSprite->SetName(name);
	}

	Sprite* ptr = newSprite.get();

	// 名前があれば登録
	std::string registeredName = newSprite->GetName();
	if (!registeredName.empty()) {
		namedSprites_[registeredName] = ptr;
	}

	sprites_.push_back(std::move(newSprite));
	return ptr;
}

//============================================================================
// JSONファイルからスプライトを生成して登録
//============================================================================
Sprite* SpriteManager::CreateFromJson(const std::string& jsonFilePath) {
	if (!spriteCommon_)
		return nullptr;

	auto newSprite = std::make_unique<Sprite>();

	// JSON読み込み (ここで spriteConfig_.name_ も読み込まれる)
	newSprite->LoadConfig(jsonFilePath);

	// 初期化
	newSprite->Initialize(spriteCommon_);

	Sprite* ptr = newSprite.get();

	// Configに保存されていた名前で登録
	std::string registeredName = newSprite->GetName();
	if (!registeredName.empty()) {
		// マップに登録（キーはJSON内の名前）
		namedSprites_[registeredName] = ptr;
	}

	sprites_.push_back(std::move(newSprite));
	return ptr;
}

//============================================================================
// 名前によるスプライトの取得
//============================================================================
Sprite* SpriteManager::GetSprite(const std::string& name) {
	auto it = namedSprites_.find(name);
	if (it != namedSprites_.end()) {
		return it->second;
	}
	return nullptr;
}

//============================================================================
// 全スプライトのクリア
//============================================================================
void SpriteManager::Clear() {
	sprites_.clear();
	namedSprites_.clear();
}