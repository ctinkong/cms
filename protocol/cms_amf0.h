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
#ifndef __AMF_0_H__
#define __AMF_0_H__
#include <string>

namespace amf0
{
	enum Amf0Type
	{
		AMF0_TYPE_NONE = -1,
		//以下是标准类型
		AMF0_TYPE_NUMERIC = 0x00,
		AMF0_TYPE_BOOLEAN,
		AMF0_TYPE_STRING,
		AMF0_TYPE_OBJECT,
		AMF0_TYPE_MOVIECLIP,
		AMF0_TYPE_NULL,
		AMF0_TYPE_UNDEFINED,
		AMF0_TYPE_REFERENCE,
		AMF0_TYPE_ECMA_ARRAY,
		AMF0_TYPE_OBJECT_END,
		AMF0_TYPE_STRICT_ARRAY,
		AMF0_TYPE_DATE,
		AMF0_TYPE_LONG_STRING,
		AMF0_TYPE_UNSUPPORTED,
		AMF0_TYPE_RECORD_SET,
		AMF0_TYPE_XML_OBJECT,
		AMF0_TYPE_TYPED_OBJECT,
		AMF0_TYPE_AMF3
	};
}

namespace amf0
{
	typedef unsigned char	 uint8;
	typedef short			 int16;
	typedef unsigned short	 uint16;
	typedef unsigned int	 uint32;
	typedef unsigned long long uint64;
	typedef struct __Amf0Node *pAmf0Node;
	struct __Amf0Data;

	/* string 类型 */
	typedef struct __Amf0String {
		uint16 size;
		uint8 *mbstr;
	} Amf0String;

	/* array MixedArray 类型 */
	//当表示 MixedArray 时,first_element 第一个元素是 key,第二个是 value，第三个是 key，第四个是vaule，依次类推
	//长度为 size / 2
	typedef struct __Amf0Array {
		uint32 size;
		pAmf0Node first_element;
		pAmf0Node last_element;
	} Amf0Array;

	/* 链表节点 */
	typedef struct __Amf0Node {
		__Amf0Data *data;
		pAmf0Node next;
		pAmf0Node prev;
	} Amf0Node;

	/* date 类型 */
	typedef struct __Amf0Date {
		uint64 milliseconds;
		uint16 timezone;
	} Amf0Date;

	/* XML string 类型 */
	typedef struct __Amf0XmlString {
		uint32 size;
		uint8 *mbstr;
	} Amf0XmlString;

	/* class 类型 */
	typedef struct __Amf0Class {
		Amf0String name;
		Amf0Array elements;
	} Amf0Class;

	/* objects 类型 */
	typedef struct __Amf0Data {
		uint8 type;
		union {
			double	number_data;
			uint8	boolean_data;
			Amf0String string_data;
			Amf0Array	array_data;
			Amf0Date	date_data;
			Amf0XmlString xmlstring_data;
			Amf0Class	class_data;
		};
	} Amf0Data;

	/* amf0 数据块 */
	typedef struct __Amf0Block
	{
		Amf0Array	array_data;
		std::string cmd; //解析amf0数据块时，用于获取操作类型，生成数据块时，忽略
	}Amf0Block;

	//翻左内存
	char		*bit64Reversal(char *bit);
	Amf0Data	*amf0DataNew(uint8 type);
	//获取内存字节数
	uint32		amf0DataSize(Amf0Data *data);
	uint8		amf0DataGetType(Amf0Data *data);
	//Amf0Data	*amf0DataClone(Amf0Data *data);
	void		amf0DataFree(Amf0Data *data);

	/* 将amf0节点生成可视化字符串 */
	std::string amf0DataDumpString(Amf0Data *data, char *retract, bool key = false);
	/* 将amf0节点生成amf0格式字符串 */
	std::string amf0Data2String(Amf0Data *data, bool key = false);

	/* null 类型 functions */
	Amf0Data	*amf0NullNew();
	Amf0Data    *amf0UndefinedNew();

	/* number 类型 functions */
	Amf0Data	*amf0NumberNew(double value);
	double		amf0NumberGetValue(Amf0Data *data);
	void		amf0NumberSetValue(Amf0Data *data, double value);

	/* boolean 类型 functions */
	Amf0Data	*amf0BooleanNew(uint8 value);
	uint8		amf0BooleanGetValue(Amf0Data *data);
	void		amf0BooleanSetValue(Amf0Data *data, uint8 value);

	/* string 类型 functions */
	Amf0Data	*amf0StringNew(uint8 *str, uint16 size);
	Amf0Data	*amf0String(const char *str);
	uint16		amf0StringGetSize(Amf0Data *data);
	uint8		*amf0StringGetUint8Ts(Amf0Data *data);

	/* object 类型 functions */
	Amf0Data    *amf0EcmaArrayNew();
	Amf0Data	*amf0ObjectNew(void);
	uint32		amf0ObjectSize(Amf0Data *data);
	Amf0Data	*amf0ObjectAdd(Amf0Data *data, const char *name, Amf0Data *element);
	Amf0Data	*amf0ObjectGet(Amf0Data *data, const char *name);
	Amf0Data	*amf0ObjectSet(Amf0Data *data, const char *name, Amf0Data *element);
	Amf0Data	*amf0ObjectDelete(Amf0Data *data, const char *name);
	Amf0Node	*amf0ObjectFirst(Amf0Data *data);
	Amf0Node	*amf0ObjectLast(Amf0Data *data);
	Amf0Node	*amf0ObjectNext(Amf0Node *node);
	Amf0Node	*amf0ObjectPrev(Amf0Node *node);
	Amf0Data	*amf0ObjectGetName(Amf0Node *node);
	Amf0Data	*amf0ObjectGetData(Amf0Node *node);

	/* array 类型 functions */
	Amf0Data	*amf0ArrayNew(void);
	uint32		amf0ArraySize(Amf0Array *array);
	Amf0Data	*amf0ArrayPush(Amf0Array *array, Amf0Data *data);
	Amf0Data	*amf0ArrayPop(Amf0Array *array);
	Amf0Node	*amf0ArrayFirst(Amf0Array *array);
	Amf0Node	*amf0ArrayLast(Amf0Array *array);
	Amf0Node	*amf0ArrayNext(Amf0Node *node);
	Amf0Node	*amf0ArrayPrev(Amf0Node *node);
	Amf0Data	*amf0ArrayGet(Amf0Node *node);
	Amf0Data	*amf0ArrayGetAt(Amf0Array *array, uint32 n);
	Amf0Data	*amf0ArrayDelete(Amf0Array *array, Amf0Node *node);
	Amf0Data    *amf0ArrayDeleteAt(Amf0Array *array, uint32 n);
	Amf0Data	*amf0ArrayInsertBefore(Amf0Array *array, Amf0Node *node, Amf0Data *element);
	Amf0Data	*amf0ArrayInsertAfter(Amf0Array *array, Amf0Node *node, Amf0Data *element);

	/* date 类型 functions */
	Amf0Data	*amf0DateNew(uint64 milliseconds, int16 timezone);
	uint64		amf0DateGetMilliseconds(Amf0Data *data);
	int16		amf0DateGetTimezone(Amf0Data *data);
	time_t		amf0Date2Time(Amf0Data *data);

	/* amf 数据块 functions */
	Amf0Block   *amf0BlockNew(void);
	void		amf0BlockRelease(Amf0Block *block);
	Amf0Data	*amf0BlockPush(Amf0Block *block, Amf0Data *data);
	uint32		amf0BlockSize(Amf0Block *block);

	//把amf0数据生成可视化字符串
	std::string amf0BlockDump(Amf0Block *block);
	//把amf0数据填充成amf0格式
	std::string amf0Block2String(Amf0Block *block);
	/*
		通过key查找value，会遍历整个amf数据块，直到找到key或遍历完毕。
		返回值表示value的类型(返回值只能是:Numeric、Boolean、String、Date,
		Date将以minseconds-zone形式返回,Boolean将以true，false形式返回
		如果其它类型:Object、Array、MixedArray,则会继续往下遍历)，
		值会以string的形式保存在strValue中
	*/
	Amf0Type amf0Block5Value(Amf0Block *block, const char *key, std::string &strValue);
	/*
		通过位置pos索引获取value，只会遍历block的array链表，不会进行递归遍历。
		返回值表示value的类型(返回值只能是:Numeric、Boolean、String、Date,
		Date将以minseconds-zone形式返回,Boolean将以true，false形式返回
		如果其它类型:Object、Array、MixedArray,strValue返回为空，函数返回值为 AMF0_TYPE_NONE)，
		值会以string的形式保存在strValue中
	*/
	Amf0Type amf0Block5Value(Amf0Block *block, int pos, std::string &strValue);

	void	 amf0BlockRemoveNode(Amf0Block *block, int pos);
	Amf0Data *amf0BlockGetAmf0Data(Amf0Block *block, int pos);

	Amf0Block   *amf0Parse(char *buf, int len);

	Amf0Data *amf0DataClone(Amf0Data * data);
}
#endif