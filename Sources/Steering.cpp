#include "Steering.h"


void AICharacter::integrate(const SteeringOutput& steer,
	float drag, float duration) {


	Position[0] += Velocity[0] * duration;
	Position[1] += Velocity[1] * duration;
	Orientation += Rotation * duration;
	//Orientation = fmodf(Orientation, Kore::pi * 2.0f);

	// Slowing velocity and rotational velocity
	drag = Kore::pow(drag, duration);
	Velocity *= drag;
	Rotation *= drag*drag;

	Velocity[0] += steer.linear.x()*duration;
	Velocity[1] += steer.linear.y()*duration;
	Rotation += steer.angular*duration;
}