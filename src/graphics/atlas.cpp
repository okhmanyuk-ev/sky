#include "atlas.h"
#include <rectpack2D/finders_interface.h>
#include <nlohmann/json.hpp>
#include <stb_image_write.h>

using namespace Graphics;

struct SaveImageContext
{
	std::string path;
	Platform::Asset::Path pathType;
};

void Atlas::SaveToFile(const std::string& path, const Image& image, const Regions& regions, Platform::Asset::Path pathType)
{
	auto writeFunc = [](void* context, void* memory, int size) {
		auto saveImageContext = static_cast<SaveImageContext*>(context);
		Platform::Asset::Write(saveImageContext->path + ".png", memory, size, saveImageContext->pathType);
	};

	SaveImageContext context;
	context.path = path;
	context.pathType = pathType;

	stbi_write_png_to_func(writeFunc, &context, image.getWidth(), image.getHeight(),
		image.getChannels(), image.getMemory(), image.getWidth() * image.getChannels());

	auto json = nlohmann::json();
	for (const auto& [name, region] : regions)
	{
		json[name] = { (int)region.pos.x, (int)region.pos.y, (int)region.size.x, (int)region.size.y };
	}
	auto json_dump = json.dump(4);

	Platform::Asset::Write(path + "_atlas.json", json_dump.data(), json_dump.size(), pathType);
}

Atlas Atlas::OpenFromFile(const std::string& image_path, const std::string& atlas_path, Platform::Asset::Path path_type)
{
	auto image_file = Platform::Asset(image_path, path_type);
	auto json_file = Platform::Asset(atlas_path, path_type);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto regions = Regions();
	for (const auto& [key, value] : json.items())
	{
		regions.insert({ key, { { value[0], value[1] }, { value[2], value[3] } } });
	}
	return Atlas(image_file, regions);
}

std::tuple<Image, Atlas::Regions> Atlas::MakeFromImages(const Images& images)
{
	using namespace rectpack2D;
	constexpr bool allow_flip = false;
	const auto runtime_flipping_mode = flipping_option::DISABLED;
	using spaces_type = empty_spaces<allow_flip, default_empty_spaces>;
	using rect_type = output_rect_t<spaces_type>;

	std::vector<rect_type> rectangles;

	for (const auto& [name, image] : images)
	{
		rectangles.emplace_back(rect_xywh(0, 0, image.getWidth(), image.getHeight()));
	}

	auto report_successful = [](rect_type&) {
		return callback_result::CONTINUE_PACKING;
	};

	auto report_unsuccessful = [](rect_type&) {
		return callback_result::ABORT_PACKING;
	};

	const auto max_side = 1 << 12;
	const auto discard_step = 1;

	auto my_custom_order_1 = [](const auto a, const auto b) {
		return a->get_wh().pathological_mult() > b->get_wh().pathological_mult();
	};

	auto my_custom_order_2 = [](const auto a, const auto b) {
		return a->get_wh().pathological_mult() < b->get_wh().pathological_mult();
	};

	const auto result_size = find_best_packing<spaces_type>(rectangles,
		make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode), my_custom_order_1, my_custom_order_2);

	const auto dst_width = result_size.w;
	const auto dst_height = result_size.h;
	const int channels = 4;
	
	auto result_image = Image(dst_width, dst_height, channels);
	auto result_regions = Regions();

	for (int i = 0; i < (int)images.size(); i++)
	{
		const auto& rect = rectangles.at(i);
		const auto& image = std::next(images.begin(), i)->second;
		const auto& name = std::next(images.begin(), i)->first;

		auto& region = result_regions[name];
		region.pos = { static_cast<float>(rect.x), static_cast<float>(rect.y) };
		region.size = { static_cast<float>(rect.w), static_cast<float>(rect.h) };

		auto src_width = image.getWidth();
		auto src_height = image.getHeight();

		auto src_mem = image.getMemory();
		auto dst_mem = result_image.getMemory();

		for (int y = 0; y < src_height; y++)
		{
			for (int x = 0; x < src_width; x++)
			{
				auto src = (uint8_t*)((size_t)src_mem + (y * src_width * channels) + (x * channels));
				auto dst = (uint8_t*)((size_t)dst_mem + (rect.y * dst_width * channels) + (y * dst_width * channels) + (rect.x * channels) + (x * channels));
				memcpy(dst, src, channels);
			}
		}
	}

	return { result_image, result_regions };
}

Atlas::Atlas(const Graphics::Image& image, const Regions& tex_regions)
{
	mTexture = std::make_shared<Renderer::Texture>(image.getWidth(), image.getHeight(), image.getChannels(), image.getMemory());
	mRegions = tex_regions;
}