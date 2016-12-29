#pragma once

#include "pch.h"

class DynamicSound {
public:

	Kore::Sound sound;
	Kore::u8* original;

	DynamicSound(const char* filename) : sound(filename) {
		original = new Kore::u8[sound.size];
		for (int i = 0; i < sound.size; ++i) {
			original[i] = sound.data[i];
		}
	}

	void play(Kore::vec3 listener, Kore::vec3 position) {
		/************************************************************************/
		/* Task P13.3: Implement the missing code in the function to create positional sounds */
		/************************************************************************/

		// Determine the distance from listener to position

		// Set these values so they reflect the direction to the sound source
		// For directly right, rightVolume = 1.0, leftVolume = 0.0
		// For directly left, leftVolume = 1.0, rightVolume = 0.0
		// For very close to the listener's horizontal position, leftVolume = rightVolume = 0.5
		// Update: The comment originally said to choose left, right = 0.5 at the largest angles,
		// this was not the original intention. If you have already finished the exercised and
		// used the original 0.5, this will also be counted as correct.
		float rightVolume, leftVolume;
		Kore::vec3 lisToPos = position - listener;
		float distance = lisToPos.getLength();

		if (distance > 0.0001f) {
			// set higher relative volume where the sound comes from (and proportionally lower on the other side)
			lisToPos.normalize();
			float cos = lisToPos.x();
			rightVolume = (cos + 1.0f) / 2.0f;
			leftVolume = 1 - rightVolume;
		}
		else {
			// if the sound is where the listener is, he hears it evenly from both sides
			rightVolume = 0.5f;
			leftVolume = 0.5f;
		}


		Kore::Mixer::stop(&sound);

		// Modify sound data
		// The arrays contain interleaved stereo data in signed 16 bit integer values
		// Example - only plays on the right channel with half amplitude
		// Modify this code to use the values you computed above
		Kore::s16* source = (Kore::s16*)original;
		Kore::s16* destinationLeft = (Kore::s16*)sound.left;
		Kore::s16* destinationRight = (Kore::s16*)sound.right;
		for (int i = 0; i < sound.size / 2; ++i) {
			if (i % 2 == 0) { // test for left channel
				destinationLeft[i / 2] = static_cast<Kore::s16>(source[i] * leftVolume / exp(distance));
			}
			else {
				destinationRight[i / 2] = static_cast<Kore::s16>(source[i] * rightVolume / exp(distance));
			}
		}

		Kore::Mixer::play(&sound);
	}
};
