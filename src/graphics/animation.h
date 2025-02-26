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
			sky::Asset::Storage storage = sky::Asset::Storage::Assets);

	public:
		Animation(const States& states);
		Animation(const sky::Asset& states_file);

	private:
		static States ParseStatesFromFile(const sky::Asset& file);

	public:
		const auto& getStates() const { return mStates; }

	private:
		States mStates;
	};
}