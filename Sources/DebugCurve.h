#pragma once

#include "pch.h"
#include "Kore/Math/Vector.h"
#include "Kore/Graphics/Graphics.h"

struct TimeValuePair {
	
	float time;
	float value;

};

/** Draws a curve for debug purposes */
class DebugCurve 
{
	// The screen space position of the curve in pixels. TODO: Define origin of the coordinates
	Kore::vec2 screenSpacePosition;

	// Width and height of the curve
	float width;
	float height;

	// The values defining the y-axis
	float minValue;
	float maxValue;

	// The value defining the x-axis
	float visibleDuration;

	int maxNumPairs;
	int numPairs;
	TimeValuePair* timeValuePairs;

	Kore::VertexStructure structure;
	Kore::VertexBuffer* vertexBuffer;
	Kore::IndexBuffer* indexBuffer;

	// Updates the buffers after adding value
	void UpdateBuffers();

	// Adds the vertices and indices for the given pair of time/value pairs. Vertices and indices is expected to point to the first free places in the buffer 
	void AddVertices(const TimeValuePair& left, const TimeValuePair& right, float*& vertices, int*& indices, int& currentIndex);

	void WriteVertex(float*& vertices, int*& indices, Kore::vec3& v, int& currentIndex);

public:

	DebugCurve();

	// Plots one value in the graph
	void AddValue(float time, float value);

	void Render();

};