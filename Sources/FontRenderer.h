#pragma once

#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Graphics/VertexStructure.h>
#include <Kore/Graphics/Kravur.h>
#include <Kore/Graphics/Graphics2.h>

/** Simple font renderer using Kravur */
class FontRenderer {
public:

	FontRenderer(const char* filename, float width, float height);
	~FontRenderer();

	// The screen space position of the upper left corner of the text
	Kore::vec2 screenSpacePosition;

	static void Init();

	void SetText(const char* inText);

	void SetColor(Kore::Color& inColor);

	void Render();

private:

	const char* text;

	Kore::Kravur* kravur;

	Kore::uint color;

	static Kore::Shader* vertexShader;
	static Kore::Shader* fragmentShader;
	static Kore::VertexStructure structure;
	static Kore::Program* program;
	static Kore::ConstantLocation mvpLocation;
	static Kore::ConstantLocation colorLocation;

	static Kore::VertexBuffer* bgVertexBuffer;
	static Kore::IndexBuffer* bgIndexBuffer;

	Kore::TextShaderPainter* textShaderPainter;

};