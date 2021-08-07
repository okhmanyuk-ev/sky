#include "scene.h"

using namespace Scene3D;

Scene::Scene()
{
	mRoot = std::make_shared<Node>();

	mCamera = std::make_shared<Graphics::Camera3D>();
	mCamera->setYaw(glm::radians(90.0f));
	mCamera->setPosition({ 0.0f, 0.0f, -1000.0f });
}

void Scene::frame()
{
	mCamera->onFrame();

	recursiveNodeUpdate(mRoot, FRAME->getTimeDelta());
	recursiveNodeUpdateTransform(mRoot);
	
	mDriver.setCameraPosition(mCamera->getPosition());

	GRAPHICS->begin();
	GRAPHICS->pushViewport(Renderer::Viewport());
	GRAPHICS->pushProjectionMatrix(mCamera->getProjectionMatrix());
	GRAPHICS->pushViewMatrix(mCamera->getViewMatrix());
	GRAPHICS->pushScissor(std::nullopt);
	GRAPHICS->pushCullMode(Renderer::CullMode::None);
	GRAPHICS->pushDepthMode(Renderer::ComparisonFunc::Less);
	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->pushTextureAddress(Renderer::TextureAddress::Wrap);
	recursiveNodeDraw(mRoot);
	GRAPHICS->pop(8);
	GRAPHICS->end();
}

void Scene::recursiveNodeUpdateTransform(std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	node->updateTransform();

	for (auto _node : node->getNodes())
		recursiveNodeUpdateTransform(_node);
}

void Scene::recursiveNodeUpdate(std::shared_ptr<Node> node, Clock::Duration delta)
{
	if (!node->isEnabled())
		return;

	node->update(delta);

	for (auto _node : node->getNodes())
		recursiveNodeUpdate(_node, delta);
}

void Scene::recursiveNodeDraw(std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	if (!node->isVisible())
		return;

	if (!node->isTransformReady())
		return;

	node->enterDraw(mDriver);
	node->draw(mDriver);

	for (auto _node : node->getNodes())
		recursiveNodeDraw(_node);

	node->leaveDraw(mDriver);
}