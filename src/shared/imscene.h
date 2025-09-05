#pragma once

#include <skygfx/skygfx.h>
#include <graphics/all.h>
#include <scene/all.h>

#define IMSCENE sky::Locator<Shared::ImScene>::Get()

namespace Shared
{
	class ImScene : public sky::Scheduler::Frameable
	{
	private:
		void onFrame() override;

	private:
		struct NodeItem
		{
			NodeItem(std::shared_ptr<Scene::Node> node, std::function<void()> destroy_func);
			std::shared_ptr<Scene::Node> node;
			std::function<void()> destroy_func;
		};

	public:
		template<class T = Scene::Node, typename... Args>
		std::shared_ptr<T> spawn(Scene::Node& target, std::optional<std::string> key = std::nullopt, Args&&... args)
		{
			std::shared_ptr<T> result = nullptr;
			auto type_index = std::type_index(typeid(T)).hash_code();
			auto type_key = fmt::format("{}_{}", (void*)&target, type_index);
			mTypesCount[type_key] += 1;
			auto type_count = mTypesCount.at(type_key);
			auto final_key = fmt::format("{}_{}", type_key, key.has_value() ? key.value() : std::to_string(type_count));
			if (mNodeItems.contains(final_key))
			{
				const auto& item = mNodeItems.at(final_key);
				result = std::dynamic_pointer_cast<T>(item.node);
			}
			mFirstCall = result == nullptr;
			if (result == nullptr)
			{
				result = std::make_shared<T>(args...);
				target.attach(result);
				auto item = NodeItem(result, [result] {
					if (result->hasParent())
						result->getParent()->detach(result);
				});
				mNodeItems.insert({ final_key, item });
			}
			mUnusedNodes.erase(final_key);
			mLastSpawnedKey = final_key;
			return result;
		}

		bool isFirstCall() const { return mFirstCall; }
		void destroyCallback(std::function<void()> func);
		void destroyAction(sky::Actions::Action action);
		void dontKill();
		void dontKillWhileHaveChilds();
		void showAndHideWithScale();
		void showWithAlpha(float duration = 0.25f, float dst_alpha = 1.0f);
		void hideWithAlpha(std::shared_ptr<Scene::Color> color, float duration = 0.25f);
		void hideWithAlpha(float duration = 0.25f);

	private:
		std::unordered_map<std::string, int> mTypesCount;
		std::unordered_map<std::string, NodeItem> mNodeItems;
		std::set<std::string> mUnusedNodes;
		bool mFirstCall = false;
		std::string mLastSpawnedKey;
	};
}
