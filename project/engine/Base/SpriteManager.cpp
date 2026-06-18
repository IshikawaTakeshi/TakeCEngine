#include "SpriteManager.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/WinApp.h"
#include <algorithm>
#include <format>

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
		// スプライトがアクティブな場合のみ描画
		if (sprite->IsActive()) sprite->Draw();
	}
}

void SpriteManager::Draw(SpriteDrawLayer drawLayer) {
	for (auto& sprite : sprites_) {
		if (sprite->IsActive() && sprite->GetDrawLayer() == drawLayer) {
			sprite->Draw();
		}
	}
}

//============================================================================
// ImGuiの更新
//============================================================================
void SpriteManager::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::Begin("SpriteManager");
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
		sprite->UpdateImGui(spriteLabel);
		i++;
	}

	//スプライトの生成
	if (ImGui::Button("Create Sprite")) {
		Create();
	}

	ImGui::End();

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

	//登録する予定の名前を取得
	std::string registeredName = newSprite->GetName();

	//既に同じ名前が登録されている場合、かぶり防止のために名前を変更
	if (!registeredName.empty() && namedSprites_.find(registeredName) != namedSprites_.end()) {
		int suffix = 1;
		std::string newName;
		do {
			newName = registeredName + "." + std::format("{:03}", suffix++);
		} while (namedSprites_.find(newName) != namedSprites_.end());
		newSprite->SetName(newName);
		registeredName = newName; // 更新された名前を登録用に使用
	}

	// 名前がリスト内になければ登録
	namedSprites_[registeredName] = ptr;

	sprites_.push_back(std::move(newSprite));
	return ptr;
}

//============================================================================
// スプライトをManager側で生成する関数
//============================================================================
void TakeC::SpriteManager::Create() {
	auto newSprite = std::make_unique<Sprite>();
	newSprite->Initialize(spriteCommon_, "white1x1.png");

	// 初期位置は画面中央
	newSprite->SetTranslate({
		WinApp::kScreenWidth / 2.0f,
		WinApp::kScreenHeight / 2.0f
		});
	// デフォルトサイズは128x128にしておく
	newSprite->SetSize({ 128.0f, 128.0f });

	// 名前を設定
	newSprite->SetName("newSprite");
	
	Sprite* ptr = newSprite.get();

	//登録する予定の名前を取得
	std::string registeredName = newSprite->GetName();

	//既に同じ名前が登録されている場合、かぶり防止のために名前を変更
	if (!registeredName.empty() && namedSprites_.find(registeredName) != namedSprites_.end()) {
		int suffix = 1;
		std::string newName;
		do {
			newName = registeredName + "." + std::format("{:03}", suffix++);
		} while (namedSprites_.find(newName) != namedSprites_.end());
		newSprite->SetName(newName);
		registeredName = newName; // 更新された名前を登録用に使用
	}

	// 名前がリスト内になければ登録
	namedSprites_[registeredName] = ptr;

	sprites_.push_back(std::move(newSprite));
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

void SpriteManager::DeleteSprite(Sprite* sprite) {
	auto it = std::remove_if(
		sprites_.begin(),
		sprites_.end(),
		[sprite](const std::unique_ptr<Sprite>& ptr) {
			return ptr.get() == sprite;
		});

	sprites_.erase(it, sprites_.end());
}
