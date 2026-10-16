#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

namespace TakeC {

/// <summary>
/// ワールドで共有するサービスを型ごとに所有し、Componentから参照できるようにします。
/// 同じ型を再登録すると旧サービスを置き換えます。
/// </summary>
class EngineServices final {
public:
	template<class T>
	void Register(std::shared_ptr<T> service) {
		static_assert(!std::is_const_v<T> && !std::is_void_v<T>);
		if (!service) {
			throw std::invalid_argument("Engine service must not be null.");
		}
		services_.insert_or_assign(std::type_index(typeid(T)), std::move(service));
	}

	template<class T>
	T* TryGet() {
		static_assert(!std::is_const_v<T> && !std::is_void_v<T>);
		const auto it = services_.find(std::type_index(typeid(T)));
		return it == services_.end() ? nullptr : static_cast<T*>(it->second.get());
	}

	template<class T>
	const T* TryGet() const {
		static_assert(!std::is_const_v<T> && !std::is_void_v<T>);
		const auto it = services_.find(std::type_index(typeid(T)));
		return it == services_.end() ? nullptr : static_cast<const T*>(it->second.get());
	}

	template<class T>
	bool Remove() {
		return services_.erase(std::type_index(typeid(T))) != 0;
	}

	void Clear() { services_.clear(); }

private:
	std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

} // namespace TakeC
