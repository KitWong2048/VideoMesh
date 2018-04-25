/*
AppValue.cpp

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

#include "../DHT/AppValue.h"

using namespace Util;

namespace DHT{
	AppValue::AppValue(){

	}

	AppValue::~AppValue(){

	}

	int AppValue::toBin(char*& _output) const{
		_output = new char[APP_VALUE_LEN];
		memmove(_output, value, APP_VALUE_LEN);
		return APP_VALUE_LEN;
	}

	int AppValue::fromBin(const char* _input){
		memmove(value, _input, APP_VALUE_LEN);
		return 0;
	}

	AbstractNodeValue& AppValue::operator=(const AbstractNodeValue& _value){
		const AppValue* p = (AppValue*) &_value;
		memmove(this->value, p->value, APP_VALUE_LEN);
		return *this;
	}

	int AppValue::len() const{
		return APP_VALUE_LEN;
	}

	string AppValue::toStr(){
		char buf[3 * APP_VALUE_LEN + 1];
		for (int i = 0; i < APP_VALUE_LEN; i++){
			sprintf(buf + 3 * i, "%03hhu", (unsigned char) value[i]);
		}
		return string(buf);
	}

}

