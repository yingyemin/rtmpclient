#include "amfdecoder.h"
#include <string.h>

void AMF_Clear(AMFObject * obj)
{
	if (!obj)
	{
		return;
	}
	for (auto i:obj->objectProps )
	{
		AMF_PropClear(i);
		delete i;
	}
	obj->objectProps.clear();
}

void AMF_PropClear(AMFObjectProperty * prop)
{
	if (!prop)
	{
		return;
	}
	AMF_Clear(&prop->vu.object);
}

unsigned short AMF_DecodeInt16(const char * data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned short val;
	val = (c[0] << 8) | c[1];
	return val;
}

unsigned int AMF_DecodeInt24(const char * data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned int val;
	val = (c[0] << 16) | (c[1] << 8) | c[2];
	return val;
}

unsigned int AMF_DecodeInt32(const char * data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned int val;
	val = (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
	return val;
}

unsigned int AMF_DecodeInt24LE(const char * data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned int val;

	val = (c[2] << 16) | (c[1] << 8) | c[0];
	return val;
}

unsigned int AMF_DecodeInt32LE(const char * data)
{
	unsigned char *c = (unsigned char *)data;
	unsigned int val;

	val = (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
	return val;
}

double AMF_DecodeNumber(const char * data)
{
	double dVal;
#if BYTE_ORDER==LITTLE_ENDIAN
	unsigned char *ci, *co;
	ci = (unsigned char *)data;
	co = (unsigned char *)&dVal;
	co[0] = ci[7];
	co[1] = ci[6];
	co[2] = ci[5];
	co[3] = ci[4];
	co[4] = ci[3];
	co[5] = ci[2];
	co[6] = ci[1];
	co[7] = ci[0];
#else	
	memcpy(&dVal, data, 8);
#endif
	return dVal;
}

bool AMD_DecodeBool(const char * data)
{
	return (*data != 0);
}

void AMF_DecodeString(const char * data, std::string & strValue)
{
	strValue;
	strValue.resize(AMF_DecodeInt16(data));
	if (strValue.size()>0)
	{
		memcpy((void*)strValue.c_str(), data + 2, strValue.size());
	}
}

void AMF_DecodeLongString(const char * data, std::string & strValue)
{
	strValue.resize(AMF_DecodeInt32(data));
	if (strValue.size()>0)
	{
		memcpy((void*)strValue.c_str(), data + 4, strValue.size());
	}
}

int AMF_Decode(AMFObject * obj, char * buffer, int bufferSize, bool decodeName)
{
	int nSize = bufferSize;
	int cur = 0;
	bool err = false;
	while (nSize>0)
	{
		if (3<=nSize&&AMF_object_end == AMF_DecodeInt24(buffer + cur))
		{
			nSize -= 3;
			cur += 3;
			err = false;
			break;
		}
		if (err)
		{
			nSize--;
			cur++;
		}
		AMFObjectProperty prop;
		int iRet = AMF_PropDecode(&prop, buffer + cur, nSize, decodeName);
		if (iRet<0)
		{
			err = true;
		}
		else
		{
			nSize -= iRet;
			cur += iRet;
			AMF_AddProp(obj, &prop);
		}
	}
	if (err)
	{
		return -1;
	}
	return cur;
}

int AMF_PropDecode(AMFObjectProperty * prop, char * buffer, int bufferSize, bool decodeName)
{
	int nSize = bufferSize;
	if (0==nSize||buffer==nullptr)
	{
		LOGF("invalid param for prop decode");
		return -1;
	}
	if (decodeName&&nSize<4)
	{
		LOGE("no enough data for decode name");
		return -1;
	}
	if (decodeName)
	{
		unsigned short nameSize = AMF_DecodeInt16(buffer);
		if (nameSize>nSize-2)
		{
			LOGE("name size > buffer size,something is wrong");
			return -1;
		}
		AMF_DecodeString(buffer, prop->name);
		nSize -= 2 + nameSize;
		buffer += 2 + nameSize;
	}
	if (nSize==0)
	{
		return -1;
	}
	prop->type = (AMFDataType)*buffer;
	nSize--;
	buffer++;
	unsigned short tmp16 = 0;
	unsigned int tmp32 = 0;
	int iRet = 0;
	switch (prop->type)
	{
	case AMF_number:
		if (nSize<8)
		{
			return -1;
		}
		prop->vu.number = AMF_DecodeNumber(buffer);
		nSize -= 8;
		break;
	case AMF_boolean:
		if (nSize<1)
		{
			return -1;
		}
		prop->vu.number = (double)AMD_DecodeBool(buffer);
		nSize--;
		break;
	case AMF_string:
		tmp16 = AMF_DecodeInt16(buffer);
		if (tmp16 + 2>nSize)
		{
			return -1;
		}
		AMF_DecodeString(buffer, prop->vu.strValue);
		nSize -= (tmp16 + 2);
		break;
	case AMF_object:
		iRet = AMF_Decode(&prop->vu.object, buffer, nSize, true);
		if (-1 == iRet)
		{
			return -1;
		}
		nSize -= iRet;
		break;
	case AMF_movieclip:
		return -1;
		LOGW("this amf type:" << prop->type << " This type is not supported and is reserved for future use");
		break;
	case AMF_null:
		//null is no data
		break;
	case AMF_undefined:
		LOGT("this amf type:" << prop->type << " The undefined type is represented by the undefined type marker. \
No further information is encoded for this value.");
		prop->type = AMF_null;
		break;
	case AMF_reference:
		LOGW("this amf type:" << prop->type << " not processed");
		return -1;
		break;
	case AMF_ecma_array:
		//忽略个数
		nSize -= 4;
		buffer += 4;
		iRet = AMF_Decode(&prop->vu.object,buffer,nSize, true);
		if (-1==iRet)
		{
			return -1;
		}
		nSize -= iRet;
		break;
	case AMF_object_end:
		return -1;
		break;
	case AMF_strict_array:
		tmp32 = AMF_DecodeInt32(buffer);
		nSize -= 4;
		buffer += 4;
		iRet = AMF_ArrayDecode(&prop->vu.object, buffer, nSize, tmp32, false);
		if (-1==iRet)
		{
			return -1;
		}
		nSize -= iRet;
		break;
	case AMF_date:
		if (nSize<10)
		{
			return -1;
		}
		prop->vu.number = AMF_DecodeNumber(buffer);
		prop->UTCoffset = AMF_DecodeInt16(buffer + 8);
		nSize -= 10;
		break;
	case AMF_long_string:
		tmp32 = AMF_DecodeInt32(buffer);
		if (nSize<tmp32 + 4)
		{
			return -1;
		}
		AMF_DecodeLongString(buffer, prop->vu.strValue);
		nSize -= 4 + tmp32;
		break;
	case AMF_unsupported:
		LOGW("this amf type:" << prop->type << " not processed");
		prop->type = AMF_null;
		break;
	case AMF_recordset:
		LOGW("this amf type:" << prop->type << " not processed");
		return -1;
		break;
	case AMF_xml_document:
		tmp32 = AMF_DecodeInt32(buffer);
		if (nSize<tmp32 + 4)
		{
			return -1;
		}
		AMF_DecodeLongString(buffer, prop->vu.strValue);
		nSize -= 4 + tmp32;
		break;
	case AMF_typed_object:
		LOGW("this amf type:" << prop->type << " not processed");
		return -1;
		break;
	case AMF_avmplus_object:
		LOGW("this amf type:" << prop->type << " not processed");
		break;
	default:
		LOGW("this amf type:" << prop->type << " not processed");
		break;
	}
	return bufferSize - nSize;
}

int AMF_ArrayDecode(AMFObject * obj, char * buffer, int bufferSize, int arrayLen, bool decodeName)
{
	int nSize = bufferSize;
	bool err = false;
	while (arrayLen>0)
	{
		arrayLen--;
		int iRet = 0;
		AMFObjectProperty prop;
		iRet = AMF_PropDecode(&prop, buffer, nSize, decodeName);
		if (iRet<0)
		{
			err = true;
		}
		else
		{
			nSize -= iRet;
			buffer += iRet;
			AMF_AddProp(obj, &prop);
		}
	}
	if (err)
	{
		return -1;
	}
	return bufferSize-nSize;
}

void AMF_AddProp(AMFObject * obj, AMFObjectProperty * prop)
{
	AMFObjectProperty *pProp = new AMFObjectProperty(*prop);
	obj->objectProps.push_back(pProp);
}

AMFObjectProperty * AMF_GetPropByIndex(AMFObject * obj, int index)
{
	int count = 0;
	for (auto i:obj->objectProps)
	{
		if (count++ ==index)
		{
			return i;
		}
	}
	return nullptr;
}

AMFObjectProperty::AMFObjectProperty():UTCoffset(0)
{
}

AMFObjectProperty::AMFObjectProperty(const AMFObjectProperty &src)
{
	this->name = src.name;
	this->type = src.type;
	this->UTCoffset = src.UTCoffset;
	this->vu = src.vu;
}

AMFObjectProperty & AMFObjectProperty::operator=(const AMFObjectProperty& src)
{
	// TODO: 在此处插入 return 语句
	this->name = src.name;
	this->type = src.type;
	this->UTCoffset = src.UTCoffset;
	this->vu = src.vu;
	return *this;
}

AMFObjectProperty::~AMFObjectProperty()
{
}

AMFObjectData::AMFObjectData()
{
	number = 0.0;
	strValue = "";
}

AMFObjectData::AMFObjectData(const AMFObjectData &src)
{
	this->number = src.number;
	this->strValue = src.strValue;
	this->object = src.object;
}

AMFObjectData & AMFObjectData::operator=(const AMFObjectData &src)
{
	// TODO: 在此处插入 return 语句
	this->number = src.number;
	this->strValue = src.strValue;
	this->object = src.object;
	return *this;
}

AMFObjectData::~AMFObjectData()
{
}

AMFObject::AMFObject()
{
}

AMFObject::AMFObject(const AMFObject &src)
{
	this->objectProps = src.objectProps;
}

AMFObject & AMFObject::operator=(const AMFObject &src)
{
	// TODO: 在此处插入 return 语句
	this->objectProps = src.objectProps;
	return *this;
}
