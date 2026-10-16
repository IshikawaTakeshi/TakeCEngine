#pragma once

#include "engine/GameObject/AIControllerComponent.h"
#include "engine/BehaviorTree/BehaviorTreeBrain.h"

namespace TakeC {

	/// <summary>
	/// AIControllerへBehaviorTreeBrainを載せるComponent
	/// </summary>
	class BehaviorTreeComponent final : public AIControllerComponent {
	public:

		//===================================================================
		// functions
		//===================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		BehaviorTreeComponent();
		~BehaviorTreeComponent() override = default;

		/// <summary>
		/// BehaviorTreeBrainのActionNodeに対して、実行コールバックの登録
		/// </summary>
		/// <param name="actionId"></param>
		/// <param name="execute"></param>
		/// <param name="reset"></param>
		void BindAction(
			std::string actionId, ActionNode::ContextExecuteCallback execute,
			ActionNode::ResetCallback reset = {});
	public:

		//===================================================================
		// accessors
		//===================================================================

		//---- getters ----
		BehaviorTreeBrain& GetBehaviorTreeBrain();
		const BehaviorTreeBrain& GetBehaviorTreeBrain() const;
		BehaviorNode* GetTree() const { return GetBehaviorTreeBrain().GetTree(); }
		BehaviorStatus GetStatus() const { return GetBehaviorTreeBrain().GetStatus(); }
		bool IsRepeat() const { return GetBehaviorTreeBrain().IsRepeat(); }

		//---- setters ----
		void SetBrain(std::unique_ptr<IAgentBrain> brain) override;
		void SetTree(std::unique_ptr<BehaviorNode> root);
		void SetRepeat(bool repeat) { GetBehaviorTreeBrain().SetRepeat(repeat); }
		void Resume() override;
	};
} // namespace TakeC
