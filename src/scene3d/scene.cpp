#include "scene.h"

using namespace Scene3D;

Scene::Scene()
{
	mRoot = std::make_shared<Node>();

	mShader = std::make_shared<Renderer::Shaders::Light>(Vertex::Layout);
	mCamera = std::make_shared<Graphics::Camera3D>(70.0f, glm::vec3(0.0f, -1.0f, 0.0f));
	mCamera->setYaw(glm::radians(90.0f));
	mCamera->setPosition({ 0.0f, 0.0f, -1000.0f });

	auto light = mShader->getPointLight();
	light.position = { 0.0f, 0.0f, -500.0f };
	mShader->setPointLight(light);
	mShader->setMaterial(Renderer::Shaders::Materials::Emerald);
}

void Scene::frame()
{
	mCamera->onFrame();

	auto view = mCamera->getViewMatrix();
	auto projection = mCamera->getProjectionMatrix();
	auto model = glm::mat4(1.0f);

	mShader->setProjectionMatrix(projection);
	mShader->setViewMatrix(view);
	mShader->setModelMatrix(model);

	mShader->setEyePosition(mCamera->getPosition());

	RENDERER->setViewport(Renderer::Viewport());
	RENDERER->setScissor(nullptr);
	RENDERER->setCullMode(Renderer::CullMode::None); // TODO: not "None"!
	RENDERER->setDepthMode(Renderer::DepthMode(Renderer::ComparisonFunc::Less));
	RENDERER->setShader(mShader);

	recursiveNodeUpdate(mRoot, FRAME->getTimeDelta());
	recursiveNodeUpdateTransform(mRoot);
	recursiveNodeDraw(mRoot);
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

	node->enterDraw();

	mShader->setModelMatrix(node->getTransform()); // TODO: this bad place for it

	node->draw();

	for (auto _node : node->getNodes())
		recursiveNodeDraw(_node);

	node->leaveDraw();
}