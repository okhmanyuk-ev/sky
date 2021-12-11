#pragma once

// experimental stuff

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <unordered_map>
#include <memory>
#include <stack>
#include <platform/system.h>
#include <renderer/all.h>
#include <graphics/tex_region.h>
#include <graphics/color.h>
#include <graphics/font.h>
#include <scene/label.h>

#define IMSCENE ENGINE->getSystem<ImScene::ImScene>()

namespace ImScene
{
	class ImScene : public Common::FrameSystem::Frameable
	{
	private:
		void onFrame() override;

	public:
		template<class T>
		std::shared_ptr<T> pushTemporaryNode(std::shared_ptr<Scene::Node> target, const std::string& unique_name)
		{
			std::shared_ptr<T> result = nullptr;

			if (mImNodes.count(unique_name) != 0)
			{
				auto node = mImNodes.at(unique_name);
				result = std::dynamic_pointer_cast<T>(node);
			}

			if (result == nullptr)
			{
				result = std::make_shared<T>();
				target->attach(result);
				assert(mImNodes.count(unique_name) == 0);
				mImNodes.insert({ unique_name, result });
			}

			mUnusedImNodes.erase(unique_name);

			return result;
		}

	private:
		std::map<std::string, std::shared_ptr<Scene::Node>> mImNodes;
		std::set<std::string> mUnusedImNodes;
	};
}