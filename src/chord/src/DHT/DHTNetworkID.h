/*
DHTNetworkID.h

Copyright (c) 2007 Hong Kong University of Science and Technology ("HKUST")
This source code is an intellectual property owned by HKUST and funded by
Innovation and Technology Fund (Ref No. GHP/045/05)

Permission is hereby granted, to any person and party obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the
Software with the rights to use, copy, modify and merge, subject to the
following conditions:

1. Commercial use of this source code or any derivatives works from this source
 code in any form is not allowed

2. Redistribution of this source code or any derivatives works from this source
 code in any form is not allowed

3. Any of these conditions can be waived if you get permission from the
copyright holder

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**	@file DHTNetworkID.h
 *	This file contains the class for encapsulating identifier used in Chord.
 */

#ifndef _H_DHT_NETWORK_ID_
#define _H_DHT_NETWORK_ID_

#include "../Util/Common.h"

using namespace std;

namespace DHT
{
	/**	@class DHTNetworkID
	 *	This class encapsulates identifier used for both node and key in the DHT network.
	 */
	class DHTNetworkID
	{
	protected:
		/**	@var IDValue
		 * Placeholder for storing  ID value.
		 */
		unsigned char IDValue[ID_LENGTH];

	public:
		/** @fn DHTNetworkID()
		 *	A constructor.
		 */
		DHTNetworkID();

		/** @fn DHTNetworkID(const unsigned char* _IDValue)
		 *	A constructor using binary representation of ID.
		 * @param _IDValue: the value of the DHT Network ID in unsigned char format.
		 */
		DHTNetworkID(const unsigned char* _IDValue);

		/** @fn DHTNetworkID(const char* _hexIDValue)
		 *	A constructor using hexadecimal representation of ID.
		 * @param _hexIDValue: the value of the DHT Network ID in hexadecimal format.
		 */
		DHTNetworkID(const char* _hexIDValue);

		/** @fn DHTNetworkID(const DHTNetworkID& _ID)
		 *	A copy constructor.
		 * @param _ID: the DHT Network ID to be copied.
		 */
		DHTNetworkID(const DHTNetworkID& _ID);

		~DHTNetworkID();

		const unsigned char* getID() const;

		int setID(const unsigned char* _IDValue);

		char* toBin() const;

		string getIDHex() const;

		string getIDDec() const;

		int setIDHex(const char* _hexIDValue);

		/** @fn bool isInInterval(const DHTNetworkID& _lower, const DHTNetworkID& _upper, bool _lowerClosed = false, bool _upperClosed = false) const
		 *	Check if the ID is between two given values.
		 * @param _lower: lower boundary.
		 * @param _upper: upper boundary.
		 * @param _lowerClosed: true if the lower boundary is included.
		 * @param _upperClosed: true if the upeer boundary is included.
		 */
		bool isInInterval(const DHTNetworkID& _lower, const DHTNetworkID& _upper, bool _lowerClosed = false, bool _upperClosed = false) const;

		/** @fn DHTNetworkID addPower(int _power) const
		 *	Add to the ID value by 2 to the power of a given value.
		 * @param _power: the number of times of power to raise,  0 <= _power < No of Bits in ID
		 * @return the ID with value added
		 */
		DHTNetworkID addPower(int _power) const;

		/** @fn bool operator==(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is equal to this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator==(const DHTNetworkID& _ID) const;

		/** @fn bool operator!=(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is not equal to this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator!=(const DHTNetworkID& _ID) const;

		/** @fn bool operator<(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is less than this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator<(const DHTNetworkID& _ID) const;

		/** @fn bool operator>(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is greater than this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator>(const DHTNetworkID& _ID) const;

		/** @fn bool operator<=(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is less than or equal to this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator<=(const DHTNetworkID& _ID) const;

		/** @fn bool operator>=(const DHTNetworkID& _ID) const
		 *	Check if the given DHTNetworkID is greater than or equal to this.
		 * @param _ID: the DHTNetworkID to compare with.
		 */
		bool operator>=(const DHTNetworkID& _ID) const;

		/** @fn DHTNetworkID& operator=(const DHTNetworkID& _ID)
		 *	Assignment operator overloading.
		 * Make a deep copy of the input object.
		 * @param _ID: the DHTNetworkID to be copied.
		 */
		DHTNetworkID& operator=(const DHTNetworkID& _ID);

		/** @fn DHTNetworkID operator+(const DHTNetworkID& _ID) const
		 *	Addition operator overloading.
		 * Add a DHTNetworkID to caller of this function.
		 * @param _ID: the DHTNetworkID to be added.
		 */
		DHTNetworkID operator+(const DHTNetworkID& _ID) const;

		/** @fn DHTNetworkID operator-(const DHTNetworkID& _ID) const
		 *	Subtraction operator overloading.
		 * Subtract a DHTNetworkID to caller of this function.
		 * @param _ID: the DHTNetworkID to be subtracted.
		 */
		DHTNetworkID operator-(const DHTNetworkID& _ID) const;


		/** @fn friend ostream& operator<<(ostream& _out, const DHTNetworkID& _ID)
		 *	output stream operator for this class
		 * @param _out: the stream to output.
		 * @param _ID: the DHTNetworkID to be output.
		 */
		friend ostream& operator<<(ostream& _out, const DHTNetworkID& _ID);
	};
}

#endif
