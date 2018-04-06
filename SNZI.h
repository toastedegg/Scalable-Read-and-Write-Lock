#pragma once
#include<atomic>
using namespace std;


#ifndef SNZI_H
#define SNZI_H

class node;
class SNZI
{
public:
	int maxThreadNum;
	int leafNum;
	node** tree;
	node** leaf;
	atomic<uint32_t> I;//1 bit for the boolean, 31 bits for the v;
	SNZI(int threadNum);
	~SNZI();
	void arrive(int threadId);
	void depart(int threadId);
	uint32_t getI();
};

#endif
