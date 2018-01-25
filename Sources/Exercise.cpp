#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Math/Random.h>
#include <Kore/Log.h>

#include "Memory.h"
#include "MeshObject.h"
#include "Steering.h"
#include "Flocking.h"
#include "StateMachine.h"

namespace {
	using namespace Kore;
	
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
	
	// Time
	double startTime;
	double lastTime;
	
	// The number of boids in the simulation. If using more, make objects[] larger
	const int numBoids = 20;
	
	// Array holding the boids
	AICharacter* boids[numBoids];
	
	// AI Characters for the moon and the Earth
	AICharacter* moon;
	AICharacter* earth;
	
	// Management for the whole flock
	Flock flock;
	
	// Flock steering component, blends the three involved delegated steering behaviours
	BlendedSteering* flockSteering;
	
	// The three steering components that flockSteering blends together
	Separation* separation;
	Cohesion* cohesion;
	VelocityMatchAndAlign* vMA;
	
	// Vector to hold steering information from the keyboard
	vec2 deltaPosition;
	
	// The two behaviours for the moon AI - wander and seek
	Wander* wander;
	Seek* seek;
	
	// The current steering behaviour for the moon
	SteeringBehaviour* moonBehaviour;
	
	// State machine for the moon's behaviour
	StateMachine moonStateMachine;
	
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	
	// null terminated array of MeshObject pointers
	MeshObject* objects[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	
	// The view projection matrix aka the camera
	mat4 P;
	mat4 View;
	
	// uniform locations - add more as you see fit
	Graphics4::TextureUnit tex;
	Graphics4::ConstantLocation pLocation;
	Graphics4::ConstantLocation vLocation;
	Graphics4::ConstantLocation mLocation;
	
	// State for managing actions in the state machine
	enum State { Wandering, Following };
	
	
	// An action that can change the behaviour of the moon AI
	class MoonAction : public Action {
	public:
		State state;
		
		virtual void act() {
			if (state == Following) {
				moonBehaviour = seek;
				Kore::log(Kore::Info, "Moon is now seeking");
			}
			else {
				moonBehaviour = wander;
				Kore::log(Kore::Info, "Moon is now wandering");
			}
		}
	};
	
	// A state in the moon's state machine. Returns the appropriate MoonAction to switch the state as the entry action
	class MoonState : public StateMachineState
	{
	public:
		
		State state;
		
		virtual Action* getEntryActions() {
			MoonAction* changeStatusAction = new MoonAction();
			changeStatusAction->state = state;
			changeStatusAction->next = nullptr;
			return changeStatusAction;
		}
		
		virtual Action* getExitActions() {
			return new Action();
		}
	};
	
	
	// A transition for the Moon's state machine.
	// Is realized as a fixed target transition with a condition
	class MoonTransition :
	public Transition,
	public ConditionalTransitionMixin,
	public FixedTargetTransitionMixin
	{
	public:
		
		virtual Action* getActions() {
			return new Action();
		}
		
		virtual bool isTriggered() {
			bool result = ConditionalTransitionMixin::isTriggered();
			return result;
		}
		
		StateMachineState* getTargetState()
		{
			return FixedTargetTransitionMixin::getTargetState();
		}
	};
	
	/************************************************************************/
	// Task 1.3: Complete this class so that it correctly returns
	// Checks if the moon is closer or further away from the specified distance
	/************************************************************************/
	class MoonCondition : public Condition {
	public:
		
		/**
		 * Performs the test for this condition.
		 */
		virtual bool test() {
			
			return false;
		}
	};
	
	// Update the AI
	void updateAI(float deltaT) {
		// Update the state machine
		// Get the actions that should be executed
		Action* actions = moonStateMachine.update();
		
		// Execute any actions that should be executed
		while (actions != nullptr) {
			actions->act();
			actions = actions->next;
		}
		
		
		// Update the steering behaviours
		float duration = deltaT;
		
		// One steering output is re-used
		SteeringOutput steer;
		
		// Handle the moon
		moonBehaviour->getSteering(&steer);
		moon->integrate(steer, 0.95f, duration);
		moon->trimMaxSpeed(1.0f);
		
		// Keep in bounds of the world
		TrimWorld(moon->Position[0]);
		TrimWorld(moon->Position[1]);
		
		moon->meshObject->M = mat4::Translation(moon->Position[0], 0.0f, moon->Position[1]);
		
		// Handle the earth - treat player input as a steering output
		steer.clear();
		steer.linear = deltaPosition;
		steer.linear *= 10.0f;
		earth->integrate(steer, 0.95f, duration);
		
		earth->trimMaxSpeed(3.0f);
		
		// Keep in bounds of the world
		TrimWorld(earth->Position[0]);
		TrimWorld(earth->Position[1]);
		
		earth->meshObject->M = mat4::Translation(earth->Position[0], 0.0f, earth->Position[1]) * mat4::Scale(2.0f, 2.0f, 2.0f);
		
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
	}
	
	void update() {
		double t = System::time() - startTime;
		double deltaT = t - lastTime;
		lastTime = t;
		
		// Update the AI
		updateAI((float)deltaT);
		
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, 0xff9999FF, 1000.0f);
		
		Graphics4::setPipeline(pipeline);
		
		// set the camera - orthogonal projection
		float val = worldSize;
		P = mat4::orthogonalProjection(-val, val, -val, val, -val, val);
		View = mat4::RotationX(Kore::pi / 2.0f * 3.0f);
		Graphics4::setMatrix(pLocation, P);
		Graphics4::setMatrix(vLocation, View);
		
		
		// iterate the MeshObjects
		MeshObject** current = &objects[0];
		while (*current != nullptr) {
			// set the model matrix
			Graphics4::setMatrix(mLocation, (*current)->M);
			
			(*current)->render(tex);
			++current;
		}
		
		Graphics4::end();
		Graphics4::swapBuffers();
	}
	
	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		
	}
	
	void mousePress(int windowId, int button, int x, int y) {
		
	}
	
	void mouseRelease(int windowId, int button, int x, int y) {
		
	}
	
	void keyDown(KeyCode code) {
		
		float movementDelta = 0.1f;
		
		if (code == KeyLeft) {
			deltaPosition[0] = -movementDelta;
		}
		else if (code == KeyRight) {
			deltaPosition[0] = movementDelta;
		}
		else if (code == KeyUp) {
			deltaPosition[1] = movementDelta;
		}
		else if (code == KeyDown) {
			deltaPosition[1] = -movementDelta;
		}
	}
	
	void keyUp(KeyCode code) {
		
		
		if (code == KeyLeft) {
			deltaPosition[0] = 0.0f;
		}
		else if (code == KeyRight) {
			deltaPosition[0] = 0.0f;
		}
		else if (code == KeyUp) {
			deltaPosition[1] = 0.0f;
		}
		else if (code == KeyDown) {
			deltaPosition[1] = 0.0f;
		}
	}
	
	void initAI() {
		// Set up Earth and moon AI characters
		Kore::Random::init(42);
		
		moon = new AICharacter();
		moon->Position = vec2(1.0f, 1.0f);
		moon->meshObject = objects[2];
		
		earth = new AICharacter();
		earth->meshObject = objects[0];
		
		// Set up the moon's behaviours
		wander = new Wander();
		wander->character = moon;
		wander->maxAcceleration = 2.0f;
		wander->turnSpeed = 2.0f;
		wander->volatility = 20.0f;
		
		seek = new Seek();
		seek->character = moon;
		seek->maxAcceleration = 3.0f;
		seek->target = &(earth->Position);
		
		moonBehaviour = wander;
		
		// Set up the moon's state machine
		MoonState* wanderState = new MoonState();
		MoonState* followState = new MoonState();
		wanderState->state = Wandering;
		followState->state = Following;
		
		MoonTransition* WanderingToFollowing = new MoonTransition();
		WanderingToFollowing->target = followState;
		
		MoonTransition* FollowingToWandering = new MoonTransition();
		FollowingToWandering->target = wanderState;
		
		/************************************************************************/
		// Task 1.3: After you have completed the MoonCondition object, instantiate it here
		/************************************************************************/
		
		MoonCondition* ShouldFollow = new MoonCondition();
		// This condition should trigger if the moon is closer than 1 unit to the Earth
		
		MoonCondition* ShouldWander = new MoonCondition();
		// This condition should trigger if the moon is further away than 1 unit from the Earth
		
		
		
		
		WanderingToFollowing->condition = ShouldFollow;
		FollowingToWandering->condition = ShouldWander;
		
		wanderState->firstTransition = WanderingToFollowing;
		followState->firstTransition = FollowingToWandering;
		
		moonStateMachine.initialState = wanderState;
		moonStateMachine.currentState = wanderState;
		
		
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
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(separation, 0.1f));
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(cohesion, 1.0f));
		flockSteering->behaviours.push_back(BlendedSteering::BehaviourAndWeight(vMA, 2.0f));
	}
	
	
	void init() {
		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
		fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
		
		// This defines the structure of your Vertex Buffer
		Graphics4::VertexStructure structure;
		structure.add("pos", Graphics4::Float3VertexData);
		structure.add("tex", Graphics4::Float2VertexData);
		structure.add("nor", Graphics4::Float3VertexData);
		
		pipeline = new Graphics4::PipelineState;
		pipeline->inputLayout[0] = &structure;
		pipeline->inputLayout[1] = nullptr;
		pipeline->vertexShader = vertexShader;
		pipeline->fragmentShader = fragmentShader;
		pipeline->depthMode = Graphics4::ZCompareLess;
		pipeline->depthWrite = true;
		pipeline->compile();
		
		tex = pipeline->getTextureUnit("tex");
		pLocation = pipeline->getConstantLocation("P");
		vLocation = pipeline->getConstantLocation("V");
		mLocation = pipeline->getConstantLocation("M");
		
		// Object 0 is the Earth
		objects[0] = new MeshObject("Level/ball.obj", "Level/unshaded.png", structure);
		
		// Object 1 is the background
		objects[1] = new MeshObject("Level/plane.obj", "Level/StarMap.png", structure);
		objects[1]->M = mat4::Translation(0.0f, 0.5f, 0.0f);
		
		// Object 2 is the Moon
		Graphics4::Texture* moonTexture = new Graphics4::Texture("Level/moonmap1k.jpg");
		objects[2] = new MeshObject(objects[0]->getVertexBuffer(), objects[0]->getIndexBuffer(), moonTexture);
		
		// Objects 3++ are the boids
		// Only load once and copy in order to speed up the loading time
		MeshObject* referenceBoid = new MeshObject("Level/boid.obj", "Level/basicTiles3x3red.png", structure);
		for (int i = 0; i < numBoids; i++) {
			objects[3 + i] = new MeshObject(referenceBoid->getVertexBuffer(), referenceBoid->getIndexBuffer(), referenceBoid->getTexture());
		}
		
		// Initialize the AI
		initAI();
		
		Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
		Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
	}
	
}

int kore(int argc, char** argv) {
	Kore::System::init("Solution 13", width, height);
	
	Memory::init();
	init();
	
	Kore::System::setCallback(update);
	
	startTime = System::time();
	lastTime = 0.0f;
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;
	
	Kore::System::start();
	
	return 0;
}

