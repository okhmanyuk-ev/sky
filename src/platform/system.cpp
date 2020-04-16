#include "system.h"

using namespace Platform;

float System::getLogicalWidth() const 
{ 
	return getWidth() / getScale(); 
}
;
float System::getLogicalHeight() const 
{ 
	return getHeight() / getScale(); 
};

void System::rescale(float value)
{
	auto old_scale = getScale();
	auto scale_delta = value / old_scale;
	auto width = getWidth();
	auto height = getHeight();
	resize(int(width * scale_delta), int(height * scale_delta));
	setScale(value);
}