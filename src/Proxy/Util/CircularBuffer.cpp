#include "../Util/CircularBuffer.h"

using namespace std;

_VMESH_NAMESPACE_HEADER


CircularBuffer::CircularBuffer(unsigned int _capacity) //:({"CircularBufferAccess"})
{
	srand(time(0));
	myCapacity = _capacity;
	mySize = 0;
	myElementCount = 0;
	int i;
	for ( i = 0; i < NUM_OF_CIRCULAR_LIST; i++ )
	{
		CircularBufferListHead[i] = CircularBufferListTail[i] = NULL;
		myAccess[i] = new Mutex("CircularBufferAccess");
		myElementCountPerList[i] = 0;
	}
}

CircularBuffer::~CircularBuffer()
{
	CircularBufferListPtr tempPtr, nextPtr;
	int i;
	for ( i = 0; i < NUM_OF_CIRCULAR_LIST; i++ )
	{
		for (tempPtr = CircularBufferListHead[i]; tempPtr!=NULL; tempPtr=nextPtr)
		{
			if (tempPtr->entry.ptr!=NULL)
			{
				delete [] tempPtr->entry.ptr;
				tempPtr->entry.ptr = NULL;
			}
			nextPtr = tempPtr->Next;
			delete tempPtr;
		}
		delete myAccess[i];
	}
}

const char* CircularBuffer::read(unsigned int _index, unsigned int& _length)
{
	int listIndex = _index & NUM_OF_CIRCULAR_LIST_MINUS_ONE;
	myAccess[listIndex]->lock();
	char* buffer =NULL;

	CircularBufferListPtr tempPtr;
	CircularBufferListPtr prevPtr = NULL;
	for (tempPtr = CircularBufferListHead[listIndex]; tempPtr!=NULL; tempPtr=tempPtr->Next)
	{
		if (tempPtr->entry.ID == _index)
		{
			// replicate the data
			buffer = tempPtr->entry.ptr;
			_length = tempPtr->entry.length;

			break;
		}
		prevPtr = tempPtr;
	}

	myAccess[listIndex]->release();
	return buffer;
}

bool CircularBuffer::write(unsigned int _index, const char* _data, unsigned int _length)
{
	int listIndex = _index & NUM_OF_CIRCULAR_LIST_MINUS_ONE;
	myAccess[listIndex]->lock();
		
	CircularBufferListPtr tempPtr;
	CircularBufferListPtr prevPtr = NULL;
	for (tempPtr = CircularBufferListHead[listIndex]; tempPtr!=NULL; tempPtr=tempPtr->Next)
	{
		if (tempPtr->entry.ID == _index)
		{

			if ( tempPtr != CircularBufferListTail[listIndex] )
			{
				if ( prevPtr )
					prevPtr->Next = tempPtr->Next;
				else
					CircularBufferListHead[listIndex] = tempPtr->Next;

				tempPtr->Next = NULL;
				CircularBufferListTail[listIndex]->Next = tempPtr;
				CircularBufferListTail[listIndex] = CircularBufferListTail[listIndex]->Next;
			}

			myAccess[listIndex]->release();
			return true;
		}
		prevPtr = tempPtr;
	}

	if (_data == NULL || _length == 0)
	{
		// LOG_DEBUG("Buffer write " + itos(_index) + " fails: _index not exists");
		myAccess[listIndex]->release();
		return false;
	}

	// allocate memory
	if (CircularBufferListHead[listIndex]==NULL)
	{
		CircularBufferListHead[listIndex] = new CircularBufferList;
		CircularBufferListTail[listIndex] = CircularBufferListHead[listIndex];
	}
	else
	{
		CircularBufferListTail[listIndex]->Next = new CircularBufferList;
		CircularBufferListTail[listIndex] = CircularBufferListTail[listIndex]->Next;
	}

	// add it to the back of the list
	CircularBufferListTail[listIndex]->entry.ID = _index;
	CircularBufferListTail[listIndex]->entry.length = _length;
	CircularBufferListTail[listIndex]->entry.ptr = new char[_length];
	memcpy(CircularBufferListTail[listIndex]->entry.ptr, _data, _length);

	mySize += _length;
	myElementCount++;
	myElementCountPerList[listIndex]++;
	myAccess[listIndex]->release();

	// clear the oldest element to meet the capacity
	while (mySize > myCapacity)
	{
		int selectedIndex = rand() & NUM_OF_CIRCULAR_LIST_MINUS_ONE;

		myAccess[selectedIndex]->lock();

		// select the circular list with less elements to remove
		if ( selectedIndex != listIndex )
		{
			myAccess[listIndex]->lock();
			if ( myElementCountPerList[listIndex] > myElementCountPerList[selectedIndex] )
			{
				myAccess[selectedIndex]->release();
				selectedIndex = listIndex;
			}
			else
				myAccess[listIndex]->release();
		}

		if ( CircularBufferListHead[selectedIndex] )
		{
			unsigned int lastSize =	CircularBufferListHead[selectedIndex]->entry.length;
			char* lastPtr = CircularBufferListHead[selectedIndex]->entry.ptr;
			if (lastPtr!=NULL)
				delete [] lastPtr;
			mySize -= lastSize;
			myElementCount--;
			myElementCountPerList[selectedIndex]--;
			tempPtr = CircularBufferListHead[selectedIndex];
			CircularBufferListHead[selectedIndex] = CircularBufferListHead[selectedIndex]->Next;
			delete tempPtr;
		}

		myAccess[selectedIndex]->release();
	}

	return true;
}

bool CircularBuffer::exist(unsigned int _index)
{
	int listIndex = _index & NUM_OF_CIRCULAR_LIST_MINUS_ONE;
	myAccess[listIndex]->lock();
	CircularBufferListPtr tempPtr;
	for (tempPtr = CircularBufferListHead[listIndex]; tempPtr!=NULL; tempPtr=tempPtr->Next)
		if (tempPtr->entry.ID == _index)
		{
			myAccess[listIndex]->release();
			return true;
		}
	myAccess[listIndex]->release();
	return false;
}


unsigned int CircularBuffer::getCapacity()
{
	return myCapacity;
}

unsigned int CircularBuffer::getSize()
{
	return mySize;
}

int CircularBuffer::getElementCount()
{
	return myElementCount;
}

_VMESH_NAMESPACE_FOOTER
