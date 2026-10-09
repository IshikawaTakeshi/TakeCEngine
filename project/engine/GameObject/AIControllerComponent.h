#pragma once

#include "engine/GameObject/Component.h"
#include "engine/AI/IAgentBrain.h"

#include <memory>

namespace TakeC {

	/// <summary>
	/// 意思決定AIのBrainクラスを保持するコンポーネント
	/// </summary>
	class AIControllerComponent : public Component {
	public:
		AIControllerComponent() = default;
		~AIControllerComponent() override = default;

		/// 旧BrainをResetして交換。Blackboardは保持し、次の有効更新で新BrainをStartします。
		virtual void SetBrain(std::unique_ptr<IAgentBrain> brain);
		IAgentBrain* GetBrain() { return brain_.get(); }
		const IAgentBrain* GetBrain() const { return brain_.get(); }
		Blackboard& GetBlackboard() { return blackboard_; }
		const Blackboard& GetBlackboard() const { return blackboard_; }

		/// 0は毎フレーム。間引く場合、Tickには蓄積した時間を渡します。
		void SetTickInterval(float seconds);
		float GetTickInterval() const { return tickInterval_; }
		bool IsPlaying() const { return playing_; }
		void Pause() { playing_ = false; }
		virtual void Resume() { playing_ = brain_ != nullptr; }
		void Stop();
		void Restart();
		void Reset();

	protected:
		/// Brain内のResetコールバックからの構造変更も防ぎます。
		class ExecutionGuard final {
		public:
			explicit ExecutionGuard(AIControllerComponent& controller);
			~ExecutionGuard();
			ExecutionGuard(const ExecutionGuard&) = delete;
			ExecutionGuard& operator=(const ExecutionGuard&) = delete;
		private:
			AIControllerComponent& controller_;
		};
		void OnAttach() final;
		void Update(const ComponentUpdateContext& context) final;
		void OnDetach() final;
		void EnsureNotExecuting() const;
		void ResetTickClock() { elapsedTime_ = 0.0f; }

	private:
		std::unique_ptr<IAgentBrain> brain_;
		Blackboard blackboard_;
		GameObject* attachedOwner_ = nullptr;
		float tickInterval_ = 0.0f;
		float elapsedTime_ = 0.0f;
		bool playing_ = false;
		bool brainStarted_ = false;
		bool executing_ = false;
	};
} // namespace TakeC
