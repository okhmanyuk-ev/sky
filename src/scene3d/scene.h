#pragma once

#include <core/engine.h>
#include <renderer/all.h>
#include <graphics/all.h>
#include <scene3d/node.h>

namespace Scene3D
{
	class Model;

	class Scene
	{
	public:
		Scene();

		void frame();

	public:
		auto getRoot() const { return mRoot; }
		auto getCamera() const { return mCamera; }
		auto& getDriver() { return mDriver; }

		auto getRenderTarget() const { return mRenderTarget; }
		void setRenderTarget(std::shared_ptr<Renderer::RenderTarget> value) { mRenderTarget = value; }

	private:
		std::shared_ptr<Node> mRoot;
		std::shared_ptr<Graphics::Camera3D> mCamera;
		Driver mDriver;
		std::shared_ptr<Renderer::RenderTarget> mRenderTarget = nullptr;

	private:
		void recursiveNodeUpdateTransform(std::shared_ptr<Node> node);
		void recursiveNodeUpdate(std::shared_ptr<Node> node, Clock::Duration delta);
		void recursiveNodeDraw(std::shared_ptr<Node> node);
	};
}

	
