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

	private:
		std::shared_ptr<Node> mRoot;

	public:
		auto getCamera() const { return mCamera; }

	public:
		using Vertex = Renderer::Vertex::PositionColorNormal;

	private:
		std::shared_ptr<Graphics::Camera3D> mCamera;
		Driver mDriver;

	private:
		void recursiveNodeUpdateTransform(std::shared_ptr<Node> node);
		void recursiveNodeUpdate(std::shared_ptr<Node> node, Clock::Duration delta);
		void recursiveNodeDraw(std::shared_ptr<Node> node);
	};
}

	
