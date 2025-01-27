#pragma once
#include "atlas.h"
#include <vector>

namespace Graphics
{
	class Animation
	{
	public:
		using Frames = std::vector<std::string>;
		using States = std::map<std::string, Frames>;

	public:
		static void SaveToFile(const std::string& path, const States& states,
			Platform::Asset::Storage storage = Platform::Asset::Storage::Assets);

	public:
		Animation(const States& states);
		Animation(const Platform::Asset& states_file);

	private:
		static States ParseStatesFromFile(const Platform::Asset& file);

	public:
		const auto& getStates() const { return mStates; }

	private:
		States mStates;
	};
}