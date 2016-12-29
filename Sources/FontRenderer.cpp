#include "FontRenderer.h"

using namespace Kore;

FontRenderer::FontRenderer(const char* filename, float width, float height)
	: text(nullptr), screenSpacePosition(Kore::vec2(0.0f, 0.0f))
{
	kravur = Kravur::load(filename, FontStyle(), 16.0f);
	
	textShaderPainter = new TextShaderPainter();
	textShaderPainter->setFont(kravur);
	// Kore::mat4 m = Kore::mat4::orthogonalProjection(-width * 0.5f, width * 0.5f, height * 0.5f, -height * 0.5f, -100.0f, 100.0f);
	Kore::mat4 m = Kore::mat4::orthogonalProjection(0.0f, width, height, 0.0f, -100.0f, 100.0f);
	textShaderPainter->setProjection(m);
}

FontRenderer::~FontRenderer()
{
	delete[] text;
}

void FontRenderer::SetText(const char* inText)
{
	text = inText;
}

void FontRenderer::SetColor(Kore::Color& inColor)
{
	Kore::u8 red = (Kore::u8) inColor.R * 255.0f;
	Kore::u8 green = (Kore::u8) inColor.G * 255.0f;
	Kore::u8 blue = (Kore::u8) inColor.B * 255.0f;
	Kore::u8 alpha = (Kore::u8) inColor.A * 255.0f;
	color = (alpha << 24) | (red << 16) | (green < 8) | blue;
}

void FontRenderer::Render()
{
	int* fontGlyphs = nullptr;
	textShaderPainter->drawString(text, 1.0f, color, screenSpacePosition.x(), screenSpacePosition.y(), Kore::mat3::Identity(), fontGlyphs);
	textShaderPainter->end();
}
