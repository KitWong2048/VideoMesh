/*
DHTNetworkID.cpp

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

#include "../DHT/DHTNetworkID.h"

using namespace Util;

namespace DHT{

	DHTNetworkID::DHTNetworkID(){
		memset(this->IDValue, 0, ID_LENGTH);
	}

	DHTNetworkID::DHTNetworkID(const unsigned char* _IDValue){
		memmove(this->IDValue, _IDValue, ID_LENGTH);
	}

	DHTNetworkID::DHTNetworkID(const char* _hexIDValue){
		setIDHex(_hexIDValue);
	}

	DHTNetworkID::DHTNetworkID(const DHTNetworkID& _ID){
		memset(this->IDValue, 0, ID_LENGTH);
		memmove(this->IDValue, _ID.IDValue, ID_LENGTH);
	}

	DHTNetworkID::~DHTNetworkID(){
	}

	const unsigned char* DHTNetworkID::getID() const{
		return IDValue;
	}

	int DHTNetworkID::setID(const unsigned char* _IDValue){
		memset(this->IDValue, 0, ID_LENGTH);
		memmove(this->IDValue, _IDValue, ID_LENGTH);
		return 0;
	}

	char* DHTNetworkID::toBin() const{
		char* x;
		x = new char[ID_LENGTH];
		memmove(x, IDValue, ID_LENGTH);
		return x;
	}

	string DHTNetworkID::getIDHex() const{
		char hexOutput[ID_LENGTH * 2 + 1];

		for (int i = 0; i < ID_LENGTH; i++)
			sprintf(hexOutput + i * 2, "%02x", (unsigned char) IDValue[i]);

		return string(hexOutput);
	}

	string DHTNetworkID::getIDDec() const{
		char decOutput[ID_LENGTH * 3 + 1];

		for (int i = 0; i < ID_LENGTH; i++)
			sprintf(decOutput + i * 3, "%03d", (unsigned char) IDValue[i]);

		return string(decOutput);
	}

	int DHTNetworkID::setIDHex(const char* _hexIDValue){
		unsigned int value[ID_LENGTH];

		for (int i = 0; i < ID_LENGTH; i++){
			sscanf(_hexIDValue + i * 2, "%02x", &value[i]);
			this->IDValue[i] = value[i];
		}

		return 0;
	}

	bool DHTNetworkID::isInInterval(const DHTNetworkID& _lower, const DHTNetworkID& _upper, bool _lowerClosed, bool _upperClosed) const{
		bool flag = false;

	//cout<<*this<<".checking "<<(_lowerClosed ? "[" : "(")<<_lower<<", "<<_upper<<(_upperClosed ? "]" : ")");

		if (_lower == _upper){
			// the upper and lower bounds are the same, i.e. the interval spans the whole Chord ring
			flag = true;
		}else if ((_lower < _upper) || (_upper == DHTNetworkID())){
			// the interval does not cross zero
			// if the upper bound is "0", we only need to compare the lower bound because we are doing modulus MATH
			if (*this > _lower)
				if (_upper == DHTNetworkID())
					flag = true;
				else if (*this <_upper)
					flag = true;
		}else{
			// the interval crosses zero
			if ((*this > _lower) || (*this < _upper))
				flag = true;
		}

		if (_lowerClosed)
			if (*this == _lower)
				flag =  true;

		if (_upperClosed)
			if (*this == _upper)
				flag = true;

		return flag;
	}

	DHTNetworkID DHTNetworkID::addPower(int _power) const{
		DHTNetworkID addedValue;
		int offsetPos;
		int offsetValue;
		bool isCarry;

		if ((_power < ID_BIT_LENGTH) && (_power >= 0)){
			addedValue = *this;
			offsetPos = ID_LENGTH - (_power / 8) - 1;
			offsetValue = int(pow(2.0, _power % 8));
			isCarry = false;

			do{
				if (IDValue[offsetPos] + offsetValue < 256){
					addedValue.IDValue[offsetPos] = addedValue.IDValue[offsetPos] + offsetValue;
					isCarry = false;
				}else{
					addedValue.IDValue[offsetPos] = (addedValue.IDValue[offsetPos] + offsetValue) % 256;
					// carry over
					offsetPos--;
					offsetValue = 1;
					isCarry = true;
				}
			}while ((isCarry) && (offsetPos>=0));
		}

		return addedValue;
	}

	bool DHTNetworkID::operator==(const DHTNetworkID& _ID) const {
		if (!memcmp(this->IDValue, _ID.IDValue, ID_LENGTH))
			return true;
		else
			return false;
	}

	bool DHTNetworkID::operator!=(const DHTNetworkID& _ID) const {
		if (!(*this == _ID))
			return true;
		else
			return false;
	}

	bool DHTNetworkID::operator<(const DHTNetworkID& _ID) const{
		if (memcmp(this->IDValue , _ID.IDValue, ID_LENGTH) < 0)
			return true;
		else
			return false;
	}

	bool DHTNetworkID::operator>(const DHTNetworkID& _ID) const {
		if ((!(*this < _ID)) && (*this != _ID))
			return true;
		else
			return false;
	}

	bool DHTNetworkID::operator<=(const DHTNetworkID& _ID) const {
		if ((*this < _ID) || (*this == _ID))
			return true;
		else
			return false;
	}

	bool DHTNetworkID::operator>=(const DHTNetworkID& _ID) const {
		if (!(*this < _ID))
			return true;
		else
			return false;
	}

	DHTNetworkID& DHTNetworkID::operator=(const DHTNetworkID& _ID){
		memmove(this->IDValue, _ID.IDValue, ID_LENGTH);

		return *this;
	}

	DHTNetworkID DHTNetworkID::operator+(const DHTNetworkID& _ID) const{
		DHTNetworkID newID;
		unsigned int carry = 0, sum = 0;
		int i = ID_LENGTH - 1;

		while (i >= 0){
			sum = this->IDValue[i] + _ID.IDValue[i] + carry;
			newID.IDValue[i] = (sum) % 256;
			carry = sum / 256;
			i--;
		}

		return newID;
	}

	DHTNetworkID DHTNetworkID::operator-(const DHTNetworkID& _ID) const{
		DHTNetworkID newID;
		unsigned int carry = 1, sum = 0;
		int i = ID_LENGTH -1 ;

		while (i >= 0){
			sum = this->IDValue[i] + 255 - _ID.IDValue[i] + carry;
			newID.IDValue[i] = (sum) % 256;
			carry = sum / 256;
			i--;
		}

		return newID;
	}

	ostream& operator<<(ostream& _out, const DHTNetworkID& _ID){
		_out<<_ID.getIDHex();

		return _out;
	}
}
