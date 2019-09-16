#pragma once
#include <string>
#include <vector>
#include <list>
#include <string.h>
#include <iostream>

using namespace std;

#define LOGE(x) cout << x << endl;
#define LOGF(x) cout << x << endl;
#define LOGW(x) cout << x << endl;
#define LOGT(x) cout << x << endl;

enum AMFDataType
{
	AMF_number,
	AMF_boolean,
	AMF_string,
	AMF_object,
	AMF_movieclip,
	AMF_null,
	AMF_undefined,
	AMF_reference,
	AMF_ecma_array,	//个数 ，《名字，值》，end
	AMF_object_end,
	AMF_strict_array,//个数，《值》，end
	AMF_date,
	AMF_long_string,
	AMF_unsupported,
	AMF_recordset,
	AMF_xml_document,
	AMF_typed_object,
	AMF_avmplus_object,
};
//最外层的obj里没有名字，只有prop内的obj有名字，和ecma array是键值对
struct AMFObjectProperty;
//只能释放一最外层的那个.都是浅复制

struct AMFObject
{
	AMFObject();
	AMFObject(const AMFObject&);
	AMFObject &operator =(const AMFObject&);
	std::list<AMFObjectProperty*> objectProps;
};

struct AMFObjectData
{
	AMFObjectData();
	AMFObjectData(const AMFObjectData&) ;
	AMFObjectData & operator =(const AMFObjectData &);
	~AMFObjectData();
	double number;
	std::string strValue;
	AMFObject object;
};

struct AMFObjectProperty
{
	AMFObjectProperty() ;
	AMFObjectProperty(const AMFObjectProperty&);
	AMFObjectProperty& operator=(const AMFObjectProperty&);
	~AMFObjectProperty();
	std::string name;
	AMFDataType type;
	short UTCoffset;
	AMFObjectData vu;
};
void AMF_Clear(AMFObject *obj);
void AMF_PropClear(AMFObjectProperty *prop);


//amf解码
unsigned short AMF_DecodeInt16(const char *data);
unsigned int AMF_DecodeInt24(const char *data);
unsigned int AMF_DecodeInt32(const char *data);
unsigned int AMF_DecodeInt24LE(const char *data);
unsigned int AMF_DecodeInt32LE(const char* data);
double AMF_DecodeNumber(const char*data);
bool AMD_DecodeBool(const char *data);
void AMF_DecodeString(const char *data, std::string &strValue);
void AMF_DecodeLongString(const char *data, std::string &strValue);

int AMF_Decode(AMFObject *obj, char *buffer, int bufferSize, bool decodeName);
int AMF_PropDecode(AMFObjectProperty *prop, char *buffer, int bufferSize, bool decodeName);
int AMF_ArrayDecode(AMFObject *obj, char *buffer, int bufferSize, int arrayLen, bool decodeName);
void AMF_AddProp(AMFObject *obj, AMFObjectProperty *prop);
AMFObjectProperty *AMF_GetPropByIndex(AMFObject *obj, int index);
