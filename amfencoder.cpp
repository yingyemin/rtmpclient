#include "amfencoder.h"

const int amfEncoder::s_default_size = 4096;

amfEncoder::amfEncoder()
	: _buff(nullptr)
	, _size(s_default_size)
	, _cur(0)
{
	this->_buff = (char*)malloc(this->_size);
}

amfEncoder::~amfEncoder()
{
	if (_buff)
	{
		free(_buff);
		_buff = nullptr;
	}
	_size = 0;
	_cur = 0;
}

amfEncoder::amfEncoder(const amfEncoder &src)
{
	this->_size = src._size;
	this->_cur = src._cur;
	if (this->_size > 0&&src._buff)
	{
		this->_buff = (char*)malloc(src._size);
		memcpy(this->_buff, src._buff, src._size);
	}
}

amfEncoder & amfEncoder::operator=(const amfEncoder &src)
{
	// TODO: 在此处插入 return 语句
	this->_size = src._size;
	this->_cur = src._cur;
	if (this->_size > 0 && src._buff)
	{
		this->_buff = (char*)malloc(src._size);
		memcpy(this->_buff, src._buff, src._size);
	}
	return *this;
}

void amfEncoder::reset()
{
	if (_buff)
	{
		free(_buff);
		_buff = nullptr;
	}
	_size = 0;
	_cur = 0;
}

char * amfEncoder::getAmfData(int & size)
{
	size = this->_cur;
	return _buff;
}

char * amfEncoder::getAmfData_(int & size)
{
	size = this->_cur;
	if (size == 0)
	{
		return nullptr;
	}
	char *buffOut = (char*)malloc(size);
	memcpy(buffOut, this->_buff, size);
	return buffOut;
}

int amfEncoder::getDataSize()
{
	return _cur;
}

bool amfEncoder::EncodeString(const std::string & str)
{
	if (str.size() <= 0xffff)
	{
		checkBuffSize(str.size() + 3);
		_buff[_cur++] = AMF_string;
		if (!EncodeInt16(str.size()))
		{
			return false;
		}
	}
	else
	{
		checkBuffSize(str.size() + 5);
		_buff[_cur++] = AMF_long_string;
		if (!EncodeInt32(str.size()))
		{
			return false;
		}
	}
	memcpy(_buff + _cur, str.data(), str.size());
	_cur += str.size();
	return true;
}

bool amfEncoder::EncodeNumber(const double dVal)
{
	unsigned long long tmp64 = *reinterpret_cast<unsigned long long*>(const_cast<double*>(&dVal));
	checkBuffSize(9);
	_buff[_cur++] = AMF_number;
#if BYTE_ORDER==BIG_ENDIAN
	_buff[_cur++] = ((tmp64 >> 0) & 0xff);
	_buff[_cur++] = ((tmp64 >> 8) & 0xff);
	_buff[_cur++] = ((tmp64 >> 16) & 0xff);
	_buff[_cur++] = ((tmp64 >> 24) & 0xff);
	_buff[_cur++] = ((tmp64 >> 32) & 0xff);
	_buff[_cur++] = ((tmp64 >> 40) & 0xff);
	_buff[_cur++] = ((tmp64 >> 48) & 0xff);
	_buff[_cur++] = ((tmp64 >> 56) & 0xff);
#else	
	_buff[_cur++] = ((tmp64 >> 56) & 0xff);
	_buff[_cur++] = ((tmp64 >> 48) & 0xff);
	_buff[_cur++] = ((tmp64 >> 40) & 0xff);
	_buff[_cur++] = ((tmp64 >> 32) & 0xff);
	_buff[_cur++] = ((tmp64 >> 24) & 0xff);
	_buff[_cur++] = ((tmp64 >> 16) & 0xff);
	_buff[_cur++] = ((tmp64 >> 8) & 0xff);
	_buff[_cur++] = ((tmp64 >> 0) & 0xff);
#endif	
	return true;
}

bool amfEncoder::EncodeBool(const bool bVal)
{
	checkBuffSize(2);
	_buff[_cur++] = AMF_boolean;
	_buff[_cur++] = bVal ? 0x01 : 0x00;
	return true;
}

bool amfEncoder::EncodeInt16(const short nVal)
{
	checkBuffSize(2);
#if BYTE_ORDER==BIG_ENDIAN
	_buff[_cur++] = ((nVal >> 0) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
#else	
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 0) & 0xff);
#endif
	return true;
}

bool amfEncoder::EncodeInt24(const int nVal)
{
	checkBuffSize(3);
#if BYTE_ORDER==BIG_ENDIAN
	_buff[_cur++] = ((nVal >> 0) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 16) & 0xff);
#else	
	_buff[_cur++] = ((nVal >> 16) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 0) & 0xff);
#endif // BYTE_ORDER==BIG_ENDIAN
	return true;
}

bool amfEncoder::EncodeInt24(char * data, const int nVal)
{
	int cur = 0;
#if BYTE_ORDER==BIG_ENDIAN
	data[cur++] = ((nVal >> 0) & 0xff);
	data[cur++] = ((nVal >> 8) & 0xff);
	data[cur++] = ((nVal >> 16) & 0xff);
#else
	data[cur++] = ((nVal >> 16) & 0xff);
	data[cur++] = ((nVal >> 8) & 0xff);
	data[cur++] = ((nVal >> 0) & 0xff);
#endif // BYTE_ORDER==BIG_ENDIAN
	return true;
}

bool amfEncoder::EncodeInt32(const int nVal)
{
	checkBuffSize(4);
#if BYTE_ORDER==BIG_ENDIAN
	_buff[_cur++] = ((nVal >> 0) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 16) & 0xff);
	_buff[_cur++] = ((nVal >> 24) & 0xff);
#else	
	_buff[_cur++] = ((nVal >> 24) & 0xff);
	_buff[_cur++] = ((nVal >> 16) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 0) & 0xff);
#endif // BYTE_ORDER==BIG_ENDIAN
	return true;
}

bool amfEncoder::EncodeInt32(char * data, const int nVal)
{
	int cur = 0;
#if BYTE_ORDER==BIG_ENDIAN
	data[cur++] = ((nVal >> 0) & 0xff);
	data[cur++] = ((nVal >> 8) & 0xff);
	data[cur++] = ((nVal >> 16) & 0xff);
	data[cur++] = ((nVal >> 24) & 0xff);
#else
	data[cur++] = ((nVal >> 24) & 0xff);
	data[cur++] = ((nVal >> 16) & 0xff);
	data[cur++] = ((nVal >> 8) & 0xff);
	data[cur++] = ((nVal >> 0) & 0xff);
#endif // BYTE_ORDER==BIG_ENDIAN
	return true;
}

bool amfEncoder::EncodeNumber(char * data, const double dVal)
{
	unsigned long long tmp64 = *reinterpret_cast<unsigned long long*>(const_cast<double*>(&dVal));
	int cur = 0;
	data[cur++] = AMF_number;
#if BYTE_ORDER==BIG_ENDIAN
	data[cur++] = ((tmp64 >> 0) & 0xff);
	data[cur++] = ((tmp64 >> 8) & 0xff);
	data[cur++] = ((tmp64 >> 16) & 0xff);
	data[cur++] = ((tmp64 >> 24) & 0xff);
	data[cur++] = ((tmp64 >> 32) & 0xff);
	data[cur++] = ((tmp64 >> 40) & 0xff);
	data[cur++] = ((tmp64 >> 48) & 0xff);
	data[cur++] = ((tmp64 >> 56) & 0xff);
#else	
	data[cur++] = ((tmp64 >> 56) & 0xff);
	data[cur++] = ((tmp64 >> 48) & 0xff);
	data[cur++] = ((tmp64 >> 40) & 0xff);
	data[cur++] = ((tmp64 >> 32) & 0xff);
	data[cur++] = ((tmp64 >> 24) & 0xff);
	data[cur++] = ((tmp64 >> 16) & 0xff);
	data[cur++] = ((tmp64 >> 8) & 0xff);
	data[cur++] = ((tmp64 >> 0) & 0xff);
#endif	
	return true;
	return true;
}

bool amfEncoder::EncodeInt32LittleEndian(const int nVal)
{
	checkBuffSize(4);
#if BYTE_ORDER==LITTLE_ENDIAN
	_buff[_cur++] = ((nVal >> 0) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 16) & 0xff);
	_buff[_cur++] = ((nVal >> 24) & 0xff);
#else	
	_buff[_cur++] = ((nVal >> 24) & 0xff);
	_buff[_cur++] = ((nVal >> 16) & 0xff);
	_buff[_cur++] = ((nVal >> 8) & 0xff);
	_buff[_cur++] = ((nVal >> 0) & 0xff);
#endif // BYTE_ORDER==BIG_ENDIAN
	return true;
}

bool amfEncoder::EncodeNamedString(const std::string & name, const  std::string & str)
{
	checkBuffSize(2 + name.size());
	if (!EncodeInt16(name.size()))
	{
		return false;
	}
	memcpy(_buff + _cur, name.data(), name.size());
	_cur += name.size();
	if (!EncodeString(str))
	{
		return false;
	}
	return true;
}

bool amfEncoder::EncodeNamedNumber(const std::string & name, const  double dVal)
{
	checkBuffSize(2 + name.size());
	if (!EncodeInt16(name.size()))
	{
		return false;
	}
	memcpy(_buff + _cur, name.data(), name.size());
	_cur += name.size();
	if (!EncodeNumber(dVal))
	{
		return false;
	}
	return true;
}

bool amfEncoder::EncodeNamedBool(const std::string & name, const  bool bVal)
{
	checkBuffSize(2 + name.size());
	if (!EncodeInt16(name.size()))
	{
		return false;
	}
	memcpy(_buff + _cur, name.data(), name.size());
	_cur += name.size();
	if (!EncodeBool(bVal))
	{
		return false;
	}
	return true;
}

bool amfEncoder::EncodeObject(AMFObject * obj)
{
	checkBuffSize(1);
	_buff[_cur++] = AMF_object;
	for (auto i : obj->objectProps)
	{
		if (!EncodeProp(i))
		{
			break;
		}
	}
	checkBuffSize(3);
	if (!EncodeInt24(AMF_object_end))
	{
		return false;
	}
	return true;
}

bool amfEncoder::EncodeProp(AMFObjectProperty * prop)
{
	if (AMF_null != prop->type&&prop->name.size() > 0)
	{
		checkBuffSize(2 + prop->name.size());
		if (!EncodeInt16(prop->name.size()))
		{
			return false;
		}
		memcpy(_buff + _cur, prop->name.data(), prop->name.size());
		_cur += prop->name.size();
	}
	bool result = true;
	switch (prop->type)
	{
	case AMF_number:
		result = EncodeNumber(prop->vu.number);
		break;
	case AMF_boolean:
		result = EncodeBool(prop->vu.number != 0.0);
		break;
	case AMF_string:
		result = EncodeString(prop->vu.strValue);
		break;
	case AMF_object:
		result = EncodeObject(&prop->vu.object);
		break;
	case AMF_movieclip:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	case AMF_null:
		checkBuffSize(1);
		_buff[_cur++] = AMF_null;
		break;
	case AMF_undefined:
		break;
	case AMF_reference:
		break;
	case AMF_ecma_array:
		result = EncodeEcmaArray(&prop->vu.object);
		break;
	case AMF_object_end:
		//在编码Object后，自动加上，这里不加
		break;
	case AMF_strict_array:
		result = EncodeStrictArray(&prop->vu.object);
		break;
	case AMF_date:
		EncodeNumber(prop->vu.number);
		EncodeInt16(0);
		break;
	case AMF_long_string:
		EncodeString(prop->vu.strValue);
		break;
	case AMF_unsupported:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	case AMF_recordset:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	case AMF_xml_document:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	case AMF_typed_object:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	case AMF_avmplus_object:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	default:
		LOGW("encode amf type:" << prop->type << "not support now");
		break;
	}
	return true;
}

bool amfEncoder::EncodeEcmaArray(AMFObject * obj)
{
	checkBuffSize(1);
	_buff[_cur++] = AMF_ecma_array;
	EncodeInt32(obj->objectProps.size());
	for (auto i : obj->objectProps)
	{
		if (!EncodeProp(i))
		{
			return false;
		}
	}
	checkBuffSize(3);
	EncodeInt24(AMF_object_end);
	return true;
}

bool amfEncoder::EncodeStrictArray(AMFObject * obj)
{
	checkBuffSize(1);
	_buff[_cur++] = AMF_strict_array;
	EncodeInt32(obj->objectProps.size());

	for (auto i : obj->objectProps)
	{
		if (!EncodeProp(i))
		{
			return false;
		}
	}
	return true;
}

bool amfEncoder::AppendByte(char data)
{
	checkBuffSize(1);
	_buff[_cur++] = data;
	return true;
}

bool amfEncoder::AppendByteArray(char * data, int size)
{
	checkBuffSize(size);
	memcpy(_buff + _cur, data, size);
	_cur += size;
	return true;
}

void amfEncoder::checkBuffSize(int acquest)
{
	while (_size - _cur < acquest)
	{
		_size += s_default_size;
		_buff = (char*)realloc(_buff, _size);
	}
}
