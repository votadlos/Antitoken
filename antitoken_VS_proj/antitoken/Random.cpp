#include "Random.h"


Random::Random()
{
	srand(time(0));
}

void Random::genBytes(unsigned char *b, int n){
	for (int i = 0; i < n; i++)
		*(b+i) = rand() % 256;
}

Random::~Random()
{
}
