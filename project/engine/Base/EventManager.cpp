#include "EventManager.h"

using namespace TakeC;

//======================================================================
//	オブザーバーの追加
//======================================================================
EventManager::EventId EventManager::AddObserver(const std::string& eventName, EventCallback callback) {
	// イベント名に対応するコールバック関数のリストに、コールバック関数を追加
	EventId id = ++nextEventrId_; // 新しいIDを生成
	observers_[eventName].push_back({ id, callback });
	return id; // 登録したIDを返す
}

//======================================================================
//	イベントの発行
//======================================================================
void EventManager::PostEvent(const std::string& eventName) {
	PostEvent(eventName, std::any()); // データなしのイベントは空のstd::anyを渡す
}

void TakeC::EventManager::PostEvent(const std::string& eventName, const std::any& eventData) {

	if (observers_.find(eventName) != observers_.end()) {
		// イベント名に対応するコールバック関数のリストを取得
		const auto& callbacks = observers_[eventName];
		// 各コールバック関数を呼び出す
		for (const auto& callback : callbacks) {
			callback.callback(eventData);
		}
	}
}

//======================================================================
//	全てのオブザーバーの削除
//======================================================================
void EventManager::ClearObservers() {
	observers_.clear();
}

//======================================================================
//	オブザーバーの個別削除
//======================================================================
void TakeC::EventManager::RemoveObserver(const std::string& eventName, EventId id) {
	if (observers_.find(eventName) != observers_.end()) {
		auto& callbacks = observers_[eventName];
		// IDが一致するものを削除する
		callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
			[id](const ObserverInfo& observer) { return observer.id == id; }),
			callbacks.end());
	}
}
