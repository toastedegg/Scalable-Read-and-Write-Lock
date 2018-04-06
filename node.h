#pragma once
#include<atomic>
using namespace std;

#ifndef NODE_H
#define NODE_H

class SNZI;
class node
{
public:
	atomic<uint32_t> xShared;//(1 bit 1/2, 15bits version, 16bits counter) in the root node the 1/2 bit is used for the a variable
	node *parent;
	SNZI *owner;
	node(node *myParent);
	~node();
	void rootArrive();
	void rootDepart();
	void arrive();
	void depart();
};

#endif
