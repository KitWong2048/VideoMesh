#ifndef _CIRCULARBUFFER_H
#define _CIRCULARBUFFER_H

#include <list>
#include <algorithm>
#include <string>
#include <stdlib.h>
#include "../Common/VMeshCommon.h"
#include "../Util/Mutex.h"
#include "../Util/Buffer.h"
#include "../Util/Thread.h"
#include "../Util/Logger.h"

/**	@file CircularBuffer.h
 *	This file contains the definition of class CircularBuffer
 *	@author Kelvin Chan
 */
_VMESH_NAMESPACE_HEADER

/** @struct CircularBufferEntry
 *  A struct to store the packet information in buffer
 */
struct CircularBufferEntry
{
	unsigned int ID;	/*!< the index (packet ID) */
	int length;			/*!< the length of the data (serizlized packet) */
	char* ptr;			/*!< the pointer of the packet */
};

typedef struct _CircularBufferList
{
	CircularBufferEntry entry;
	_CircularBufferList* Next;
	_CircularBufferList()
	{
		Next=NULL;
	}
} CircularBufferList;
typedef CircularBufferList* CircularBufferListPtr;
const int NUM_OF_CIRCULAR_LIST = 128;
const int NUM_OF_CIRCULAR_LIST_MINUS_ONE = NUM_OF_CIRCULAR_LIST - 1;
/**	@class CircularBuffer
 *	This class implements a Circular Map;
 *	The memory location of the data are put into this buffer;
 *	and the pointer can be retrieved later;
 */
class CircularBuffer : public Buffer{
public:


	/**	@fn CircularBuffer(unsigned int _capacity)
	 *	Constructor that initialize data members;
	 *	@param _capacity size of capacity of the buffer
	 */
	CircularBuffer(unsigned int _capacity);

	/**	@fn ~CircularBuffer()
	 *	Default destructor and destruct everything inside the buffer
	 */
	virtual ~CircularBuffer();

	virtual const char* read(unsigned int _index, unsigned int& _length);
	
	virtual bool write(unsigned int _index, const char* _data, unsigned int _length);
	
	virtual bool exist(unsigned int _index);

	virtual unsigned int getCapacity();

	virtual unsigned int getSize();

	virtual int getElementCount();

private:
	//CircularBufferListPtr PreviousEntry(CircularBufferListPtr index);
	//void DeleteEntry(CircularBufferListPtr index);
	unsigned int myCapacity;
	unsigned int mySize;
	int myElementCount;
	Mutex* myAccess[NUM_OF_CIRCULAR_LIST];
	int myElementCountPerList[NUM_OF_CIRCULAR_LIST];
	CircularBufferListPtr CircularBufferListHead[NUM_OF_CIRCULAR_LIST];
	CircularBufferListPtr CircularBufferListTail[NUM_OF_CIRCULAR_LIST];
};
_VMESH_NAMESPACE_FOOTER
#endif
