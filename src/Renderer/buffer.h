#pragma once

#include <vector>

namespace Renderer
{
	struct Buffer
	{
		Buffer() {}
		template<typename T> Buffer(T* memory, size_t count) : data((void*)memory), size(count * sizeof(T)), stride(sizeof(T)) {}
		template<typename T> Buffer(const std::vector<T>& values) : Buffer(values.data(), values.size()) {}

		void* data = nullptr;
		size_t size = 0;
		size_t stride = 0;
	};
}