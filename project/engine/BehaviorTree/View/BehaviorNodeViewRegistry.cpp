#include "BehaviorNodeViewRegistry.h"

#include <algorithm>
#include <utility>

namespace TakeC {

bool BehaviorNodeViewRegistry::Register(
	BehaviorNodeViewRegistration registration,
	bool replaceExisting) {
	if (registration.typeId.empty() || !registration.createAt || !registration.place) {
		return false;
	}

	auto it = registrations_.find(registration.typeId);
	if (it != registrations_.end() && !replaceExisting) {
		return false;
	}

	std::string typeId = registration.typeId;
	registrations_.insert_or_assign(std::move(typeId), std::move(registration));
	return true;
}

std::shared_ptr<BehaviorNodeView> BehaviorNodeViewRegistry::CreateAt(
	std::string_view typeId,
	ImFlow::ImNodeFlow& editor,
	const ImVec2& position,
	const BehaviorNodeData& data) const {
	auto it = registrations_.find(std::string(typeId));
	if (it == registrations_.end()) {
		return nullptr;
	}
	return it->second.createAt(editor, position, data);
}

bool BehaviorNodeViewRegistry::Place(std::string_view typeId, ImFlow::ImNodeFlow& editor) const {
	auto it = registrations_.find(std::string(typeId));
	if (it == registrations_.end()) {
		return false;
	}
	it->second.place(editor);
	return true;
}

bool BehaviorNodeViewRegistry::Contains(std::string_view typeId) const {
	return registrations_.contains(std::string(typeId));
}

bool BehaviorNodeViewRegistry::Unregister(std::string_view typeId) {
	return registrations_.erase(std::string(typeId)) != 0;
}

void BehaviorNodeViewRegistry::Clear() {
	registrations_.clear();
}

std::vector<const BehaviorNodeViewRegistration*> BehaviorNodeViewRegistry::Registrations() const {
	std::vector<const BehaviorNodeViewRegistration*> result;
	result.reserve(registrations_.size());
	for (const auto& [typeId, registration] : registrations_) {
		(void)typeId;
		result.push_back(&registration);
	}
	std::sort(result.begin(), result.end(), [](const auto* lhs, const auto* rhs) {
		if (lhs->category != rhs->category) {
			return lhs->category < rhs->category;
		}
		return lhs->displayName < rhs->displayName;
	});
	return result;
}

} // namespace TakeC
