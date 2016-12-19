#include "DebugCurve.h"

void DebugCurve::UpdateBuffers()
{
	// For each pair of values in the buffer, we draw two triangles
	int numVertices = numPairs * 3 * 2;

	vertexBuffer = new Kore::VertexBuffer(numVertices, structure, 0);
	float* vertices = vertexBuffer->lock();

	indexBuffer = new Kore::IndexBuffer(numVertices);
	int* indices = indexBuffer->lock();

	// TODO: Borders of the array
	for (int i = 0; i < numPairs; i++)
	{
		AddVertices(timeValuePairs[i], timeValuePairs[i + 1], vertices, indices, TODO);
	}

	vertexBuffer->unlock();
	indexBuffer->unlock();
}

void DebugCurve::AddVertices(const TimeValuePair& left, const TimeValuePair& right, float*& vertices, int*& indices, int& currentIndex)
{
	// Need two x-values corresponding to the time
	float xStart = left.time;
	float xEnd = right.time;

	// Need two y-values corresponding to the values
	// @@TODO: Need to scale them to the correct range
	float yStart = left.value;
	float yEnd = right.value;


}

void DebugCurve::WriteVertex(float*& vertices, int*& indices, Kore::vec3& v, int& currentIndex)
{
	vertices[0] = v[0];
	vertices[1] = v[1];
	vertices[2] = v[2];
	indices[0] = currentIndex;
	vertices = &vertices[3];
	indices = &indices[1];
}

DebugCurve::DebugCurve() : screenSpacePosition(Kore::vec2(0.0f, 0.0f)), width(100.0f), height(100.0f), minValue(0.0f), maxValue(1.0f), visibleDuration(5.0f)
{
}

void DebugCurve::AddValue(float time, float value)
{

}

void DebugCurve::Render()
{

}
