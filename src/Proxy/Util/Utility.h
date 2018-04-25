#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <string>
#include "../Common/VMeshCommon.h"

/**	@file Utility.h
 *	This file contains the general purpose utility functions
 *	@author Kelvin Chan
 */
_VMESH_NAMESPACE_HEADER


/**	@fn packIntIntoChar(const int val, char* data)
 *	Convert an integer into character array representation
 *	@param val integer to be packed
 *	@param data starting position of char string to be filled
 *	@return number of char string filled (sizeof(int))
 */
int packIntIntoChar(const int val, char* data);

/**	@fn unpackIntFromChar(int& val, const char* data)
 *	Convert character array representation into an integer
 *	@param val integer to be unpacked
 *	@param data starting position of char string to be parsed
 *	@return number of characters parsed (sizeof(int))
 */
int unpackIntFromChar(int& val, const char* data);

/**	@fn packUIntIntoChar(const unsigned int val, char* data)
 *	Convert an unsigned integer into character array representation
 *	@param val integer to be packed
 *	@param data starting position of char string to be filled
 *	@return number of char string filled (sizeof(unsigned int))
 */
int packUIntIntoChar(const unsigned int val, char* data);

/**	@fn unpackUIntFromChar(unsigned int& val, const char* data)
 *	Convert character array representation into an unsigned integer
 *	@param val integer to be unpacked
 *	@param data starting position of char string to be parsed
 *	@return number of characters parsed (sizeof(unsigned int))
 */
int unpackUIntFromChar(unsigned int& val, const char* data);

/**	@fn itos(int val)
 *	Convert an integer to a string representation
 *	@param val integer to be converted
 *	@return string representation of an integer
 */
std::string itos(int val);

/**	@fn getHostIP()
 *	Get the IP address of local machine
 *	@return the IP address of local machine
 */
std::string getHostIP();

_VMESH_NAMESPACE_FOOTER
#endif
