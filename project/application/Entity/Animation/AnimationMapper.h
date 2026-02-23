#pragma once
#include "application/Entity/GameCharacterBehavior.h"
#include "engine/Animation/NodeAnimation.h"
#include <unordered_map>


//============================================================================
// AnimationMapper
//============================================================================
class AnimationMapper {
public:
	//--------------------------------------------------------------------
	// ビヘイビア → アニメーション のエントリ
	//--------------------------------------------------------------------
	struct Entry {
		Animation* animation = nullptr;
		float blendDuration = 0.2f; // 遷移にかける時間（秒）
	};

	/// <summary>
	/// Behaviorとアニメーションを登録する
	/// </summary>
	/// <param name="behavior">対象のビヘイビア</param>
	/// <param name="animation">対応するアニメーション（AnimationManagerから取得）</param>
	/// <param name="blendDuration">遷移時間（秒）</param>
	void Register(GameCharacterBehavior behavior, Animation* animation,
		float blendDuration = 0.2f);

	/// <summary>
	/// Behaviorに対応するEntryを返す。登録されていない場合はnullptrを返す
	/// </summary>
	const Entry* Get(GameCharacterBehavior behavior) const;

private:
	std::unordered_map<GameCharacterBehavior, Entry> map_;
};
