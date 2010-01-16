#ifndef RANDOM_H
#define RANDOM_H

struct Random_Manager {
	int Seed;

	int	  Int(int min, int max)			{ Seed=214013*Seed+2531011;	return min+(Seed ^ Seed>>15)%(max-min+1); }
	float Float(float min, float max)	{ Seed=214013*Seed+2531011; return min+2*(Seed>>16)*(1.0f/65535.0f)*(max-min); }
};

extern Random_Manager Random;

#endif