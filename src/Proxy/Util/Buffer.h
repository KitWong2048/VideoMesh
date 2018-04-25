#ifndef _BUFFER_H
#define _BUFFER_H

#include "../Common/VMeshCommon.h"

/**	@file Buffer.h
 *	This file contains the definition of interface Buffer
 *	@author Kelvin Chan
 */
_VMESH_NAMESPACE_HEADER

/**	@class Buffer
 *	This class provides the abstract of putting and retrieving packets in memory
 */
class Buffer
{
public:
	/**	@fn Buffer()
	 *	Default blank constructor
	 *	BUFFER_SIZE is the buffer capacity defined VMeshCommon.h
	 *	@see VMeshCommon.h
	 */
	Buffer() {};

	/**	@fn ~Buffer()
	 *	Default blank destructor
	 */
	virtual ~Buffer() {};

	/**	@fn read(unsigned int _index, unsigned int& _length)
	 *	get data from the buffer
	 *	@param _index the index of the data to be retrieved
	 *	@param _length length of the data retrieved would be passed by reference
	 *	@return data of allocated memory
	 *	@return NULL if not exists
	 */
	virtual const char* read(unsigned int _index, unsigned int& _length) = 0;

	/**	@fn write(unsigned int _index, const char* _data, unsigned int _length)
	 *	write data to the buffer
	 *	@param _index the index of the data to be written
	 *	@param _data array of the data (deep copy)
	 *	@param _length length of the data to be written
	 *	@return true if write succeeds, false otherwise
	 *	@return false if the index already exists or the file is full
	 */
	virtual bool write(unsigned int _index, const char* _data, unsigned int _length) = 0;

	/**	@fn exist(unsigned int _index)
	 *	check if an index exist in the buffer
	 *	@param _index the index of the data
	 *	@return true if the index exists, false otherwise
	 */
	virtual bool exist(unsigned int _index) = 0;

	/**	@fn getCapacity()
	 *	get the capacity size of the buffer
	 *	@return the capacity size of the buffer
	 */
	virtual unsigned int getCapacity() = 0;

	/**	@fn getSize()
	 *	get the used size of the buffer
	 *	@return the used size of the buffer
	 */
	virtual unsigned int getSize() = 0;

	/**	@fn getElementCount()
	 *	get the number of index in the buffer
	 *	@return the number of index in the buffer
	 */
	virtual int getElementCount() = 0;

private:

};
_VMESH_NAMESPACE_FOOTER
#endif
