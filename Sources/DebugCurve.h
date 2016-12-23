#pragma once

#include "pch.h"
#include "Kore/Math/Vector.h"
#include "Kore/Graphics/Graphics.h"
#include "Kore/Graphics/VertexStructure.h"

struct TimeValuePair {
	
	float time;
	float value;

};

/** Draws a curve for debug purposes */
class DebugCurve 
{
	static Kore::Shader* vertexShader;
	static Kore::Shader* fragmentShader;
	static Kore::VertexStructure structure;
	static Kore::Program* program;
	static Kore::ConstantLocation mvpLocation;
	static Kore::ConstantLocation colorLocation;

	static Kore::VertexBuffer* bgVertexBuffer;
	static Kore::IndexBuffer* bgIndexBuffer;

	// The screen space position of the upper left corner of the curve from the upper left of the window
	Kore::vec2 screenSpacePosition;

	Kore::vec4 bgColor;
	Kore::vec4 fgColor;

	// Width and height of the curve display
	float width;
	float height;

	// The values defining the y-axis
	float minValue;
	float maxValue;

	float minValueSeen;
	float maxValueSeen;

	// The value defining the x-axis
	float visibleDuration;

	int maxNumPairs;
	int numPairs;
	TimeValuePair* timeValuePairs;

	float lastTime;

	Kore::VertexBuffer* vertexBuffer;
	Kore::IndexBuffer* indexBuffer;

	// Keeps track if UpdateBuffers needs to be called. In the OpenGL backend, changing the buffers before Graphics::start will not work correctly
	bool buffersNeedUpdate;

	// Updates the buffers after adding value
	void UpdateBuffers();

	float MapTime(float time) const;
	float MapValue(float value) const;

	// Adds the vertices and indices for the given pair of time/value pairs. Vertices and indices is expected to point to the first free places in the buffer 
	void AddVertices(const TimeValuePair& left, const TimeValuePair& right, float*& vertices, int*& indices, int& currentIndex);

	static void WriteVertex(float*& vertices, int*& indices, Kore::vec3& v, int& currentIndex);

	// Should the y-axis be adjusted automatically?
	bool autoAdjust;

	// Called when no more pairs fit into the array to clean up
	void CleanupPairs();

public:

	DebugCurve();

	// Plots one value in the graph
	void AddValue(float time, float value);

	void Render(int width, int height);

	static void Init();

};