#include "vertex.h"

using namespace Renderer::Vertex;

const Layout Position::Layout = { sizeof(Position), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(Position, pos) } }
};

const Layout PositionColor::Layout = { sizeof(PositionColor), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionColor, pos) },
	{ Attribute::Type::Color, Attribute::Format::R32G32B32A32F, offsetof(PositionColor, col) } } 
};

const Layout PositionTexture::Layout = { sizeof(PositionTexture), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionTexture, pos) },
	{ Attribute::Type::TexCoord, Attribute::Format::R32G32F, offsetof(PositionTexture, tex) } }
};

const Layout PositionNormal::Layout = { sizeof(PositionNormal), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionNormal, pos) },
	{ Attribute::Type::Normal, Attribute::Format::R32G32B32F, offsetof(PositionNormal, normal) } }
};

const Layout PositionColorNormal::Layout = { sizeof(PositionColorNormal), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionColorNormal, pos) },
	{ Attribute::Type::Color, Attribute::Format::R32G32B32A32F, offsetof(PositionColorNormal, col) },
	{ Attribute::Type::Normal, Attribute::Format::R32G32B32F, offsetof(PositionColorNormal, normal) } }
};

const Layout PositionColorTexture::Layout = { sizeof(PositionColorTexture), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionColorTexture, pos) },
	{ Attribute::Type::Color, Attribute::Format::R32G32B32A32F, offsetof(PositionColorTexture, col) },
	{ Attribute::Type::TexCoord, Attribute::Format::R32G32F, offsetof(PositionColorTexture, tex) } }
};

const Layout PositionTextureNormal::Layout = { sizeof(PositionTextureNormal), {
	{ Attribute::Type::Position, Attribute::Format::R32G32B32F, offsetof(PositionTextureNormal, pos) },
	{ Attribute::Type::TexCoord, Attribute::Format::R32G32F, offsetof(PositionTextureNormal, tex) },
	{ Attribute::Type::Normal, Attribute::Format::R32G32B32F, offsetof(PositionTextureNormal, normal) } }
};