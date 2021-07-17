#include "emitter.h"
#include "sprite.h"
#include "rectangle.h"
#include "actions.h"

using namespace Scene;

// emitter

Emitter::Emitter()
{
	runAction(Actions::Collection::RepeatInfinite([this]()->Actions::Collection::UAction {
		if (!mRunning)
			return nullptr;

		auto delay = glm::linearRand(mMinDelay, mMaxDelay);
		return Actions::Collection::Delayed(delay, Actions::Collection::Execute([this] {
			if (!mRunning)
				return;

			emit();
		}));
	}));
}

void Emitter::emit(int count)
{
	assert(!mHolder.expired());
	assert(count > 0);

	for (int i = 0; i < count - 1; i++)
	{
		emit();
	}

	auto holder = mHolder.lock();

	if (!holder->isTransformReady())
		return;

	if (!holder->hasScene())
		return;

	auto particle = createParticle();
	particle->setPosition(holder->unproject(project(getAbsoluteSize() * glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f)))));
	particle->setScale(mBeginScale);
	particle->setPivot(0.5f);
	particle->setRotation(glm::radians(glm::linearRand(0.0f, 360.0f)));

	auto colored_particle = std::dynamic_pointer_cast<Color>(particle);
	colored_particle->setColor(mBeginColor);
	colored_particle->setAlpha(0.0f);

	auto duration = glm::linearRand(mMinDuration, mMaxDuration);
	auto direction = glm::linearRand(mMinDirection, mMaxDirection);

	particle->runAction(Actions::Collection::MakeSequence(
		Actions::Collection::MakeParallel(
			Actions::Collection::ChangePosition(particle, particle->getPosition() + (direction * mDistance), duration, Easing::CubicOut),
			Actions::Collection::ChangeScale(particle, mEndScale, duration),
			Actions::Collection::ChangeColor(colored_particle, mBeginColor, mEndColor, duration),
			Actions::Collection::ChangeAlpha(colored_particle, mBeginColor.a, mEndColor.a, duration)
		),
		Actions::Collection::Kill(particle)
	));

	holder->attach(particle);
}

// sprite emitter

std::shared_ptr<Node> SpriteEmitter::createParticle()
{
	auto particle = std::make_shared<Sprite>();
	particle->setTexture(mTexture);
	particle->setSampler(getSampler());
	particle->setBlendMode(getBlendMode());
	return particle;
}

// rectangle emitter

std::shared_ptr<Node> RectangleEmitter::createParticle()
{
	auto particle = std::make_shared<Rectangle>();
	particle->setRounding(mRounding);
	particle->setSize(mBeginSize);
	return particle;
}