//
// Simple (non-crypto!!!) PRNG
//
#pragma once
#include <cstdlib>
#include <ctime>

class Random
{
public:
	Random();
	~Random();
	void genBytes(unsigned char *b, int n);
private:

};

