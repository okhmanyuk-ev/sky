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
		std::shared_ptr<T> attachTemporaryNode(Scene::Node& target, std::optional<std::string> key = std::nullopt)
		{
			std::shared_ptr<T> result = nullptr;

			auto type_index = std::type_index(typeid(T)).hash_code();

			auto type_key = fmt::format("{}_{}", (void*)&target, type_index);

			mTypesCount[type_key] += 1;
			auto type_count = mTypesCount.at(type_key);
			
			auto final_key = fmt::format("{}_{}", type_key, key.has_value() ? key.value() : std::to_string(type_count));

			if (mNodes.count(final_key) != 0)
			{
				auto node = mNodes.at(final_key);
				result = std::dynamic_pointer_cast<T>(node);
			}

			mNodeWasInitialized = result == nullptr;

			if (result == nullptr)
			{
				result = std::make_shared<T>();
				target.attach(result);
				assert(mNodes.count(final_key) == 0);
				mNodes.insert({ final_key, result });
			}

			mUnusedNodes.erase(final_key);

			return result;
		}

		bool nodeWasInitialized() const { return mNodeWasInitialized; }

	private:
		std::unordered_map<std::string, int> mTypesCount;
		std::unordered_map<std::string, std::shared_ptr<Scene::Node>> mNodes;
		std::set<std::string> mUnusedNodes;
		bool mNodeWasInitialized = false;
	};
}