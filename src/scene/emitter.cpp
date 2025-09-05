#include "emitter.h"
#include "sprite.h"
#include "rectangle.h"
#include "actions.h"

using namespace Scene;

// emitter

Emitter::Emitter()
{
	runAction(Actions::Collection::RepeatInfinite([this]()->std::unique_ptr<Actions::Action> {
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

	assert(mCreateParticleCallback);

	auto particle = mCreateParticleCallback();
	particle->setPosition(holder->unproject(project(getAbsoluteSize() * glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f)))));
	particle->setPivot(0.5f);
	particle->setRotation(glm::radians(glm::linearRand(0.0f, 360.0f)));

	auto colored_particle = std::dynamic_pointer_cast<Color>(particle);
	
	auto duration = glm::linearRand(mMinDuration, mMaxDuration);
	auto direction = glm::linearRand(mMinDirection, mMaxDirection);

	particle->runAction(Actions::Collection::Sequence(
		Actions::Collection::MakeParallel(
			Actions::Collection::ChangePosition(particle, particle->getPosition() + (direction * mDistance), duration, Easing::CubicOut),
			Actions::Collection::ChangeScale(particle, mEndScale, duration),
			Actions::Collection::ChangeColor(colored_particle, mEndColor, duration),
			Actions::Collection::ChangeAlpha(colored_particle, mEndColor.a, duration)
		),
		Actions::Collection::Kill(particle)
	));

	holder->attach(particle);
}
