#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <type_traits>

//==============================================================================
//		LightCounter class
//==============================================================================

template<typename LightT>
class LightCounter {
public:

	static const std::size_t MaxLights_ = 128; // 最大ライト数

	LightCounter() = default;
	~LightCounter() = default;

	//初期化
	void Initialize();
	//ライト追加
	uint32_t Add(const LightT& data);
	//ライト削除
	bool Remove(uint32_t index);
	//ライト数取得
	uint32_t GetCount() const;
	//ライトデータ取得
	LightT* GetLightData(uint32_t index);

private:

	std::array<LightT, MaxLights_> data_; // ライトデータ配列
	std::array<bool, MaxLights_> used_{}; // 使用中フラグ
	std::vector<uint32_t> freeList_; // 空きインデックスリスト
	std::size_t count_ = 0; // 現在のライト数
};
