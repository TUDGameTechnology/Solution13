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


namespace {
	using namespace Kore;

	FontRenderer* fontRenderer;

	// Window size should be square otherwise we would need WORLD_SIZE_X and WORLD_SIZE_Z
	const int width = 512;
	const int height = 512;

	// The world is viewed using an orthographic projection and represents toroidal space: Objects that leave on one side come out on the other side.
	const float worldSize = 3.0f;

	// Trims the input variable to be inside the world size
	void TrimWorld(float& x)
	{
		if (x < -worldSize) x = worldSize;
		if (x > worldSize) x = -worldSize;
	}

	// Variables for the debug visualizers
	float debugUpdateFrequency = 0.5f;
	float nextDebugUpdate = debugUpdateFrequency;

	DebugCurve* debugCurve;

	// The consideration which we will be visualizing
	Consideration* debuggedConsideration = nullptr;

	// Variables for when the next sound will be played
	float playSoundFrequency = 1.0f;
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

	double startTime = 0.0;
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;

	double lastTime = 0.0;

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
		TrimWorld(moon->Position[0]);
		TrimWorld(moon->Position[1]);

		// Handle the earth - treat player input as a steering output
		steer.clear();
		steer.linear = deltaPosition;
		steer.linear *= 10.0f;
		earth->integrate(steer, 0.95f, duration);

		earth->trimMaxSpeed(3.0f);

		// Keep in bounds of the world
		TrimWorld(earth->Position[0]);
		TrimWorld(earth->Position[1]);

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
			TrimWorld(boid->Position[0]);
			TrimWorld(boid->Position[1]);

			boid->meshObject->M = mat4::Translation(boid->Position[0], 0.0f, boid->Position[1]) * mat4::RotationY(boid->Orientation + Kore::pi);
		}

		// Update the reasoner
		reasoner->Update(deltaT);

		// Show the debug output for the reasoner
		fontRenderer->SetText(reasoner->GetStateString());
	}

	void updateDebugCurves(float deltaTime) {
		nextDebugUpdate -= deltaTime;
		if (nextDebugUpdate < 0.0f)
		{
			nextDebugUpdate = debugUpdateFrequency;

			// Update the debug curve
			if (debuggedConsideration)
			{
				debugCurve->AddValue((float)time, debuggedConsideration->GetWeight());
			}
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
		float val = worldSize;
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
			current->Position[0] = Wander::randomBinomial(worldSize);
			current->Position[1] = Wander::randomBinomial(worldSize);
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

		// The moon has three options: Wander, Seek and Continue Seek. The latter is added to force the moon to commit for a certain amount of time to the seek action
		// even if the distance to the earth is temporarily larger than the original threshold.
		Option* wanderOption = new Option("Wander");
		WanderTask* moonWanderTask = new WanderTask(moon);
		wanderOption->SetTask(moonWanderTask);
		reasoner->AddOption(wanderOption);

		Option* seekOption = new Option("Seek"); 
		FollowTask* moonFollowTask = new FollowTask(moon,earth);
		seekOption->SetTask(moonFollowTask);
		reasoner->AddOption(seekOption);

		Option* continueSeekOption = new Option("Continue Seek");
		continueSeekOption->SetTask(moonFollowTask);
		reasoner->AddOption(continueSeekOption);

		// Some curves we will need several times
		ConstCurve* constOneCurve = new ConstCurve(1.0f);
		IdentityCurve* identityCurve = new IdentityCurve();

		// Wandering should happen if the moon is far enough from the earth
		// This consideration has an output of 1 whenever this is the case, 0 otherwise.
		DistanceConsideration* farConsideration = new DistanceConsideration(earth);
		BooleanCurve* closeEnoughCurve = new BooleanCurve();
		closeEnoughCurve->comparisonOperator = BooleanCurve::MoreThen;
		closeEnoughCurve->Threshold = distance;
		farConsideration->SetCurves(constOneCurve, closeEnoughCurve);
		wanderOption->SetRootConsideration(farConsideration);

		// If the moon is close enough, it should start seeking
		DistanceConsideration* moonCloseToEarth = new DistanceConsideration(earth);
		moonCloseToEarth->SetOwner(moon);
		BooleanCurve* closerThanDistanceCurve = new BooleanCurve(BooleanCurve::LessThen, distance);
		moonCloseToEarth->SetCurves(constOneCurve, closerThanDistanceCurve);
		seekOption->SetRootConsideration(moonCloseToEarth);

		// Continue seek is a bit more complicated. First, we use the "opt in" pattern: The reasoner should always choose this state right after the seek option has stopped executing
		LastExecutionStoppedConsideration* optIn = new LastExecutionStoppedConsideration(seekOption);
		// We use this curve to force this option right after seek has ended
		ValueInRangeCurve* optInCurve = new ValueInRangeCurve(0.0f, 0.3f, 2.0f);
		optIn->SetCurves(optInCurve, optInCurve);

		// While "continue seek" is active, a decay curve should decrease the weight
		// Also, if "continue seek" is finally stopped, it should not be activated again.
		// We do this by creating a composite consideration which combines the "is currently executing" consideration with the decay curve. If the option is not executing anymore,
		// this pulls the weight down to 0
		LastExecutionStoppedConsideration* lastExecutionStoppedConsideration = new LastExecutionStoppedConsideration(seekOption);
		ExponentialDecayCurve* lastExecutionCurve = new ExponentialDecayCurve(0.8f, 50.0f);
		lastExecutionStoppedConsideration->SetCurves(constOneCurve, lastExecutionCurve);

		IsExecutingConsideration* isExecuting = new IsExecutingConsideration(continueSeekOption);
		isExecuting->SetCurves(constOneCurve, identityCurve);

		CompositeConsideration* optOutComposite = new CompositeConsideration(CompositeConsideration::CM_Max, CompositeConsideration::CM_Multiply);
		optOutComposite->AddConsideration(isExecuting);
		optOutComposite->AddConsideration(lastExecutionStoppedConsideration);

		CompositeConsideration* executionHistoryComposite = new CompositeConsideration(CompositeConsideration::CM_Max, CompositeConsideration::CM_Max);
		executionHistoryComposite->AddConsideration(optIn);
		executionHistoryComposite->AddConsideration(optOutComposite);

		continueSeekOption->SetRootConsideration(executionHistoryComposite);
	
		reasoner->SetOwner(moon);

		// We want to see the output of this complex consideration for continue seek in the debug view
		debuggedConsideration = executionHistoryComposite;
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
		debugCurve->SetPositionAndSize(Kore::vec2(0.0f, height - 100.0f), 100.0f, 100.0f);

		moonSound = new DynamicSound("untitled.wav");

		fontRenderer = new FontRenderer("Roboto", (float) width, (float) height);
		fontRenderer->SetColor(Kore::Color(Kore::Color::White));
	}

}

	int kore(int argc, char** argv) {
		Kore::System::setName("TUD Game Technology - ");
		Kore::System::setup();
		Kore::WindowOptions options;
		options.title = "Solution 13";
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

