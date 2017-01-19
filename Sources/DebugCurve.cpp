#include "DebugCurve.h"
#include "Kore/IO/FileReader.h"
#include "Kore/Graphics/Shader.h"

Kore::Shader* DebugCurve::vertexShader;
Kore::Shader* DebugCurve::fragmentShader;
Kore::VertexStructure DebugCurve::structure;
Kore::Program* DebugCurve::program;
Kore::ConstantLocation DebugCurve::mvpLocation;
Kore::ConstantLocation DebugCurve::colorLocation;

Kore::VertexBuffer* DebugCurve::bgVertexBuffer;
Kore::IndexBuffer* DebugCurve::bgIndexBuffer;

void DebugCurve::UpdateBuffers()
{
	// For each pair of values in the buffer, we draw two triangles
	int numVertices = numPairs * 3 * 2;

	if (numVertices == 0)
	{
		return;
	}

	delete vertexBuffer;
	vertexBuffer = new Kore::VertexBuffer(numVertices, structure, 0);
	float* vertices = vertexBuffer->lock();

	delete indexBuffer;
	indexBuffer = new Kore::IndexBuffer(numVertices);
	int* indices = indexBuffer->lock();
	int currentIndex = 0;

	// TODO: Borders of the array
	for (int i = 0; i < numPairs -1 ; i++)
	{
		AddVertices(timeValuePairs[i], timeValuePairs[i + 1], vertices, indices, currentIndex);
	}

	vertexBuffer->unlock();
	indexBuffer->unlock();
}

float DebugCurve::MapTime(float time) const
{
	// The latest measurement is at the very right
	float result = (time - lastTime) / visibleDuration + 1.0f;
	result = Kore::max(result, 0.0f);
	return result;
}

float DebugCurve::MapValue(float value) const
{
	// Clamp to range
	float result = Kore::max(minValueSeen, Kore::min(maxValueSeen, value));
	result = (result - minValueSeen) / (maxValueSeen - minValueSeen);
	return result;
}

void DebugCurve::AddVertices(const TimeValuePair& left, const TimeValuePair& right, float*& vertices, int*& indices, int& currentIndex)
{
	// Need two x-values corresponding to the time
	float xStart = MapTime(left.time);
	float xEnd = MapTime(right.time);

	// Need two y-values corresponding to the values
	float yStart = MapValue(left.value);
	float yEnd = MapValue(right.value);

	// Left triangle
	WriteVertex(vertices, indices, Kore::vec3(xStart, yStart, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(xEnd, yEnd, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(xStart, 0.0f, 0.0f), currentIndex);

	// Right triangle
	WriteVertex(vertices, indices, Kore::vec3(xStart, 0.0f, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(xEnd, yEnd, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(xEnd, 0.0f, 0.0f), currentIndex);
}

void DebugCurve::WriteVertex(float*& vertices, int*& indices, const Kore::vec3& v, int& currentIndex)
{
	vertices[0] = v[0];
	vertices[1] = v[1];
	vertices[2] = v[2];
	indices[0] = currentIndex;
	vertices = &vertices[3];
	indices = &indices[1];
	currentIndex++;
}

void DebugCurve::CleanupPairs()
{
	TimeValuePair* oldArray = timeValuePairs;
	timeValuePairs = new TimeValuePair[maxNumPairs];
	float cutOffTime = lastTime - visibleDuration;
	int currentIndex = 0;
	for (int i = 0; i < numPairs; i++)
	{
		TimeValuePair& currentPair = oldArray[i];
		if (currentPair.time >= cutOffTime)
		{
			timeValuePairs[currentIndex++] = currentPair;
		}
	}
	numPairs = currentIndex;
	delete[] oldArray;
}

DebugCurve::DebugCurve() 
	: screenSpacePosition(Kore::vec2(0.0f, 0.0f)), width(100.0f), height(100.0f), minValue(0.0f), maxValue(1.0f),
	visibleDuration(5.0f), numPairs(0), maxNumPairs(100), vertexBuffer(nullptr), indexBuffer(nullptr), buffersNeedUpdate(false)
{
	timeValuePairs = new TimeValuePair[maxNumPairs];
	UpdateBuffers();
	bgColor = Kore::vec4(1.0f, 0.0, 0.0f, 0.2f);
	fgColor = Kore::vec4(0.0f, 1.0f, 0.0f, 0.2f);
	minValueSeen = minValue;
	maxValueSeen = maxValue;
	autoAdjust = true;
}

void DebugCurve::AddValue(float time, float value)
{
	TimeValuePair pair;
	pair.time = time;
	pair.value = value;
	timeValuePairs[numPairs++] = pair;

	if (numPairs == maxNumPairs)
	{
		CleanupPairs();
	}

	if (autoAdjust)
	{
		minValueSeen = Kore::min(minValueSeen, pair.value);
		maxValueSeen = Kore::max(maxValueSeen, pair.value);
	}

	lastTime = time;
	
	buffersNeedUpdate = true;
}

void DebugCurve::SetPositionAndSize(Kore::vec2 position, float w, float h)
{
	screenSpacePosition = position;
	width = w;
	height = h;
}

void DebugCurve::Render(int windowWidth, int windowHeight)
{
	if (buffersNeedUpdate)
	{
		// Update the buffers
		UpdateBuffers();
		buffersNeedUpdate = false;
	}

	Kore::mat4 m = Kore::mat4::orthogonalProjection(-windowWidth * 0.5f, windowWidth * 0.5f, -windowHeight * 0.5f, windowHeight * 0.5f, -100.0f, 100.0f);
	// Now, we are in the center of the screen with width and height of 1
	m = m * Kore::mat4::Translation(screenSpacePosition.x() - windowWidth * 0.5f, -screenSpacePosition.y() + windowHeight * 0.5f - height, 0.0f);

	m = m * Kore::mat4::Scale(width, height, 1.0f);
	// Now, we are at the center of the screen with correct width and height

	// Draw the background
	Kore::Graphics::setRenderState(Kore::RenderState::DepthTest, false);
	program->set();
	Kore::Graphics::setMatrix(mvpLocation, m);

	Kore::Graphics::setFloat4(colorLocation, bgColor);
	Kore::Graphics::setVertexBuffer(*bgVertexBuffer);
	Kore::Graphics::setIndexBuffer(*bgIndexBuffer);
	Kore::Graphics::drawIndexedVertices();

	// We have too little data
	if (numPairs < 2)
	{
		return;
	}

	Kore::Graphics::setBlendingMode(Kore::BlendingOperation::SourceAlpha, Kore::BlendingOperation::InverseSourceAlpha);
	Kore::Graphics::setFloat4(colorLocation, fgColor);
	Kore::Graphics::setVertexBuffer(*vertexBuffer);
	Kore::Graphics::setIndexBuffer(*indexBuffer);
	Kore::Graphics::drawIndexedVertices();
}

void DebugCurve::Init()
{
	Kore::FileReader vs("debugCurve.vert");
	Kore::FileReader fs("debugCurve.frag");
	vertexShader = new Kore::Shader(vs.readAll(), vs.size(), Kore::ShaderType::VertexShader);
	fragmentShader = new Kore::Shader(fs.readAll(), fs.size(), Kore::ShaderType::FragmentShader);

	// This defines the structure of your Vertex Buffer
	structure.add("pos", Kore::VertexData::Float3VertexData);

	program = new Kore::Program;
	program->setVertexShader(vertexShader);
	program->setFragmentShader(fragmentShader);
	program->link(structure);

	mvpLocation = program->getConstantLocation("MVP");
	colorLocation = program->getConstantLocation("color");

	bgVertexBuffer = new Kore::VertexBuffer(6, structure, 0);
	float* vertices = bgVertexBuffer->lock();

	bgIndexBuffer = new Kore::IndexBuffer(6);
	int* indices = bgIndexBuffer->lock();
	int currentIndex = 0;

	WriteVertex(vertices, indices, Kore::vec3(0.0f, 0.0f, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(0.0f, 1.0f, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(1.0f, 1.0f, 0.0f), currentIndex);

	WriteVertex(vertices, indices, Kore::vec3(0.0f, 0.0f, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(1.0f, 1.0f, 0.0f), currentIndex);
	WriteVertex(vertices, indices, Kore::vec3(1.0f, 0.0f, 0.0f), currentIndex);

	bgVertexBuffer->unlock();
	bgIndexBuffer->unlock();
}
