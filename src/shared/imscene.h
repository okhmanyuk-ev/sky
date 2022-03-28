#pragma once

#include <platform/all.h>
#include <renderer/all.h>
#include <graphics/all.h>
#include <scene/all.h>

#define IMSCENE ENGINE->getSystem<Shared::ImScene>()

namespace Shared
{
	class ImScene : public Common::FrameSystem::Frameable
	{
	private:
		void onFrame() override;

	public:
		template<class T = Scene::Node>
		std::shared_ptr<T> spawn(Scene::Node& target, std::optional<std::string> key = std::nullopt)
		{
			std::shared_ptr<T> result = nullptr;
			auto type_index = std::type_index(typeid(T)).hash_code();
			auto type_key = fmt::format("{}_{}", (void*)&target, type_index);
			mTypesCount[type_key] += 1;
			auto type_count = mTypesCount.at(type_key);
			auto final_key = fmt::format("{}_{}", type_key, key.has_value() ? key.value() : std::to_string(type_count));
			if (mNodes.contains(final_key))
			{
				auto node = mNodes.at(final_key);
				result = std::dynamic_pointer_cast<T>(node);
			}
			mNodeJustSpawned = result == nullptr;
			if (result == nullptr)
			{
				result = std::make_shared<T>();
				target.attach(result);
				assert(!mNodes.contains(final_key));
				mNodes.insert({ final_key, result });
			}
			mUnusedNodes.erase(final_key);
			mLastSpawn = result;
			destroyCallback([result] {
				if (!result->hasParent())
					return;

				result->getParent()->detach(result);
			});
			return result;
		}

		bool justAllocated() const { return mNodeJustSpawned; }
		void destroyCallback(std::function<void()> func);
		void destroyAction(Actions::Collection::UAction action);
		void dontKill();
		void dontKillUntilHaveChilds();
		void showAndHideWithScale();

	private:
		std::unordered_map<std::string, int> mTypesCount;
		std::unordered_map<std::string, std::shared_ptr<Scene::Node>> mNodes;
		std::set<std::string> mUnusedNodes;
		bool mNodeJustSpawned = false;
		std::shared_ptr<Scene::Node> mLastSpawn;
		std::map<std::shared_ptr<Scene::Node>, std::function<void()>> mDestroyCallbacks;
	};
}
