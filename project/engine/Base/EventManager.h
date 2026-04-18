#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <any>


//======================================================================
// EventManager class
//======================================================================

namespace TakeC {

	class EventManager {
	public:

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		EventManager() = default;
		~EventManager() = default;

		// 実行したいイベントのコールバック関数の型
		using EventCallback = std::function<void(const std::any&)>;
		using EventId = uint32_t;

		struct ObserverInfo {
			EventId id;
			EventCallback callback;
		};

		//===================================================================
		// functions
		//===================================================================

		/// <summary>
		/// オブザーバーの追加
		/// </summary>
		/// <param name="eventName"></param>
		/// <param name="callback"></param>
		EventId AddObserver(const std::string& eventName, EventCallback callback);

		/// <summary>
		/// イベントの発行
		/// </summary>
		/// <param name="eventName"></param>
		void PostEvent(const std::string& eventName);

		/// <summary>
		/// イベントの発行（データ付き）
		/// </summary>
		/// <param name="eventName"></param>
		/// <param name="eventData"></param>
		void PostEvent(const std::string& eventName, const std::any& eventData);

		/// <summary>
		/// 全てのオブザーバーの削除
		/// </summary>
		/// <param name="eventName"></param>
		void ClearObservers();

		/// <summary>
		/// オブザーバーの個別削除
		/// </summary>
		void RemoveObserver(const std::string& eventName, EventId id);

	private:

		// イベント名とコールバック関数のリストを紐づけるマップ
		std::map<std::string, std::vector<ObserverInfo>> observers_;
		// 次に登録されるイベントオブザーバーのIDを生成するためのカウンター
		EventId nextEventrId_ = 0; 
	};

}