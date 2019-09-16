#pragma once

#include "amfdecoder.h"
//解码AMF采用结构体方便些，感觉类的封装不利于amf的数据的访问

class amfEncoder
{
public:
	amfEncoder();
	~amfEncoder();
	amfEncoder(const amfEncoder&);
	amfEncoder&operator=(const amfEncoder&);
	void reset();
	char *getAmfData(int &size);
	char *getAmfData_(int &size);
	int		getDataSize();
	bool	EncodeString(const std::string &str);
	bool	EncodeNumber(const double dVal);
	bool	EncodeBool(const bool bVal);
	bool	EncodeInt16(const short nVal);
	bool	EncodeInt24(const int nVal);
	static bool EncodeInt24(char *data, const int nVal);
	bool	EncodeInt32(const int nVal);
	static bool EncodeInt32(char *data, const int nVal);
	static bool EncodeNumber(char *data, const double dVal);
	bool	EncodeInt32LittleEndian(const int nVal);
	bool	EncodeNamedString(const std::string &name, const  std::string &str);
	bool	EncodeNamedNumber(const std::string &name, const  double dVal);
	bool	EncodeNamedBool(const std::string &name, const  bool bVal);
	bool	EncodeObject(AMFObject *obj);
	bool	EncodeProp(AMFObjectProperty *prop);
	bool	EncodeEcmaArray(AMFObject *obj);
	bool	EncodeStrictArray(AMFObject *obj);
	bool	AppendByte(char data);
	bool	AppendByteArray(char *data, int size);
private:
	void	checkBuffSize(int acquest);
	char	*_buff;
	int		_size;
	int		_cur;
	static const int s_default_size;
};
