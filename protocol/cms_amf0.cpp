/*
The MIT License (MIT)

Copyright (c) 2017- cms(hsc)

Author: 天空没有乌云/kisslovecsh@foxmail.com

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
#include <protocol/cms_amf0.h>
#include <common/cms_utility.h>
#include <log/cms_log.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <sstream>
#include <netinet/in.h>

namespace amf0
{

	char *bitReversal(char *bit, int len)
	{
		// 		if (isBigEndian())
		// 		{
		// 			return bit;
		// 		}
		char *pBegin = bit;
		char *pEnd = bit + len - 1;

		for (int i = 0; i < len / 2; ++i)
		{
			char temp;
			temp = *pBegin;
			*pBegin = *pEnd;
			*pEnd = temp;

			++pBegin;
			--pEnd;
		}
		return bit;
	}

	char *bit64Reversal(char *bit)
	{
		// 		if (isBigEndian())
		// 		{
		// 			return bit;
		// 		}
		char *pBegin = bit;
		char *pEnd = bit + 7;

		for (int i = 0; i < 4; ++i)
		{
			char temp;
			temp = *pBegin;
			*pBegin = *pEnd;
			*pEnd = temp;

			++pBegin;
			--pEnd;
		}
		return bit;
	}

	Amf0Data *amf0DataNew(uint8 type)
	{
		Amf0Data *data = new Amf0Data;
		if (data != NULL)
		{
			data->type = type;
		}
		return data;
	}

	uint32 amf0DataSize(Amf0Data *data)
	{
		uint32 size = 0;
		Amf0Node *node = NULL;
		if (data != NULL)
		{
			size += sizeof(uint8);
			switch (data->type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				size += sizeof(double);
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				size += sizeof(uint8);
			}
			break;
			case AMF0_TYPE_STRING:
			{
				size += sizeof(uint16) + (uint32)amf0StringGetSize(data);
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				node = amf0ObjectFirst(data);
				while (node != NULL)
				{
					size += sizeof(uint16) + (uint32)amf0StringGetSize(amf0ObjectGetName(node));
					size += (uint32)amf0DataSize(amf0ObjectGetData(node));
					node = amf0ObjectNext(node);
				}
				size += sizeof(uint16) + sizeof(uint8);
			}
			break;
			case AMF0_TYPE_NULL:
			{
				//size += sizeof(uint8);
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				//logs->info("+++ amf0DataSize unhandle type[ %d ] +++", data->type);
			}
			break;
			/*case AMF0_TYPE_REFERENCE:*/
			case AMF0_TYPE_ECMA_ARRAY:
			{
				size += sizeof(uint32);
				node = amf0ObjectFirst(data);
				while (node != NULL)
				{
					size += sizeof(uint16) + (uint32)amf0StringGetSize(amf0ObjectGetName(node));
					size += (uint32)amf0DataSize(amf0ObjectGetData(node));
					node = amf0ObjectNext(node);
				}
				size += sizeof(uint16) + sizeof(uint8);
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				size += sizeof(uint32);
				node = amf0ArrayFirst(&data->array_data);
				while (node != NULL)
				{
					size += (uint32)amf0DataSize(amf0ArrayGet(node));
					node = amf0ArrayNext(node);
				}
			}
			break;
			case AMF0_TYPE_DATE:
			{
				size += sizeof(uint64) + sizeof(int16);
			}
			break;
			/*case AMF0_TYPE_SIMPLEOBJECT:*/
			case AMF0_TYPE_TYPED_OBJECT:
			case AMF0_TYPE_AMF3:
			case AMF0_TYPE_OBJECT_END:
			{
				//logs->info("+++ amf0DataSize unhandle type[ %d ] +++", data->type);
			}
			break; /* end of composite object */
			default:
			{
				logs->info("+++ amf0DataSize unexpect type[ %d ] +++", data->type);
			}
			break;
			}
		}
		return size;
	}

	uint8 amf0DataGetType(Amf0Data *data)
	{
		return data != NULL ? data->type : AMF0_TYPE_NULL;
	}
	/* return a new copy of AMF data */
	//Amf0Data *amf0DataClone(Amf0Data *data);

	void amf0DataFree(Amf0Data *data)
	{
		Amf0Node *node = NULL;
		if (data != NULL)
		{
			switch (data->type)
			{
			case AMF0_TYPE_NUMERIC:
				break;
			case AMF0_TYPE_BOOLEAN:
				break;
			case AMF0_TYPE_STRING:
				if (data->string_data.mbstr != NULL)
				{
					delete[] data->string_data.mbstr;
				}
				break;
			case AMF0_TYPE_OBJECT:
			case AMF0_TYPE_ECMA_ARRAY:
			case AMF0_TYPE_STRICT_ARRAY:
			{
				node = amf0ObjectFirst(data);
				Amf0Node *tmp = NULL;
				while (node != NULL)
				{
					amf0DataFree(node->data);
					tmp = node;
					node = node->next;
					delete tmp;
				}
				data->array_data.size = 0;
			}
			break;
			case AMF0_TYPE_NULL:
			case AMF0_TYPE_UNDEFINED:
				break;
			case AMF0_TYPE_DATE:
				break;
				/*case AMF0_TYPE_SIMPLEOBJECT:*/
			case AMF0_TYPE_TYPED_OBJECT:
			case AMF0_TYPE_AMF3:
			case AMF0_TYPE_OBJECT_END:
				break; /* end of composite object */
			default:
				break;
			}
			delete data;;
		}
	}
	/* dump AMF data into a string */
	std::string amf0DataDumpString(Amf0Data *data, char *retract, bool key/* = false*/)
	{
		std::string strAmfData;
		std::ostringstream strStream;
		if (data != NULL)
		{
			switch (data->type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				double d = data->number_data;
				strStream << retract << "Number" << " " << d << "\n";
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				char szTrue[] = "true";
				char szFlase[] = "false";
				if (data->boolean_data == 0)
				{
					strStream << retract << "Boolean" << " " << szFlase << "\n";
				}
				else
				{
					strStream << retract << "Boolean" << " " << szTrue << "\n";
				}
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_STRING:
			{
				uint16 len = data->string_data.size;
				len = htons(len);
				if (key == true)
				{
					strStream << retract << "Name: " << data->string_data.mbstr << "\n";
				}
				else
				{
					strStream << retract << "String" << " " << data->string_data.mbstr << "\n";
				}
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				Amf0Node *node = amf0ObjectFirst(data);
				int index = 0;
				strStream << retract << "Object" << "\n";
				strAmfData.append(strStream.str());
				std::string strRetract = retract;
				strRetract += "    ";
				while (node != NULL)
				{
					std::string strNode = amf0DataDumpString(node->data, (char *)strRetract.c_str(), index % 2 == 0 ? true : false);
					if (index % 2 == 0)
					{
						strAmfData.append("  Property\n");
					}
					strAmfData.append(strNode);
					node = node->next;
					++index;
				}
			}
			break;
			case AMF0_TYPE_NULL:
			{
				strStream << retract << "NULL" << "\n";
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				strStream << retract << "undefined" << "\n";
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_ECMA_ARRAY:
			{
				int len = amf0ObjectSize(data);
				strStream << retract << "ECMA Array Length: " << len << "\n";
				strAmfData.append(strStream.str());
				Amf0Node *node = amf0ObjectFirst(data);
				int index = 0;
				std::string strRetract = retract;
				strRetract += "    ";
				while (node != NULL)
				{
					std::string strNode = amf0DataDumpString(node->data, (char *)strRetract.c_str(), index % 2 == 0 ? true : false);
					if (index % 2 == 0)
					{
						strAmfData.append("  Property\n");
					}
					strAmfData.append(strNode);
					node = node->next;
					++index;
				}
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				int len = amf0ArraySize(&data->array_data);
				strStream << retract << "STRICT Array Length: " << len << "\n";
				strAmfData.append(strStream.str());
				Amf0Node *node = amf0ArrayFirst(&data->array_data);
				std::string strRetract = retract;
				strRetract += "    ";
				while (node != NULL)
				{
					std::string strNode = amf0DataDumpString(node->data, (char *)strRetract.c_str());
					strAmfData.append(strNode);
					node = amf0ArrayNext(node);
				}
			}
			break;
			case AMF0_TYPE_DATE:
			{
				uint16 zone = 8; //中国时区
				std::string strRetract = retract;
				strRetract += "    ";
				strStream << retract << "DATE \n" << strRetract << "time: " << data->date_data.milliseconds << "\n" << strRetract << "zone: " << zone << "\n";
				strAmfData.append(strStream.str());
			}
			break;
			case AMF0_TYPE_TYPED_OBJECT:
			case AMF0_TYPE_AMF3:
			case AMF0_TYPE_OBJECT_END:
			{
				//logs->info("+++ amf0Data2String unhandle type[ %d ] +++", data->type);
			}
			break; /* end of composite object */
			default:
			{
				logs->info("+++ amf0Data2String unexpect type[ %d ] +++", data->type);
			}
			}
		}
		return strAmfData;
	}
	std::string amf0Data2String(Amf0Data *data, bool key/* = false*/)
	{
		std::string strAmfData;
		if (data != NULL)
		{
			switch (data->type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				double d = data->number_data;
				char *bit = bit64Reversal((char *)&d);
				strAmfData.append(1, (char)AMF0_TYPE_NUMERIC);
				strAmfData.append(bit, sizeof(double));
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				strAmfData.append(1, (char)AMF0_TYPE_BOOLEAN);
				strAmfData.append(1, (char)data->boolean_data);
			}
			break;
			case AMF0_TYPE_STRING:
			{
				uint16 len = data->string_data.size;
				len = htons(len);
				if (key == false)
				{
					strAmfData.append(1, (char)AMF0_TYPE_STRING);
				}
				strAmfData.append((char *)&len, sizeof(uint16));
				strAmfData.append((char *)data->string_data.mbstr, data->string_data.size);
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				strAmfData.append(1, (char)AMF0_TYPE_OBJECT);
				Amf0Node *node = amf0ObjectFirst(data);
				int index = 0;
				while (node != NULL)
				{
					std::string strNode = amf0Data2String(node->data, index % 2 == 0 ? true : false);
					strAmfData.append(strNode);
					node = node->next;
					++index;
				}
				strAmfData.append(1, (char)0x00);
				strAmfData.append(1, (char)0x00);
				strAmfData.append(1, (char)0x09);
			}
			break;
			case AMF0_TYPE_NULL:
			{
				strAmfData.append(1, (char)AMF0_TYPE_NULL);
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				strAmfData.append(1, (char)AMF0_TYPE_UNDEFINED);
			}
			break;
			/*case AMF0_TYPE_REFERENCE:*/
			case AMF0_TYPE_ECMA_ARRAY:
			{
				int len = amf0ObjectSize(data);
				len = htonl(len);
				strAmfData.append(1, (char)AMF0_TYPE_ECMA_ARRAY);
				strAmfData.append((char *)&len, sizeof(int));
				Amf0Node *node = amf0ObjectFirst(data);
				int index = 0;
				while (node != NULL)
				{
					std::string strNode = amf0Data2String(node->data, index % 2 == 0 ? true : false);
					strAmfData.append(strNode);
					node = node->next;
					++index;
				}
				strAmfData.append(1, (char)0x00);
				strAmfData.append(1, (char)0x00);
				strAmfData.append(1, (char)0x09);
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				int len = amf0ArraySize(&data->array_data);
				len = htonl(len);
				strAmfData.append(1, (char)AMF0_TYPE_STRICT_ARRAY);
				strAmfData.append((char *)&len, sizeof(int));
				Amf0Node *node = amf0ArrayFirst(&data->array_data);
				while (node != NULL)
				{
					std::string strNode = amf0Data2String(node->data);
					strAmfData.append(strNode);
					node = amf0ArrayNext(node);
				}
			}
			break;
			case AMF0_TYPE_DATE:
			{
				char *bit = bit64Reversal((char *)&data->date_data.milliseconds);
				uint16 zone = 8; //中国时区
				zone = htons(zone);
				strAmfData.append(1, (char)AMF0_TYPE_DATE);
				strAmfData.append(bit, sizeof(double));
				strAmfData.append((char*)&zone, sizeof(uint16));
			}
			break;
			/*case AMF0_TYPE_SIMPLEOBJECT:*/
			case AMF0_TYPE_TYPED_OBJECT:
			case AMF0_TYPE_AMF3:
			case AMF0_TYPE_OBJECT_END:
			{
				//logs->info("+++ amf0Data2String unhandle type[ %d ] +++", data->type);
			}
			break; /* end of composite object */
			default:
			{
				logs->info("+++ amf0Data2String unexpect type[ %d ] +++", data->type);
			}
			}
		}
		return strAmfData;
	}

	Amf0Data *amf0NullNew()
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_NULL);
		return data;
	}
	Amf0Data *amf0UndefinedNew()
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_UNDEFINED);
		return data;
	}

	Amf0Data *amf0NumberNew(double value)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_NUMERIC);
		if (data != NULL)
		{
			data->number_data = value;
		}
		return data;
	}

	double amf0NumberGetValue(Amf0Data *data)
	{
		return data != NULL ? data->number_data : 0;
	}

	void amf0NumberSetValue(Amf0Data *data, double value)
	{
		if (data != NULL)
		{
			data->number_data = value;
		}
	}

	/* boolean functions */
	Amf0Data *amf0BooleanNew(uint8 value)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_BOOLEAN);
		if (data != NULL)
		{
			data->boolean_data = value;
		}
		return data;
	}

	uint8 amf0BooleanGetValue(Amf0Data *data)
	{
		return data != NULL ? data->boolean_data : 0;
	}

	void amf0BooleanSetValue(Amf0Data *data, uint8 value)
	{
		if (data != NULL)
		{
			data->boolean_data = value;
		}
	}

	/* string functions */
	Amf0Data *amf0StringNew(uint8 *str, uint16 size)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_STRING);
		if (data != NULL)
		{
			data->string_data.size = size;
			data->string_data.mbstr = new uint8[size + 1];
			memcpy(data->string_data.mbstr, str, size);
			data->string_data.mbstr[size] = '\0';
		}
		return data;
	}

	Amf0Data *amf0String(const char *str)
	{
		return amf0StringNew((uint8 *)str, strlen(str));
	}

	uint16 amf0StringGetSize(Amf0Data *data)
	{
		if (data != NULL)
		{
			return data->string_data.size;
		}
		return 0;
	}

	uint8 *amf0StringGetUint8Ts(Amf0Data *data)
	{
		if (data != NULL)
		{
			return data->string_data.mbstr;
		}
		return NULL;
	}

	Amf0Data *amf0ArrayNew(void)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_STRICT_ARRAY);
		if (data != NULL)
		{
			data->array_data.size = 0;
			data->array_data.first_element = NULL;
			data->array_data.last_element = NULL;
			return data;
		}
		return NULL;
	}

	uint32 amf0ArraySize(Amf0Array *array)
	{
		return array != NULL ? array->size : 0;
	}

	Amf0Data *amf0ArrayPush(Amf0Array *array, Amf0Data *data)
	{
		if (array != NULL && data != NULL)
		{
			Amf0Node *node = new Amf0Node;
			if (node != NULL) {
				node->data = data;
				node->next = NULL;
				node->prev = NULL;
				if (array->size == 0) {
					array->first_element = node;
					array->last_element = node;
				}
				else
				{
					array->last_element->next = node;
					node->prev = array->last_element;
					array->last_element = node;
				}
				++(array->size);
				return data;
			}
		}
		return NULL;
	}

	Amf0Data *amf0ArrayPop(Amf0Array *array)
	{
		return amf0ArrayDelete(array, array->last_element);
	}

	Amf0Node *amf0ArrayFirst(Amf0Array *array)
	{
		return array != NULL ? array->first_element : NULL;
	}

	Amf0Node *amf0ArrayLast(Amf0Array *array)
	{
		return array != NULL ? array->last_element : NULL;
	}

	Amf0Node *amf0ArrayNext(Amf0Node *node)
	{
		return node != NULL ? node->next : NULL;
	}

	Amf0Node *amf0ArrayPrev(Amf0Node *node)
	{
		return node != NULL ? node->prev : NULL;
	}

	Amf0Data *amf0ArrayGet(Amf0Node *node)
	{
		return node != NULL ? node->data : NULL;
	}

	Amf0Data *amf0ArrayGetAt(Amf0Array *array, uint32 n)
	{
		Amf0Data *data = NULL;
		if (array != NULL && n < array->size)
		{
			Amf0Node *node = array->first_element;
			for (uint32 i = 0; i < n; ++i)
			{
				node = node->next;
			}
			data = node->data;
		}
		return data;
	}

	Amf0Data *amf0ArrayDelete(Amf0Array *array, Amf0Node *node)
	{
		if (array == NULL)
		{
			return NULL;
		}
		Amf0Data *data = NULL;
		if (node != NULL)
		{
			if (node->next != NULL)
			{
				node->next->prev = node->prev;
			}
			if (node->prev != NULL)
			{
				node->prev->next = node->next;
			}
			if (node == array->first_element)
			{
				array->first_element = node->next;
			}
			if (node == array->last_element)
			{
				array->last_element = node->prev;
			}
			data = node->data;
			delete node;
			--(array->size);
		}
		return data;
	}

	Amf0Data *amf0ArrayDeleteAt(Amf0Array *array, uint32 n)
	{
		Amf0Data *data = NULL;
		if (array != NULL && n < array->size)
		{
			Amf0Node *node = array->first_element;
			for (uint32 i = 0; i < n; ++i)
			{
				node = node->next;
			}
			data = amf0ArrayDelete(array, node);
		}
		return data;
	}

	Amf0Data *amf0ArrayInsertBefore(Amf0Array *array, Amf0Node *node, Amf0Data *element)
	{
		if (array != NULL && node != NULL && element != NULL)
		{
			Amf0Node * newNode = new Amf0Node;
			if (newNode != NULL)
			{
				newNode->data = element;
				newNode->next = node;
				newNode->prev = node->prev;

				if (node->prev != NULL)
				{
					node->prev->next = newNode;
					node->prev = newNode;
				}
				if (node == array->first_element)
				{
					array->first_element = newNode;
				}
				++(array->size);
				return element;
			}

		}
		return NULL;
	}

	Amf0Data *amf0ArrayInsertAfter(Amf0Array *array, Amf0Node *node, Amf0Data *element)
	{
		if (array != NULL && node != NULL && element != NULL)
		{
			Amf0Node *newNode = new Amf0Node;
			if (newNode != NULL)
			{
				newNode->data = element;
				newNode->next = node->next;
				newNode->prev = node;

				if (node->next != NULL)
				{
					node->next->prev = newNode;
					node->next = newNode;
				}
				if (node == array->last_element)
				{
					array->last_element = newNode;
				}
				++(array->size);
				return element;
			}
		}
		return NULL;
	}

	Amf0Data *amf0EcmaArrayNew()
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_ECMA_ARRAY);
		if (data != NULL)
		{
			data->array_data.size = 0;
			data->array_data.first_element = NULL;
			data->array_data.last_element = NULL;
		}
		return data;
	}

	Amf0Data *amf0ObjectNew(void)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_OBJECT);
		if (data != NULL)
		{
			data->array_data.size = 0;
			data->array_data.first_element = NULL;
			data->array_data.last_element = NULL;
		}
		return data;
	}

	uint32 amf0ObjectSize(Amf0Data *data)
	{
		return data != NULL ? data->array_data.size / 2 : 0;
	}

	Amf0Data *amf0ObjectAdd(Amf0Data *data, const char *name, Amf0Data *element)
	{
		if (amf0ArrayPush(&data->array_data, amf0String(name)) != NULL)
		{
			if (amf0ArrayPush(&data->array_data, element) != NULL)
			{
				return element;
			}
			else
			{
				amf0ArrayPop(&data->array_data);
			}
		}
		return NULL;
	}

	Amf0Data *amf0ObjectGet(Amf0Data *data, const char *name)
	{
		if (data != NULL)
		{
			//第一个为key，第二个为 value，第三个为key，第四个为value，依次类推
			Amf0Node *node = amf0ArrayFirst(&data->array_data);
			while (node != NULL)
			{
				uint16 size = node->data->string_data.size;
				//比较key
				if (size == strlen(name) &&
					strncmp((char *)node->data->string_data.mbstr, name, size) == 0)
				{
					//获取value
					node = node->next;
					return node != NULL ? node->data : NULL;
				}
				node = node->next->next;
			}
		}
		return NULL;
	}

	Amf0Data *amf0ObjectSet(Amf0Data *data, const char *name, Amf0Data *element)
	{
		if (data != NULL)
		{
			//第一个为key，第二个为 value，第三个为key，第四个为value，依次类推
			Amf0Node *node = amf0ArrayFirst(&data->array_data);
			while (node != NULL)
			{
				uint16 size = node->data->string_data.size;
				//比较key
				if (size == strlen(name) &&
					strncmp((char *)node->data->string_data.mbstr, name, size) == 0)
				{
					//获取value
					node = node->next;
					if (node != NULL && node->data != NULL)
					{
						amf0DataFree(node->data);
						node->data = element;
						return element;
					}
				}
				node = node->next->next;
			}
		}
		return NULL;
	}

	Amf0Data *amf0ObjectDelete(Amf0Data *data, const char *name)
	{
		if (data != NULL)
		{
			//第一个为key，第二个为 value，第三个为key，第四个为value，依次类推
			Amf0Node *node = amf0ArrayFirst(&data->array_data);
			while (node != NULL)
			{
				uint16 size = node->data->string_data.size;
				//比较key
				if (size == strlen(name) &&
					strncmp((char *)node->data->string_data.mbstr, name, size) == 0)
				{
					//获取value
					Amf0Node *dataNode = node->next; ;
					amf0DataFree(amf0ArrayDelete(&data->array_data, node));
					return amf0ArrayDelete(&data->array_data, dataNode);
				}
				node = node->next->next;
			}
		}
		return NULL;
	}

	Amf0Node *amf0ObjectFirst(Amf0Data *data)
	{
		return data != NULL ? amf0ArrayFirst(&data->array_data) : NULL;
	}

	Amf0Node *amf0ObjectLast(Amf0Data *data)
	{
		if (data != NULL)
		{
			Amf0Node *node = amf0ArrayLast(&data->array_data);
			if (node != NULL)
			{
				return node->prev;
			}
		}
		return NULL;
	}

	Amf0Node *amf0ObjectNext(Amf0Node *node)
	{
		if (node != NULL)
		{
			Amf0Node *next = node->next;
			if (next != NULL)
			{
				return next->next;
			}
		}
		return NULL;
	}

	Amf0Node *amf0ObjectPrev(Amf0Node *node)
	{
		if (node != NULL)
		{
			Amf0Node *prev = node->prev;
			if (prev != NULL)
			{
				return prev->prev;
			}
		}
		return NULL;
	}

	Amf0Data *amf0ObjectGetName(Amf0Node *node)
	{
		return node != NULL ? node->data : NULL;
	}

	Amf0Data *amf0ObjectGetData(Amf0Node *node)
	{
		if (node != NULL)
		{
			Amf0Node *dataNode = node->next;
			if (dataNode != NULL)
			{
				return dataNode->data;
			}
		}
		return NULL;
	}

	Amf0Data *amf0DateNew(uint64 milliseconds, int16 timezone)
	{
		Amf0Data *data = amf0DataNew(AMF0_TYPE_DATE);
		if (data != NULL)
		{
			data->date_data.milliseconds = milliseconds;
			data->date_data.timezone = timezone;
		}
		return data;
	}

	uint64 amf0DateGetMilliseconds(Amf0Data *data)
	{
		return data != NULL ? data->date_data.milliseconds : 0;
	}

	int16  amf0DateGetTimezone(Amf0Data *data)
	{
		return data != NULL ? data->date_data.timezone : 0;
	}

	time_t amf0Date2Time(Amf0Data *data)
	{
		return (time_t)(data != NULL ? data->date_data.milliseconds / 1000 : 0);
	}

	Amf0Block *amf0BlockNew(void)
	{
		Amf0Block *block = new Amf0Block;
		if (block != NULL)
		{
			block->array_data.size = 0;
			block->array_data.first_element = NULL;
			block->array_data.last_element = NULL;
		}
		return block;
	}

	Amf0Data *amf0BlockPush(Amf0Block *block, Amf0Data *data)
	{
		if (block != NULL && data != NULL)
		{
			return amf0ArrayPush(&block->array_data, data);
		}
		return NULL;
	}

	void amf0BlockRelease(Amf0Block *block)
	{
		if (block != NULL)
		{
			Amf0Node *node = amf0ArrayFirst(&block->array_data);
			while (node != NULL)
			{
				amf0DataFree(amf0ArrayDelete(&block->array_data, node));
				node = amf0ArrayFirst(&block->array_data);
			}
			delete block;
		}
	}

	uint32 amf0BlockSize(Amf0Block *block)
	{
		uint32 size = 0;
		if (block != NULL)
		{
			Amf0Node *node = amf0ArrayFirst(&block->array_data);
			while (node)
			{
				size += amf0DataSize(node->data);
				node = node->next;
			}
		}
		return size;
	}

	std::string amf0BlockDump(Amf0Block *block)
	{
		std::string strAmfData;
		if (block != NULL)
		{
			Amf0Node *node = amf0ArrayFirst(&block->array_data);
			while (node)
			{
				strAmfData.append(amf0DataDumpString(node->data, (char *)""));
				node = node->next;
			}
		}
		return strAmfData;
	}
	std::string amf0Block2String(Amf0Block *block)
	{
		std::string strAmfData;
		if (block != NULL)
		{
			Amf0Node *node = amf0ArrayFirst(&block->array_data);
			while (node)
			{
				strAmfData.append(amf0Data2String(node->data));
				node = node->next;
			}
		}
		return strAmfData;
	}

	Amf0Type amf0Data5Value(Amf0Data *data, const char *key, std::string &strValue)
	{
		Amf0Type type = AMF0_TYPE_NONE;
		if (data != NULL)
		{
			switch (data->type)
			{
			case AMF0_TYPE_OBJECT:
			case AMF0_TYPE_ECMA_ARRAY:
			{
				Amf0Node *nodeKey = amf0ObjectFirst(data);
				while (nodeKey != NULL)
				{
					if (strlen(key) == nodeKey->data->string_data.size &&
						strncmp((char *)nodeKey->data->string_data.mbstr, key, nodeKey->data->string_data.size) == 0)
					{
						Amf0Node *nodeValue = nodeKey->next;
						if (nodeValue->data->type == AMF0_TYPE_NUMERIC)
						{
							type = AMF0_TYPE_NUMERIC;
							char szNumber[30] = { 0 };
							sprintf(szNumber, "%f", nodeValue->data->number_data);
							strValue = szNumber;
							break;
						}
						else if (nodeValue->data->type == AMF0_TYPE_BOOLEAN)
						{
							type = AMF0_TYPE_BOOLEAN;
							if (nodeValue->data->boolean_data == 0x00)
							{
								strValue = "false";
							}
							else
							{
								strValue = "true";
							}
							break;
						}
						else if (nodeValue->data->type == AMF0_TYPE_STRING)
						{
							type = AMF0_TYPE_STRING;
							strValue = (char *)nodeValue->data->string_data.mbstr;
							break;
						}
						else if (nodeValue->data->type == AMF0_TYPE_DATE)
						{
							type = AMF0_TYPE_DATE;
							char szTimeZone[30] = { 0 };
							sprintf(szTimeZone, "%llu-%d", nodeValue->data->date_data.milliseconds, nodeValue->data->date_data.timezone);
							strValue = szTimeZone;
							break;
						}
						else if (nodeValue->data->type == AMF0_TYPE_OBJECT ||
							nodeValue->data->type == AMF0_TYPE_ECMA_ARRAY)
						{
							type = amf0Data5Value(nodeValue->data, key, strValue);
							if (type != AMF0_TYPE_NONE)
							{
								break;
							}
						}
					}
					nodeKey = amf0ObjectNext(nodeKey);
				}
			}
			break;
			default:
				break;
			}
		}
		return type;
	}

	Amf0Type amf0Block5Value(Amf0Block *block, const char *key, std::string &strValue)
	{
		Amf0Type type = AMF0_TYPE_NONE;
		if (block != NULL)
		{
			Amf0Node *node = amf0ArrayFirst(&block->array_data);
			while (node)
			{
				type = amf0Data5Value(node->data, key, strValue);
				if (type != AMF0_TYPE_NONE)
				{
					break;
				}
				node = node->next;
			}
		}
		return type;
	}

	Amf0Type amf0Block5Value(Amf0Block *block, int pos, std::string &strValue)
	{
		Amf0Type type = AMF0_TYPE_NONE;
		if (block != NULL)
		{
			Amf0Data *data = amf0ArrayGetAt(&block->array_data, pos);
			if (data != NULL)
			{
				if (data->type == AMF0_TYPE_NUMERIC)
				{
					type = AMF0_TYPE_NUMERIC;
					char szNumber[30] = { 0 };
					sprintf(szNumber, "%f", data->number_data);
					strValue = szNumber;
				}
				else if (data->type == AMF0_TYPE_BOOLEAN)
				{
					type = AMF0_TYPE_BOOLEAN;
					if (data->boolean_data == 0x00)
					{
						strValue = "false";
					}
					else
					{
						strValue = "true";
					}
				}
				else if (data->type == AMF0_TYPE_STRING)
				{
					type = AMF0_TYPE_STRING;
					strValue = (char *)data->string_data.mbstr;
				}
				else if (data->type == AMF0_TYPE_DATE)
				{
					type = AMF0_TYPE_DATE;
					char szTimeZone[30] = { 0 };
					sprintf(szTimeZone, "%llu-%d", data->date_data.milliseconds, data->date_data.timezone);
					strValue = szTimeZone;
				}
			}
		}
		return type;
	}

	void amf0BlockRemoveNode(Amf0Block *block, int pos)
	{
		if (block != NULL && pos >= 0)
		{
			amf0DataFree(amf0ArrayDeleteAt(&block->array_data, pos));
		}
	}

	Amf0Data *amf0BlockGetAmf0Data(Amf0Block *block, int pos)
	{
		Amf0Data *data = NULL;
		if (block && pos >= 0)
		{
			data = amf0ArrayGetAt(&block->array_data, pos);
		}
		return data;
	}

	char *amf0TypeParse(char *pData, int len, uint8 &type)
	{
		if (len < 1)
		{
			type = AMF0_TYPE_NONE;
			return pData + len;
		}
		type = *pData;
		return ++pData;
	}

	char *amf0NumberParse(char *pData, int len, Amf0Data **data)
	{
		if (len < 8)
		{
			*data = NULL;
			return pData += len;
		}
		char *bit = bit64Reversal(pData);
		double value = *((double*)bit);
		*data = amf0NumberNew(value);
		if (*data == NULL)
		{
			assert(true);
		}
		return pData + 8;
	}

	char *amf0BooleanParse(char *pData, int len, Amf0Data **data)
	{
		if (len < 1)
		{
			*data = NULL;
			return pData + len;
		}
		uint8 *pBoolean = (uint8 *)pData;
		*data = amf0BooleanNew(*pBoolean);
		if (*data == NULL)
		{
			assert(true);
		}
		return pData + 1;
	}

	char *amf0StringParse(char *pData, int len, Amf0Data **data)
	{
		//兼容 艾米范
		/*if (pData[0] == 0x09)
		{
			pData++;
			int len = *pData;
			pData++;
			pData += len;
		}*/
		if (len < 2)
		{
			*data = NULL;
			return pData + len;
		}
		int size = 0;
		uint16 ln = *((uint16*)pData);
		ln = ntohs(ln);
		size = sizeof(uint16) + ln;
		if (len < size)
		{
			*data = NULL;
			return pData + len;
		}
		*data = amf0StringNew((uint8*)(pData + sizeof(uint16)), ln);
		if (*data == NULL)
		{
			assert(true);
		}
		return pData + size;
	}

	char *amf0DateParse(char *pData, int len, Amf0Data **data)
	{
		if (len < 10)
		{
			*data = NULL;
			return pData + len;
		}
		pData = bit64Reversal(pData);
		uint64 minSeconds = *((double *)pData);
		pData += 8;
		uint16 zone = *((uint16 *)pData);
		zone = ntohs(zone);
		*data = amf0DateNew(minSeconds, zone);
		if (*data == NULL)
		{
			return pData + len;
		}
		pData += 2;
		return pData;
	}

	char *amf0ObjectParse(char *pData, int len, Amf0Data **ppObject);
	char *amf0EcmaArrayParse(char *pData, int len, Amf0Data **ppEcma);

	char *amf0StrictArrayParse(char *pData, int len, Amf0Data **ppArray)
	{
		if (len < 4)
		{
			*ppArray = NULL;
			return pData + len;
		}
		*ppArray = amf0ArrayNew();
		if (*ppArray == NULL)
		{
			*ppArray = NULL;
			return pData + len;
		}
		char *pEnd = pData + len;
		Amf0Data *array = *ppArray;
		int num = *((int *)pData);
		num = ntohl(num);
		uint8 type;
		pData += 4;
		for (int i = 0; i < num && pData < pEnd; ++i)
		{
			pData = amf0TypeParse(pData, pEnd - pData, type);
			switch (type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				Amf0Data *data = NULL;
				pData = amf0NumberParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				Amf0Data *data = NULL;
				pData = amf0BooleanParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_STRING:
			{
				Amf0Data *data = NULL;
				pData = amf0StringParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				Amf0Data *data = NULL;
				pData = amf0ObjectParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_MOVIECLIP:
			{
				logs->info("+++ amf0StrictArray UnHandle MovieClip[ %d ] +++", AMF0_TYPE_MOVIECLIP);
				return pEnd;
			}
			break;
			case AMF0_TYPE_NULL:
			{
				logs->info("+++ amf0StrictArray ignore NULL[ %d ] +++", AMF0_TYPE_NULL);
				Amf0Data *data = amf0::amf0NullNew();
				if (data != NULL)
				{
					amf0ArrayPush(&array->array_data, data);
				}
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				logs->info("+++ amf0StrictArray ignore UnDefined[ %d ] +++", AMF0_TYPE_UNDEFINED);
				//++pData;
			}
			break;
			case AMF0_TYPE_REFERENCE:
			{
				logs->info("+++ amf0StrictArray UnHandle Reference[ %d ] +++", AMF0_TYPE_REFERENCE);
				return pEnd;
			}
			break;
			case AMF0_TYPE_ECMA_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0EcmaArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0StrictArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_DATE:
			{
				Amf0Data *data = NULL;
				pData = amf0DateParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					return pEnd;
				}
				amf0ArrayPush(&array->array_data, data);
			}
			break;
			case AMF0_TYPE_LONG_STRING:
			{
				logs->info("+++ amf0StrictArray UnHandle LongString[ %d ] +++", AMF0_TYPE_LONG_STRING);
				return pEnd;
			}
			break;
			case AMF0_TYPE_UNSUPPORTED:
			{
				logs->info("+++ amf0StrictArray UnHandle UnSupported[ %d ] +++", AMF0_TYPE_UNSUPPORTED);
				return pEnd;
			}
			break;
			case AMF0_TYPE_RECORD_SET:
			{
				logs->info("+++ amf0StrictArray UnHandle RecordSet[ %d ] +++", AMF0_TYPE_RECORD_SET);
				return pEnd;
			}
			break;
			case AMF0_TYPE_XML_OBJECT:
			{
				logs->info("+++ amf0StrictArray UnHandle XmlObject[ %d ] +++", AMF0_TYPE_XML_OBJECT);
				return pEnd;
			}
			break;
			case AMF0_TYPE_TYPED_OBJECT:
			{
				logs->info("+++ amf0StrictArray UnHandle TypedObject[ %d ] +++", AMF0_TYPE_TYPED_OBJECT);
				return pEnd;
			}
			break;
			case AMF0_TYPE_AMF3:
			{
				logs->info("+++ amf0StrictArray UnHandle AMF3[ %d ] +++", AMF0_TYPE_AMF3);
				return pEnd;
			}
			break;
			default:
			{
				logs->info("+++ amf0StrictArray UnHandle UnKnow Type[ %d ] +++", type);
				return pEnd;
			}
			break;
			}
		}
		return pData;
	}

	char *amf0EcmaArrayParse(char *pData, int len, Amf0Data **ppEcma)
	{
		if (len < 4)
		{
			*ppEcma = NULL;
			return pData + len;
		}
		*ppEcma = amf0EcmaArrayNew();
		if (*ppEcma == NULL)
		{
			assert(true);
		}
		char *pEnd = pData + len;
		Amf0Data *ecma = *ppEcma;
		bool bFinish = false;
		uint8 type;
		//跳过 ecma array 长度，该长度有时候会出现0情况
		pData += 4;

		do
		{
			//key 一定是string类型
			Amf0Data *dataString = NULL;
			pData = amf0StringParse(pData, pEnd - pData, &dataString);
			if (dataString == NULL)
			{
				return pEnd;
			}
			//value
			pData = amf0TypeParse(pData, pEnd - pData, type);
			switch (type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				Amf0Data *data = NULL;
				pData = amf0NumberParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				Amf0Data *data = NULL;
				pData = amf0BooleanParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_STRING:
			{
				Amf0Data *data = NULL;
				pData = amf0StringParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				Amf0Data *data = NULL;
				pData = amf0ObjectParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_MOVIECLIP:
			{
				logs->info("+++ amf0EcmaArray UnHandle MovieClip[ %d ] +++", AMF0_TYPE_MOVIECLIP);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_NULL:
			{
				logs->info("+++ amf0EcmaArray ignore NULL[ %d ] +++", AMF0_TYPE_NULL);
				Amf0Data *data = amf0::amf0NullNew();
				if (data != NULL)
				{
					amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
				}
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				logs->info("+++ amf0EcmaArray ignore UnDefined[ %d ] +++", AMF0_TYPE_UNDEFINED);
				//++pData;
			}
			break;
			case AMF0_TYPE_REFERENCE:
			{
				logs->info("+++ amf0EcmaArray UnHandle Reference[ %d ] +++", AMF0_TYPE_REFERENCE);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_ECMA_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0EcmaArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0StrictArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_DATE:
			{
				Amf0Data *data = NULL;
				pData = amf0DateParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(ecma, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_LONG_STRING:
			{
				logs->info("+++ amf0EcmaArray UnHandle LongString[ %d ] +++", AMF0_TYPE_LONG_STRING);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_UNSUPPORTED:
			{
				logs->info("+++ amf0EcmaArray UnHandle UnSupported[ %d ] +++", AMF0_TYPE_UNSUPPORTED);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_RECORD_SET:
			{
				logs->info("+++ amf0EcmaArray UnHandle RecordSet[ %d ] +++", AMF0_TYPE_RECORD_SET);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_XML_OBJECT:
			{
				logs->info("+++ amf0EcmaArray UnHandle XmlObject[ %d ] +++", AMF0_TYPE_XML_OBJECT);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_TYPED_OBJECT:
			{
				logs->info("+++ amf0EcmaArray UnHandle TypedObject[ %d ] +++", AMF0_TYPE_TYPED_OBJECT);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_AMF3:
			{
				logs->info("+++ amf0EcmaArray UnHandle AMF3[ %d ] +++", AMF0_TYPE_AMF3);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			default:
			{
				logs->info("+++ amf0EcmaArray UnHandle UnKnow Type[ %d ] +++", type);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			}
			if (pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x09)
			{
				//object 结束
				pData += 3;
				bFinish = true;
			}
			amf0DataFree(dataString);
		} while (!bFinish && pData < pEnd);
		return pData;
	}

	char *amf0ObjectParse(char *pData, int len, Amf0Data **ppObject)
	{
		*ppObject = amf0ObjectNew();
		if (*ppObject == NULL)
		{
			*ppObject = NULL;
			return pData + len;
		}
		Amf0Data *object = *ppObject;
		bool bFinish = false;
		uint8 type;
		char *pEnd = pData + len;
		do
		{
			//key 一定是string类型
			Amf0Data *dataString = NULL;
			pData = amf0StringParse(pData, pEnd - pData, &dataString);
			if (dataString == NULL)
			{
				return pEnd;
				//assert(true);
			}
			//value
			pData = amf0TypeParse(pData, pEnd - pData, type);
			switch (type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				Amf0Data *data = NULL;
				pData = amf0NumberParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				Amf0Data *data = NULL;
				pData = amf0BooleanParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_STRING:
			{
				Amf0Data *data = NULL;
				pData = amf0StringParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				Amf0Data *data = NULL;
				pData = amf0ObjectParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_MOVIECLIP:
			{
				logs->info("+++ amf0ObjectParse UnHandle MovieClip[ %d ] +++", AMF0_TYPE_MOVIECLIP);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_NULL:
			{
				logs->info("+++ amf0ObjectParse ignore NULL[ %d ] +++", AMF0_TYPE_NULL);
				Amf0Data *data = amf0::amf0NullNew();
				if (data != NULL)
				{
					amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
				}
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				logs->info("+++ amf0ObjectParse ignore UnDefined[ %d ] +++", AMF0_TYPE_UNDEFINED);
				//++pData;
			}
			break;
			case AMF0_TYPE_REFERENCE:
			{
				logs->info("+++ amf0ObjectParse UnHandle Reference[ %d ] +++", AMF0_TYPE_REFERENCE);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_ECMA_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0EcmaArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0StrictArrayParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_DATE:
			{
				Amf0Data *data = NULL;
				pData = amf0DateParse(pData, pEnd - pData, &data);
				if (data == NULL)
				{
					amf0DataFree(dataString);
					return pEnd;
				}
				amf0ObjectAdd(object, (char *)dataString->string_data.mbstr, data);
			}
			break;
			case AMF0_TYPE_LONG_STRING:
			{
				logs->info("+++ amf0ObjectParse UnHandle LongString[ %d ] +++", AMF0_TYPE_LONG_STRING);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_UNSUPPORTED:
			{
				logs->info("+++ amf0ObjectParse UnHandle UnSupported[ %d ] +++", AMF0_TYPE_UNSUPPORTED);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_RECORD_SET:
			{
				logs->info("+++ amf0ObjectParse UnHandle RecordSet[ %d ] +++", AMF0_TYPE_RECORD_SET);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_XML_OBJECT:
			{
				logs->info("+++ amf0ObjectParse UnHandle XmlObject[ %d ] +++", AMF0_TYPE_XML_OBJECT);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_TYPED_OBJECT:
			{
				logs->info("+++ amf0ObjectParse UnHandle TypedObject[ %d ] +++", AMF0_TYPE_TYPED_OBJECT);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			case AMF0_TYPE_AMF3:
			{
				logs->info("+++ amf0ObjectParse UnHandle AMF3[ %d ] +++", AMF0_TYPE_AMF3);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			default:
			{
				logs->info("+++ amf0ObjectParse UnHandle UnKnow Type[ %d ] +++", type);
				amf0DataFree(dataString);
				return pEnd;
			}
			break;
			}
			if (pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x09)
			{
				//object 结束
				pData += 3;
				bFinish = true;
			}
			amf0DataFree(dataString);
		} while (!bFinish && pData < pEnd);
		return pData;
	}

	Amf0Block *amf0Parse(char *buf, int len)
	{
		if (buf == NULL || len <= 0)
		{
			return NULL;
		}
		Amf0Block *afm0Block = amf0BlockNew();
		char *pData = buf;
		char *pEnd = buf + len;
		uint8 type = 0;
		bool bGetCmd = false;
		while (pData < pEnd)
		{
			pData = amf0TypeParse(pData, pEnd - pData, type);
			switch (type)
			{
			case AMF0_TYPE_NUMERIC:
			{
				Amf0Data *data = NULL;
				pData = amf0NumberParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_BOOLEAN:
			{
				Amf0Data *data = NULL;
				pData = amf0BooleanParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_STRING:
			{
				Amf0Data *data = NULL;
				pData = amf0StringParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
					if (bGetCmd == false)
					{
						afm0Block->cmd = (char *)data->string_data.mbstr;
						//std::transform(afm0Block->cmd.begin(), afm0Block->cmd.end(), afm0Block->cmd.begin(), tolower);
					}
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_OBJECT:
			{
				Amf0Data *data = NULL;
				pData = amf0ObjectParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_MOVIECLIP:
			{
				logs->info("+++ amf0Parse UnHandle MovieClip[ %d ] +++", AMF0_TYPE_MOVIECLIP);
				goto END;
			}
			break;
			case AMF0_TYPE_NULL:
			{
				Amf0Data *data = amf0::amf0NullNew();
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_UNDEFINED:
			{
				logs->info("+++ amf0Parse UnHandle UnDefined[ %d ] +++", AMF0_TYPE_UNDEFINED);
				//++pData;
			}
			break;
			case AMF0_TYPE_REFERENCE:
			{
				logs->info("+++ amf0Parse UnHandle Reference[ %d ] +++", AMF0_TYPE_REFERENCE);
				goto END;
			}
			break;
			case AMF0_TYPE_ECMA_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0EcmaArrayParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_OBJECT_END:
			{
				goto END;
			}
			break;
			case AMF0_TYPE_STRICT_ARRAY:
			{
				Amf0Data *data = NULL;
				pData = amf0StrictArrayParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_DATE:
			{
				Amf0Data *data = NULL;
				pData = amf0DateParse(pData, pEnd - pData, &data);
				if (data != NULL)
				{
					amf0BlockPush(afm0Block, data);
				}
				else
				{
					goto END;
				}
			}
			break;
			case AMF0_TYPE_LONG_STRING:
			{
				logs->info("+++ amf0Parse UnHandle LongString[ %d ] +++", AMF0_TYPE_LONG_STRING);
				goto END;
			}
			break;
			case AMF0_TYPE_UNSUPPORTED:
			{
				logs->info("+++ amf0Parse UnHandle UnSupported[ %d ] +++", AMF0_TYPE_UNSUPPORTED);
				goto END;
			}
			break;
			case AMF0_TYPE_RECORD_SET:
			{
				logs->info("+++ amf0Parse UnHandle RecordSet[ %d ] +++", AMF0_TYPE_RECORD_SET);
				goto END;
			}
			break;
			case AMF0_TYPE_XML_OBJECT:
			{
				logs->info("+++ amf0Parse UnHandle XmlObject[ %d ] +++", AMF0_TYPE_XML_OBJECT);
				goto END;
			}
			break;
			case AMF0_TYPE_TYPED_OBJECT:
			{
				logs->info("+++ amf0Parse UnHandle TypedObject[ %d ] +++", AMF0_TYPE_TYPED_OBJECT);
				goto END;
			}
			break;
			case AMF0_TYPE_AMF3:
			{
				logs->info("+++ amf0Parse UnHandle AMF3[ %d ] +++", AMF0_TYPE_AMF3);
				goto END;
			}
			break;
			default:
			{
				goto END;
			}
			break;
			}
			bGetCmd = true;
		}
	END:
		return afm0Block;
	}

	void amf0ListClone(Amf0Array *array, Amf0Array *outArray)
	{
		Amf0Node *node;
		node = array->first_element;
		while (node != NULL)
		{
			amf0ArrayPush(outArray, amf0DataClone(node->data));
			node = node->next;
		}
	}

	Amf0Data *amf0DataClone(Amf0Data * data)
	{
		if (data != NULL) {
			switch (data->type)
			{
			case AMF0_TYPE_NUMERIC:
				return amf0NumberNew(amf0NumberGetValue(data));
			case AMF0_TYPE_BOOLEAN:
				return amf0BooleanNew(amf0BooleanGetValue(data));
			case AMF0_TYPE_STRING:
				if (data->string_data.mbstr != NULL)
				{
					return amf0StringNew((uint8_t *)amf0StringGetUint8Ts(data), amf0StringGetSize(data));
				}
				else {
					return amf0StringNew(NULL, 0);;
				}
			case AMF0_TYPE_NULL:
				return amf0NullNew();
			case AMF0_TYPE_UNDEFINED:
				return amf0UndefinedNew();
				/*case AMF0_TYPE_REFERENCE:*/
			case AMF0_TYPE_OBJECT:
			case AMF0_TYPE_ECMA_ARRAY:
			case AMF0_TYPE_STRICT_ARRAY:
			{
				Amf0Data *ad = amf0DataNew(data->type);
				if (ad != NULL)
				{
					ad->array_data.size = 0;
					ad->array_data.first_element = NULL;
					ad->array_data.last_element = NULL;
					amf0ListClone(&data->array_data, &ad->array_data);
				}
				return ad;
			}
			case AMF0_TYPE_DATE:
				return amf0DateNew(amf0DateGetMilliseconds(data), amf0DateGetTimezone(data));
				/*case AMF0_TYPE_SIMPLEOBJECT:*/
			case AMF0_TYPE_TYPED_OBJECT:
				return NULL;
			}
		}
		return NULL;
	}
}
