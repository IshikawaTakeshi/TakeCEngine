#pragma once

#include "engine/GameObject/AIControllerComponent.h"
#include "engine/GameObject/GameObject.h"

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace TakeC {

/// <summary>
/// Perceptionフェーズでゲーム固有の感知処理を実行し、AIのBlackboardへ結果を書き込みます。
/// </summary>
class SensorComponent final : public Component {
public:
	using SensorCallback = std::function<void(
		const ComponentUpdateContext&, const GameObject&, Blackboard&)>;

	ComponentPhase GetUpdatePhase() const noexcept override { return ComponentPhase::Perception; }

	/// <summary>登録順に実行します。感知対象は生ポインタではなくIDで保持してください。</summary>
	void AddSensor(SensorCallback sensor) {
		if (updating_) {
			throw std::logic_error("Cannot modify sensors during perception update.");
		}
		if (!sensor) {
			throw std::invalid_argument("Sensor callback must not be empty.");
		}
		sensors_.push_back(std::move(sensor));
	}

	void ClearSensors() {
		if (updating_) {
			throw std::logic_error("Cannot modify sensors during perception update.");
		}
		sensors_.clear();
	}

	std::size_t GetSensorCount() const { return sensors_.size(); }

	/// <summary>複数AIがある場合に指定します。取り付け後に同じGameObjectのControllerを渡してください。</summary>
	void SetController(AIControllerComponent* controller) {
		if (updating_) {
			throw std::logic_error("Cannot change sensor controller during perception update.");
		}
		if (controller && &controller->GetOwner() != &GetOwner()) {
			throw std::invalid_argument("Sensor controller must belong to the same GameObject.");
		}
		controller_ = controller;
	}

protected:
	void Update(const ComponentUpdateContext& context) override {
		AIControllerComponent* controller = controller_ ? controller_ : GetOwner().GetComponent<AIControllerComponent>();
		if (!controller || !controller->IsEnabled()) {
			return;
		}
		updating_ = true;
		try {
			for (const SensorCallback& sensor : sensors_) {
				if (GetOwner().IsDestroyRequested() || !GetOwner().IsActive()) {
					break;
				}
				sensor(context, GetOwner(), controller->GetBlackboard());
			}
		} catch (...) {
			updating_ = false;
			throw;
		}
		updating_ = false;
	}

	void OnDetach() override {
		controller_ = nullptr;
	}

private:
	std::vector<SensorCallback> sensors_;
	AIControllerComponent* controller_ = nullptr;
	bool updating_ = false;
};

} // namespace TakeC
