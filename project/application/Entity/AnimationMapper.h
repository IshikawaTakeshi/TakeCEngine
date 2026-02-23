#pragma once
#include "engine/Animation/NodeAnimation.h"
#include "application/Entity/GameCharacterBehavior.h"
#include <map>

//============================================================================
// AnimationEntry struct
//============================================================================
struct AnimationEntry {
	Animation* animation = nullptr; //アニメーションへの非所有ポインタ
	float blendDuration = 0.2f;    //ブレンドにかかる時間（秒）
};

//============================================================================
// AnimationMapper class
// GameCharacterBehavior から AnimationEntry へのマッピングを管理する
//============================================================================
class AnimationMapper {
public:

	/// <summary>
	/// ビヘイビアとアニメーションの対応を登録
	/// </summary>
	/// <param name="behavior">対象のビヘイビア</param>
	/// <param name="animation">対応するアニメーション（非所有）</param>
	/// <param name="blendDuration">ブレンドにかかる時間（秒）</param>
	void Register(GameCharacterBehavior behavior, Animation* animation, float blendDuration = 0.2f) {
		entries_[behavior] = { animation, blendDuration };
	}

	/// <summary>
	/// ビヘイビアに対応するエントリを検索
	/// </summary>
	/// <param name="behavior">検索するビヘイビア</param>
	/// <returns>見つかった場合はエントリのポインタ、なければ nullptr</returns>
	const AnimationEntry* Find(GameCharacterBehavior behavior) const {
		auto it = entries_.find(behavior);
		if (it != entries_.end()) {
			return &it->second;
		}
		return nullptr;
	}

private:
	std::map<GameCharacterBehavior, AnimationEntry> entries_;
};
