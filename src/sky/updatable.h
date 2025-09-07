#pragma once

#include <memory>

namespace sky
{
	class Updatable
	{
	public:
		Updatable();
		~Updatable();

	public:
		virtual void onFrame() = 0;

	private:
		std::shared_ptr<bool> mFinished = std::make_shared<bool>(false);
	};
}