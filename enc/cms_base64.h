
/*

Copyright (C) 2007 Coolsoft Company. All rights reserved.

http://www.coolsoft-sd.com/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

 * The origin of this software must not be misrepresented; you must not claim that
   you wrote the original software. If you use this software in a product, an
   acknowledgment in the product documentation would be appreciated but is not required.
 * Altered source versions must be plainly marked as such, and must not be misrepresented
   as being the original software.
 * This notice may not be removed or altered from any source distribution.

*/

#pragma once

#include <string>
using namespace std;

// Encoding and decoding Base64 code
class Base64
{

public:
	//Standard base64
	// 63rd char used for Base64 code
	static const char CHAR_63 = '+';
	// 64th char used for Base64 code
	static const char CHAR_64 = '/';
	// Char used for padding
	static const char CHAR_PAD = '=';

	//not Standard base64
	// 63rd char used for Base64 code
	static const char CHAR_63_1 = '*';
	// 64th char used for Base64 code
	static const char CHAR_64_1 = '*';
	// Char used for padding
	static const char CHAR_PAD1 = '[';

public:

	// Encodes binary data to Base64 code
	// Returns size of encoded data.
	static int Encode(char *inData,
		int dataLength,
		char *outCode);

	static int Encode1(char *inData,
		int dataLength,
		char *outCode);

	// Decodes Base64 code to binary data
	// Returns size of decoded data.
	static int Decode(char *inCode,
		int codeLength,
		char *outData);

	static int Decode1(char *inCode,
		int codeLength,
		char *outData);

	// Returns maximum size of decoded data based on size of Base64 code.
	static int GetDataLength(int codeLength);

	// Returns maximum length of Base64 code based on size of uncoded data.
	static int GetCodeLength(int dataLength);

};
