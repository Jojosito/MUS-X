#pragma once

#include <rack.hpp>
#include <bitset>
#include <iostream>

namespace musx {

using namespace rack;
using simd::float_4;
using simd::int32_4;

class simdTests
{

public:

void test()
{
	uint32_t x0 = 0;
	uint32_t xf = -1;

	//std::cerr << std::hex << x0 << " " << xf << std::endl;

	float_4 b = {0.f, 1.f, 2.f, -3.f};

	float_4 mask = b > 0.5f; // 0x00000000 or 0xffffffff
	printHexValues(mask); // hex values:   0 ffffffff ffffffff 0
	for (size_t i=0; i<4; i++)
	{
		assert(*(uint32_t*)&mask[i] == x0 || *(uint32_t*)&mask[i] == xf);
	}


	float_4 testIfElse = {};
	testIfElse = simd::ifelse(mask, 1.f, -1.f);
	printFloatValues(testIfElse); // float values: -1 1 1 -1	OK


	float_4 testAnd = {};
	testAnd += mask & 5.f;
	printFloatValues(testAnd); // float values: 0 5 5 0		OK


	float_4 mask2 = b > 1.5f;
	float_4 testIfElse2 = {};
	testIfElse2 = simd::ifelse(mask & mask2, 1.f, -1.f);
	printFloatValues(testIfElse2); // float values: -1 -1 1 -1		OK


	float_4 testAnd2 = {};
	testAnd2 += mask & mask2 & 5.f;
	printFloatValues(testAnd2); // float values: 0 0 5 0


	// integers
	int32_4 intA = {0, INT32_MIN, INT32_MAX, INT32_MAX};
	int32_4 intB = {4, 1, 		  0,         INT32_MAX-1};
	//float_4 mask3 = intA > intB; // hex values:   0 0 bf800000 0    				  DOES NOT WORK AS EXPECTED!!!!!!
	//float_4 mask3 = (float_4)intA > (float_4)intB; // hex values:   0 0 ffffffff 0  DOES NOT WORK RELIABLY WITH BIG NUMBERS
	//float_4 mask3 = *(float_4*)&intA > *(float_4*)&intB; // hex values:   0 0 0 0   DOES NOT WORK AS EXPECTED!!!!!!
	int32_4 mask3Int = intA > intB;
	printIntValues(mask3Int); // int  values: 0 0 ffffffff ffffffff		OK
	float_4 mask3 = mask & *(float_4*)&mask3Int; // hex values:   0 0 ffffffff 0		OK
	printHexValues(mask3);


	// movemask
	int movemask1 = simd::movemask(mask); // 00000000000000000000000000000110  OK
	printIntBitValues(movemask1);

	int movemask2 = simd::movemask(mask3Int); // 00000000000000000000000000001100	OK
	printIntBitValues(movemask2);


}

void printIntValues(int32_4 x)
{
	std::cerr << "int  values:  ";
	for (size_t i=0; i<4; i++)
	{
		std::cerr << std::dec << x[i] << ' ';
	}
	std::cerr << std::endl;
}

void printIntHexValues(int32_4 x)
{
	std::cerr << "int hex values: ";
	for (size_t i=0; i<4; i++)
	{
		std::cerr << std::hex << x[i] << ' ';
	}
	std::cerr << std::endl;
}

void printIntBitValues(int i)
{
	std::cerr << std::bitset<32>{i} << std::endl;
}

void printFloatValues(float_4 x)
{
	std::cerr << "float values: ";
	for (size_t i=0; i<4; i++)
	{
		std::cerr << std::dec << x[i] << ' ';
	}
	std::cerr << std::endl;
}

void printHexValues(float_4 x)
{
	std::cerr << "hex values:   ";
	for (size_t i=0; i<4; i++)
	{
		printHexValue(x[i]);
	}
	std::cerr << std::endl;
}

void printHexValue(float x)
{
	uint32_t* i = (uint32_t*)&x;
	std::cerr << std::hex << *i << ' ';
}

};



}
