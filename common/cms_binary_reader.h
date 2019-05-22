/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: Ìì¿ÕÃ»ÓÐÎÚÔÆ/kisslovecsh@foxmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef BinaryReader_INCLUDED
#define BinaryReader_INCLUDED

#include <istream>

class  BinaryReader
{
public:

	BinaryReader(int sock);

	~BinaryReader();

	BinaryReader& operator >> (bool& value);
	BinaryReader& operator >> (char& value);
	BinaryReader& operator >> (unsigned char& value);
	BinaryReader& operator >> (signed char& value);
	BinaryReader& operator >> (short& value);
	BinaryReader& operator >> (unsigned short& value);
	BinaryReader& operator >> (int& value);
	BinaryReader& operator >> (unsigned int& value);
	BinaryReader& operator >> (long& value);
	BinaryReader& operator >> (unsigned long& value);
	BinaryReader& operator >> (float& value);
	BinaryReader& operator >> (double& value);
	BinaryReader& operator >> (long long& value);


	BinaryReader& operator >> (std::string& value);

	void read7BitEncoded(int& value);

	void readRaw(char* value, int length);

private:
	char* m_pHead;
	char* m_pPos;
	int   m_iLen;
	bool  m_bError;
};




#endif 
