#include "EventManager.h"

using namespace TakeC;

//======================================================================
//	オブザーバーの追加
//======================================================================
void EventManager::AddObserver(const std::string& eventName, EventCallback callback) {
	// イベント名に対応するコールバック関数のリストに、コールバック関数を追加
	observers_[eventName].push_back(callback);
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
			callback(eventData);
		}
	}
}

//======================================================================
//	全てのオブザーバーの削除
//======================================================================
void EventManager::ClearObservers() {
	observers_.clear();
}
