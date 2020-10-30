#include "atlas.h"
#include <rectpack2D/finders_interface.h>
#include <nlohmann/json.hpp>

using namespace Graphics;

void Atlas::SaveToFile(const std::string& path, const Regions& regions, Platform::Asset::Path pathType)
{
	auto json = nlohmann::json();
	for (const auto& [name, region] : regions)
	{
		json[name] = { (int)region.pos.x, (int)region.pos.y, (int)region.size.x, (int)region.size.y };
	}
	auto json_dump = json.dump(4);

	Platform::Asset::Write(path + ".json", json_dump.data(), json_dump.size(), pathType);
}

std::tuple<Image, Atlas::Regions> Atlas::MakeFromImages(const Images& _images, bool anti_bleeding)
{
	Images images;

	if (anti_bleeding)
	{
		for (const auto& [name, _img] : _images)
		{
			images.insert({ name, MakeAntibleedImage(_img) });
		}
	}
	else
	{
		images = _images;
	}

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

	const auto finder_input = make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode);
	const auto result_size = find_best_packing<spaces_type>(rectangles, finder_input);

	const auto dst_width = result_size.w;
	const auto dst_height = result_size.h;
	const int channels = 4;
	
	auto dst_image = Image(dst_width, dst_height, channels);
	auto dst_regions = Regions();

	for (int i = 0; i < (int)images.size(); i++)
	{
		const auto& rect = rectangles.at(i);
		const auto& src_image = std::next(images.begin(), i)->second;
		const auto& name = std::next(images.begin(), i)->first;

		auto& region = dst_regions[name];
		region.pos = { static_cast<float>(rect.x), static_cast<float>(rect.y) };
		region.size = { static_cast<float>(rect.w), static_cast<float>(rect.h) };

		if (anti_bleeding)
		{
			region.pos.x += 1;
			region.pos.y += 1;

			region.size.x -= 1;
			region.size.y -= 1;
		}

		for (int x = 0; x < src_image.getWidth(); x++)
		{
			 for (int y = 0; y < src_image.getHeight(); y++)
			{
				auto src_pixel = src_image.getPixel(x, y);
				auto dst_pixel = dst_image.getPixel(rect.x + x, rect.y + y);
				memcpy(dst_pixel, src_pixel, channels);
			}
		}
	}

	return { dst_image, dst_regions };
}

Image Atlas::MakeAntibleedImage(const Image& image)
{
	auto w = image.getWidth() + 2;
	auto h = image.getHeight() + 2;
	auto channels = image.getChannels();

	auto result = Image(w, h, channels);

	auto src_mem = image.getMemory();

	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			auto pixel = result.getPixel(x, y);
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
			pixel[3] = 0;
		}
	}

	for (int x = 1; x < w - 1; x++)
	{
		for (int y = 1; y < h - 1; y++)
		{
			auto src_pixel = image.getPixel(x - 1, y - 1);
			auto dst_pixel = result.getPixel(x, y);
			memcpy(dst_pixel, src_pixel, channels);
		}
	}

	// vertical

	for (int y = 1; y < h - 1; y++)
	{
		{
			auto src_pixel = image.getPixel(0, y - 1);
			auto dst_pixel = result.getPixel(0, y);
			memcpy(dst_pixel, src_pixel, channels);
		}
		{
			auto src_pixel = image.getPixel(image.getWidth() - 1, y - 1);
			auto dst_pixel = result.getPixel(result.getWidth() - 1, y);
			memcpy(dst_pixel, src_pixel, channels);
		}
	}

	// horizontal

	for (int x = 1; x < w - 1; x++)
	{
		{
			auto src_pixel = image.getPixel(x - 1, 0);
			auto dst_pixel = result.getPixel(x, 0);
			memcpy(dst_pixel, src_pixel, channels);
		}
		{
			auto src_pixel = image.getPixel(x - 1, image.getHeight() - 1);
			auto dst_pixel = result.getPixel(x, result.getHeight() - 1);
			memcpy(dst_pixel, src_pixel, channels);
		}
	}

	return result;
}

Atlas::Atlas(const Regions& regions) :
	mRegions(regions)
{
	//
}

Atlas::Atlas(const Platform::Asset& regions_file) : 
	Atlas(ParseRegionsFromFile(regions_file))
{
	//
}

Atlas::Regions Atlas::ParseRegionsFromFile(const Platform::Asset& file)
{
	auto json_string = std::string((char*)file.getMemory(), file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto regions = Regions();
	for (const auto& [key, value] : json.items())
	{
		regions.insert({ key, { { value[0], value[1] }, { value[2], value[3] } } });
	}
	return regions;
}