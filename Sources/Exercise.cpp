#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Random.h>
#include <Kore/Log.h>

#include "MeshObject.h"
#include "Steering.h"
#include "Flocking.h"

#include "Reasoner.h"
#include "Option.h"
#include "Consideration.h"
#include "DistanceConsideration.h"
#include "Task.h"

#include "DebugCurve.h"
#include "FontRenderer.h"
#include "DynamicSound.h"
#include "LastExecutionConsideration.h"


// The world is viewed using an orthographic projection and represents toroidal space: Objects that leave on one side come out on the other side.
#define WORLD_SIZE 3.0f

#define TRIM_WORLD(var) \
if (var < -WORLD_SIZE) var = WORLD_SIZE; \
if (var > WORLD_SIZE) var = -WORLD_SIZE;

namespace {
	using namespace Kore;

	FontRenderer* fontRenderer;

	// Window size should be square otherwise we would need WORLD_SIZE_X and WORLD_SIZE_Z
	const int width = 512;
	const int height = 512;

	// Variables for the debug visualizers
	float debugUpdateFrequency = 0.5f;
	float nextDebugUpdate = debugUpdateFrequency;

	DebugCurve* debugCurve;

	// Variables for when the next sound will be played
	float playSoundFrequency = 0.5f;
	float nextPlaySound = playSoundFrequency;

	// The sound emitted by the moon object
	DynamicSound* moonSound;

	// Time in seconds since program start
	double time = 0.0;

	// The number of boids in the simulation. If using more, make objects[] larger
	const int numBoids = 20;

	// Array holding the boids
	AICharacter* boids[numBoids];

	/** The utility-based reasoner */
	Reasoner* reasoner;

	DistanceConsideration* farEnoughConsideration;
	LastExecutionConsideration* lastExecutionConsideration;

	// AI Characters for the moon and the Earth
	AICharacter* moon;
	AICharacter* earth;

	// Management for the whole flock
	Flock flock;


	// Flock steering component, blends the three involved delegated steering behaviours
	BlendedSteering* flockSteering;

	// The three steering components that flockSteering blends together
	Separation *separation;
	Cohesion *cohesion;
	VelocityMatchAndAlign *vMA;

	// Vector to hold steering information from the keyboard
	vec2 deltaPosition;

	// The two behaviours for the moon AI - wander and seek
	Wander* wander;
	Seek* seek;

	// The current steering behaviour for the moon
	SteeringBehaviour* moonBehaviour;

	double startTime;
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;

	double lastTime;

	// null terminated array of MeshObject pointers
	MeshObject* objects[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	// The view projection matrix aka the camera
	mat4 P;
	mat4 View;

	// uniform locations - add more as you see fit
	TextureUnit tex;
	ConstantLocation pLocation;
	ConstantLocation vLocation;
	ConstantLocation mLocation;

	// Update the AI
	void updateAI(float deltaT) {

		// Update the steering behaviours
		float duration = deltaT;

		// One steering output is re-used
		SteeringOutput steer;

		// Keep in bounds of the world
		TRIM_WORLD(moon->Position[0]);
		TRIM_WORLD(moon->Position[1]);

		// Handle the earth - treat player input as a steering output
		steer.clear();
		steer.linear = deltaPosition;
		steer.linear *= 10.0f;
		earth->integrate(steer, 0.95f, duration);

		earth->trimMaxSpeed(3.0f);

		// Keep in bounds of the world
		TRIM_WORLD(earth->Position[0]);
		TRIM_WORLD(earth->Position[1]);

		earth->meshObject->M = mat4::Translation(earth->Position[0], 0.0f, earth->Position[1]) * mat4::Scale(2.0f);

		// Handle the boids

		for (int i = 0; i < numBoids; i++) {
			AICharacter* boid = boids[i];

			// Get the steering output
			flockSteering->character = boid;
			flockSteering->getSteering(&steer);

			// Update the kinematic
			boid->integrate(steer, 0.7f, duration);

			// Face the direction we are moving
			boid->setOrientationFromVelocity();

			// Check for maximum speed
			boid->trimMaxSpeed(4.0f);

			// Keep in bounds of the world
			TRIM_WORLD(boid->Position[0]);
			TRIM_WORLD(boid->Position[1]);

			boid->meshObject->M = mat4::Translation(boid->Position[0], 0.0f, boid->Position[1]) * mat4::RotationY(boid->Orientation + Kore::pi);
		}

		// Update the reasoner
		reasoner->Update(deltaT);
		fontRenderer->SetText(reasoner->GetStateString());
	}

	void updateDebugCurves(float deltaTime) {
		nextDebugUpdate -= deltaTime;
		if (nextDebugUpdate < 0.0f)
		{
			nextDebugUpdate = debugUpdateFrequency;

			// Update the curves
			// debugCurve->AddValue((float)time, farEnoughConsideration->GetValue());
		}
	}

	void updateSound(float deltaTime)
	{
		nextPlaySound -= deltaTime;
		if (nextPlaySound < 0.0f)
		{
			nextPlaySound = playSoundFrequency;
			moonSound->play(earth->get3DPosition(), moon->get3DPosition());
		}
	}


	void update() {
		time = System::time() - startTime;
		double deltaT = time - lastTime;
		updateDebugCurves((float) deltaT);
		updateSound((float) deltaT);
		lastTime = time;

		// Update the AI
		updateAI((float)deltaT);

		Kore::Audio::update();

		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag, 0xff9999FF, 1000.0f);
		Graphics::setRenderState(DepthTest, true);
		Graphics::setRenderState(DepthTestCompare, ZCompareLess);

		program->set();

		// set the camera - orthogonal projection
		float val = WORLD_SIZE;
		P = mat4::orthogonalProjection(-val, val, -val, val, -val, val);
		View = mat4::RotationX(Kore::pi / 2.0f * 3.0f);
		Graphics::setMatrix(pLocation, P);
		Graphics::setMatrix(vLocation, View);


		// iterate the MeshObjects
		MeshObject** current = &objects[0];
		while (*current != nullptr) {
			// set the model matrix
			Graphics::setMatrix(mLocation, (*current)->M);

			(*current)->render(tex);
			++current;
		}

		// Draw the debug UI
		debugCurve->Render(width, height);

		// Draw the text
		fontRenderer->Render();

		Graphics::end();
		Graphics::swapBuffers();
	}

	void handleKey(KeyCode code, bool isDown)
	{
		float movementDelta = isDown ? 0.1f : 0.0f;
		if (code == Key_Left || code == Key_A) {
			deltaPosition[0] = -movementDelta;
		}
		else if (code == Key_Right || code == Key_D) {
			deltaPosition[0] = movementDelta;
		}
		else if (code == Key_Up || code == Key_W) {
			deltaPosition[1] = movementDelta;
		}
		else if (code == Key_Down || code == Key_S) {
			deltaPosition[1] = -movementDelta;
		}
	}

	void keyDown(KeyCode code, wchar_t character) {
		handleKey(code, true);
	}

	void keyUp(KeyCode code, wchar_t character) {
		handleKey(code, false);
	}


	void initAI() {
		// Set up Earth and moon AI characters
		Kore::Random::init(42);

		moon = new AICharacter();
		moon->Position = vec2(1.0f, 1.0f);
		moon->meshObject = objects[2];

		earth = new AICharacter();
		earth->meshObject = objects[0];

		// Set up the boids
		for (int i = 0; i < numBoids; i++) {
			boids[i] = new AICharacter();
			AICharacter* current = boids[i];
			current->meshObject = objects[3 + i];
			current->Position[0] = Wander::randomBinomial(WORLD_SIZE);
			current->Position[1] = Wander::randomBinomial(WORLD_SIZE);
			current->Orientation = Wander::randomReal(Kore::pi);
			current->Velocity[0] = Wander::randomBinomial(2.0f);
			current->Velocity[1] = Wander::randomReal(2.0f);
			current->Rotation = 0.0f;
			flock.boids.push_back(current);
		}


		float accel = 2.0f;
		// Set up the steering behaviours (we use one for all)
		separation = new Separation;
		separation->maxAcceleration = accel;
		separation->neighbourhoodSize = 1.0f;
		separation->neighbourhoodMinDP = -1.0f;
		separation->theFlock = &flock;

		cohesion = new Cohesion;
		cohesion->maxAcceleration = accel;
		cohesion->neighbourhoodSize = 1.0f;
		cohesion->neighbourhoodMinDP = 0.0f;
		cohesion->theFlock = &flock;

		vMA = new VelocityMatchAndAlign;
		vMA->maxAcceleration = accel;
		vMA->neighbourhoodSize = 2.0f;
		vMA->neighbourhoodMinDP = 0.0f;
		vMA->theFlock = &flock;


		flockSteering = new BlendedSteering();
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(
			separation, 0.1f
			));
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(
			cohesion, 1.0f
			));
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(
			vMA, 2.0f
			));

		// Set up the reasoner
		reasoner = new Reasoner(moon);

		float distance = 1.0f;

		// The moon has two options: Wander or Follow the Earth
		Option* wanderOption = new Option("Wander");
		// Wandering should happen if the moon is close enough to the earth
		DistanceConsideration* closeEnoughConsideration = new DistanceConsideration(earth);
		ConstCurve* constOneCurve = new ConstCurve();
		constOneCurve->ConstValue = 1.0f;
		BooleanCurve* closeEnoughCurve = new BooleanCurve();
		closeEnoughCurve->comparisonOperator = BooleanCurve::MoreThen;
		closeEnoughCurve->Threshold = distance;
		closeEnoughConsideration->SetCurves(constOneCurve, closeEnoughCurve);
		wanderOption->SetRootConsideration(closeEnoughConsideration);
		// Create the corresponding WanderTask for the wanderOption
		WanderTask* moonWanderTask = new WanderTask(moon);
		wanderOption->SetTask(moonWanderTask);
		
		reasoner->AddOption(wanderOption);

		Option* seekOption = new Option("Seek");

		// If the moon is close enough, it should start seeking

		
		// We use three basic considerations
		// First one: Drives weight to 1 if the moon is close enough
		DistanceConsideration* moonCloseToEarth = new DistanceConsideration(earth);
		moonCloseToEarth->SetOwner(moon);
		BooleanCurve* closerThanDistanceCurve = new BooleanCurve(BooleanCurve::LessThen, distance);
		moonCloseToEarth->SetCurves(constOneCurve, closerThanDistanceCurve);


		// Second one: An "is executing" one
		IsExecutingConsideration* isExecuting = new IsExecutingConsideration(seekOption);
		isExecuting->SetCurves(constOneCurve, new IdentityCurve());
		
		// Third one: A "time since last execution"
		lastExecutionConsideration = new LastExecutionConsideration(seekOption);
		ExponentialDecayCurve* lastExecutionCurve = new ExponentialDecayCurve(0.8f, 5.0f);
		lastExecutionConsideration->SetCurves(constOneCurve, lastExecutionCurve);

		// We combine the latter two so that the IsExecutingConsideration can pull the other curve to 0 to 
		// prevent the moon from switching back to following after a while
		CompositeConsideration* executionHistoryComposite = new CompositeConsideration(CompositeConsideration::CM_Max, CompositeConsideration::CM_Multiply);
		executionHistoryComposite->AddConsideration(isExecuting);
		executionHistoryComposite->AddConsideration(lastExecutionConsideration);

		// Then, we combine to get the overall curve
		CompositeConsideration* root = new CompositeConsideration(CompositeConsideration::CM_Max, CompositeConsideration::CM_Max);
		root->AddConsideration(executionHistoryComposite);
		root->AddConsideration(moonCloseToEarth);



		seekOption->SetRootConsideration(root);

		// Create the corresponding FollowTask for the seekOption
		FollowTask* moonFollowTask = new FollowTask(moon,earth);
		seekOption->SetTask(moonFollowTask);
		
		reasoner->AddOption(seekOption);

	}


	void init() {
		// Initialize the debug curve shaders etc.
		DebugCurve::Init();

		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);

		// This defines the structure of your Vertex Buffer
		VertexStructure structure;
		structure.add("pos", Float3VertexData);
		structure.add("tex", Float2VertexData);
		structure.add("nor", Float3VertexData);

		program = new Program;
		program->setVertexShader(vertexShader);
		program->setFragmentShader(fragmentShader);
		program->link(structure);

		tex = program->getTextureUnit("tex");
		pLocation = program->getConstantLocation("P");
		vLocation = program->getConstantLocation("V");
		mLocation = program->getConstantLocation("M");

		// Object 0 is the Earth
		objects[0] = new MeshObject("Level/ball.obj", "Level/unshaded.png", structure);

		// Object 1 is the background
		objects[1] = new MeshObject("Level/plane.obj", "Level/StarMap.png", structure);
		objects[1]->M = mat4::Translation(0.0f, 0.5f, 0.0f);

		// Object 2 is the Moon
		Kore::Texture* moonTexture = new Kore::Texture("Level/moonmap1k.jpg");
		objects[2] = new MeshObject(objects[0]->getVertexBuffer(), objects[0]->getIndexBuffer(), moonTexture);

		// Objects 3++ are the boids
		// Only load once and copy in order to speed up the loading time
		MeshObject* referenceBoid = new MeshObject("Level/boid.obj", "Level/basicTiles3x3red.png", structure);
		for (int i = 0; i < numBoids; i++) {
			objects[3 + i] = new MeshObject(referenceBoid->getVertexBuffer(), referenceBoid->getIndexBuffer(), referenceBoid->getTexture());
		}

		// Initialize the AI
		initAI();


		Graphics::setTextureAddressing(tex, Kore::U, Repeat);
		Graphics::setTextureAddressing(tex, Kore::V, Repeat);

		debugCurve = new DebugCurve();

		moonSound = new DynamicSound("untitled.wav");

		fontRenderer = new FontRenderer("Roboto", (float) width, (float) height);
		fontRenderer->SetText("Hello, world!");
		fontRenderer->SetColor(Kore::Color(Kore::Color::Red));
	}

}

	int kore(int argc, char** argv) {
		Kore::System::setName("TUD Game Technology - ");
		Kore::System::setup();
		Kore::WindowOptions options;
		options.title = "Solution 9";
		options.width = width;
		options.height = height;
		options.x = 100;
		options.y = 100;
		options.targetDisplay = -1;
		options.mode = WindowModeWindow;
		options.rendererOptions.depthBufferBits = 16;
		options.rendererOptions.stencilBufferBits = 8;
		options.rendererOptions.textureFormat = 0;
		options.rendererOptions.antialiasing = 0;
		Kore::System::initWindow(options);

		init();

		Kore::System::setCallback(update);

		Kore::Mixer::init();
		Kore::Audio::init();


		startTime = System::time();


		Keyboard::the()->KeyDown = keyDown;
		Keyboard::the()->KeyUp = keyUp;

		Kore::System::start();



		return 0;
	}

