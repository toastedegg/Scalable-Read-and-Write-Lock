#include <atomic>
#include <iostream>
#include "SNZI.h"
#include "node.h"
#include "rwlock.h"
using namespace std;

node::node(node *myParent)
	{
		owner = NULL;
		parent = myParent;
		xShared.store(0, memory_order_relaxed);
	}
node::~node() {}
void node::rootArrive()
	{
		uint32_t xNew;
		uint32_t xLocal;
		//uint32_t xLocaltmp;
		uint32_t v;
		uint32_t c;
		uint32_t a;
		uint32_t Ilocal;
		uint32_t Inew;
		while (true)
		{
			xLocal = xShared.load(memory_order_acquire);
			//xLocaltmp = xLocal;
			a = xLocal&(0x80000000);
			v = xLocal&(0x7fff0000);
			c = xLocal&(0x0000ffff);
			if (c == 0)
			{
				xNew = 0x80000000 + ((((v >> 16) + 1)&(0x00007fff)) << 16) + 0x00000001;
			}
			else
			{
				xNew = a + v + c + 1;
			}
			if (atomic_compare_exchange_strong_explicit(&xShared, &xLocal, xNew, memory_order_seq_cst, memory_order_seq_cst))
			{
				//cout << (xNew & 0x0000ffff);
				break;
			}
		}
		if ((xNew & 0x80000000) == 0x80000000)
		{
			while (true)
			{
				Ilocal = owner->I.load(memory_order_acquire);
				Inew = 0x80000000 + (((Ilocal & 0x7fffffff) + 1) & 0x7fffffff);
				if (atomic_compare_exchange_strong_explicit(&owner->I, &Ilocal, Inew, memory_order_seq_cst, memory_order_seq_cst))
				{
					//cout << 'y';
					break;
				}
			}
			xNew = 0x00000000 + v + c;
			atomic_compare_exchange_strong_explicit(&xShared, &xLocal, xNew, memory_order_seq_cst, memory_order_seq_cst);
		}
	}
void node::rootDepart()
	{
		uint32_t xNew;
		uint32_t xLocal;
		uint32_t v;
		uint32_t c;
		uint32_t a;
		uint32_t Ilocal;
		uint32_t Inew;
		while (true)
		{
			xLocal = xShared.load(memory_order_acquire);
			//a = xLocal&(0x8000);
			v = xLocal&(0x7fff0000);
			c = xLocal&(0x0000ffff);
			xNew = 0x00000000 + v + c - 1;
			if (atomic_compare_exchange_strong_explicit(&xShared, &xLocal, xNew, memory_order_seq_cst, memory_order_seq_cst))
			{
				//cout << c - 1;
				if (c >= 2)
					return;
				while (true)
				{
					Ilocal = owner->I.load(memory_order_acquire);
					if ((xShared.load(memory_order_acquire) & 0x7fff0000) != v)
						return;
					Inew = 0x00000000 + (((Ilocal & 0x7fffffff) + 1) & 0x7fffffff);
					if (atomic_compare_exchange_strong_explicit(&owner->I, &Ilocal, Inew, memory_order_seq_cst, memory_order_seq_cst))
					{
						//cout << 'k';
						return;
					}
				}
			}
		}
	}
void node::arrive()
	{
		if (parent == NULL)//root
		{
			this->rootArrive();
			return;
		}
		uint32_t xNew;
		bool succ = false;
		int undoArr = 0;
		uint32_t xLocal;
		uint32_t xLocaltmp;
		uint32_t v;
		uint32_t c;
		uint32_t half;
		while (!succ)
		{
			xLocal = xShared.load(memory_order_acquire);
			xLocaltmp = xLocal;
			half = xLocal&(0x80000000);
			v = xLocal&(0x7fff0000);
			c = xLocal&(0x0000ffff);
			if (c >= 1)
			{
				xNew = half + v + c + 1;
				if (atomic_compare_exchange_strong_explicit(&xShared, &xLocaltmp, xNew, memory_order_seq_cst, memory_order_seq_cst))
				{
					succ = true;
				}
				else
					xLocaltmp = xLocal;
			}
			if ((c == 0) && (half == 0x00000000))
			{
				xNew = 0x80000000 + c + ((((v >> 16) + 1)&(0x00007fff)) << 16);
				if (atomic_compare_exchange_strong_explicit(&xShared, &xLocaltmp, xNew, memory_order_seq_cst, memory_order_seq_cst))
				{
					succ = true;
					xLocal = xNew;
					xLocaltmp = xLocal;
					half = xLocal&(0x80000000);
					v = xLocal&(0x7fff0000);
					c = xLocal&(0x0000ffff);
				}
				else
					xLocaltmp = xLocal;
			}
			if (half == 0x80000000)
			{
				parent->arrive();
				xNew = 0x00000000 + v + 0x00000001;
				if (!atomic_compare_exchange_strong_explicit(&xShared, &xLocaltmp, xNew, memory_order_seq_cst, memory_order_seq_cst))
				{
					undoArr++;
				}
			}
		}
		while (undoArr > 0)
		{
			parent->depart();
			undoArr--;
		}
	}
void node::depart()
	{
		if (parent == NULL)
		{
			this->rootDepart();
			return;
		}
		uint32_t xNew;
		uint32_t xLocal;
		uint32_t xLocaltmp;
		uint32_t v;
		uint32_t c;
		uint32_t half;
		while (true)
		{
			xLocal = xShared.load(memory_order_acquire);
			xLocaltmp = xLocal;
			half = xLocal & 0x80000000;
			v = xLocal & 0x7fff0000;
			c = xLocal & 0x0000ffff;
			if (c < 1)
			{
				//cout << 'f';//asertion fail
				break;
			}
			xNew = half + v + c - 1;
			if (atomic_compare_exchange_strong_explicit(&xShared, &xLocaltmp, xNew, memory_order_seq_cst, memory_order_seq_cst))
			{
				if (c == 1)
					parent->depart();
				return;
			}
		}
	}

SNZI::SNZI(int threadNum)
	{
		I.store(0, memory_order_release);
		int le = 2;
		maxThreadNum = 3;
		while (le*2 <= threadNum)
		{
			le *= 2;
			maxThreadNum += le;
		}
		leafNum=le;
		tree = new node*[maxThreadNum];
		tree[0] = new node(NULL);
		tree[0]->owner = this;
		for (int i = 1; i < maxThreadNum; i++)
		{
			tree[i] = new node(tree[(i - 1) / 2]);
		}
		leaf=new node*[le];
		for (int i=0;i<leafNum;i++)
		{
			leaf[i]=tree[i+maxThreadNum-leafNum];
		}
	}
SNZI::~SNZI()
	{
		for (int i = 0; i < maxThreadNum; i++)
			delete tree[i];
		delete[]tree;
		delete[]leaf;
	}
void SNZI::arrive(int threadId)
	{
		leaf[threadId%leafNum]->arrive();
	}
void SNZI::depart(int threadId)
	{
		leaf[threadId%leafNum]->depart();
	}
uint32_t SNZI::getI()
{
	return (I.load(memory_order_seq_cst) >> 31);
}

/*void foo(SNZI* mysnzi)
{
	int mynode = mysnzi->arrive();
	cout << 'i';
	mysnzi->depart(mynode);
	cout << 'o';
	mynode = mysnzi->arrive();
	cout << 'i';
	mysnzi->depart(mynode);
	cout << 'o';
	mynode = mysnzi->arrive();
	cout << 'i';
	mysnzi->depart(mynode);
	cout << 'o';
}

int main()
{
	thread a[5];
	SNZI *mysnzi = new SNZI(5);
	for (int i = 0; i < 3; i++)
	{
		a[i] = thread(&foo, mysnzi);
	}
	for (int i = 0; i < 3; i++)
	{
		a[i].join();
	}
	delete mysnzi;
	return 0;
}*/
