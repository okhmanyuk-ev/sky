#pragma once
#include "atlas.h"
#include <vector>

namespace Graphics
{
	class Animation
	{
	public:
		using Frames = std::vector<std::string>;
		using StatesMap = std::map<std::string, Frames>;

	public:
		Animation(const Atlas& atlas, const StatesMap& states);

	public:
		const auto& getAtlas() const { return mAtlas; }
		const auto& getStates() const { return mStates; }

	private:
		Atlas mAtlas;
		StatesMap mStates;
	};
}