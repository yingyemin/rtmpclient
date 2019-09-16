#include "rtmpclient.h"
#include "amfencoder.h"
#include "amfdecoder.h"
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>

#define RTMP_MIN(a,b)   (((a) < (b)) ? (a) : (b))

bool Read8(int &i8, FILE *fp){
    if(fread(&i8, 1, 1, fp) != 1){
        return false;
    }
    return true;
}
bool Read16(int &i16, FILE *fp){
    if(fread(&i16, 2, 1, fp) != 1){
        return false;
    }
    i16 = htons(i16);
    return true;
}
bool Read24(int &i24, FILE *fp){
    unsigned char byte;
    if(fread(&byte, 1, 1, fp) != 1){
        return false;
    }
    i24 += byte << 16;
    if(fread(&byte, 1, 1, fp) != 1){
        return false;
    }
    i24 += byte << 8;
    if(fread(&byte, 1, 1, fp) != 1){
        return false;
    }
    i24 += byte;
    return true;
}
bool Read32(int &i32, FILE *fp){
    if(fread(&i32, 4, 1, fp) != 1){
        return false;
    }
    i32 = htonl(i32);
    return true;
}
bool ReadTime(uint32_t &time, FILE *fp){
    if(fread(&time, 4, 1, fp) != 1){
        return false;
    }
    time = ((time>>16&0xff)|(time<<16&0xff0000)|(time&0xff00)|(time&0xff000000));
    return true;
}

RtmpClient::RtmpClient(const string& url, const string& filename)
	:_url(url)
	,_filename(filename)
{
    _numInvokes = 0;
    _chunkSize = 128;
}

RtmpClient::~RtmpClient()
{}

int RtmpClient::parseUrl()
{
    char *p;
    char *phost;
    char *papp, *pinstance;

    char proto[4] = {0};
    char hostname[128] = {0};
    char app[12] = {0};
    char instance[64] = {0};
    char* url = (char*)_url.c_str();

    if (NULL == url || NULL == proto)
    {
        return -1;
    }

    p = strchr(url, ':');
    if (NULL != p)
    {
        memcpy(proto, url, RTMP_MIN(p - url, 4));
    }

    p++;            //skip ':'
    if (*p == '/')          //skip '/'
    {
        p++;
    }
    if (*p == '/')
    {
        p++;
    }

    phost = strchr(p, '/');
    if (NULL != phost)
    {
        memcpy(hostname, p, RTMP_MIN(phost - p, 128));
    }

    phost++; //skip '/'

    papp = strchr(phost, '/');
    if (NULL != papp)
    {
        memcpy(app, phost, RTMP_MIN(papp - phost, 12));
    }

    papp++;


    memcpy(instance, papp, 64);
    _ip = hostname;
    _proto = proto;
    _app = app;
    _streamname = instance;
    _port = 1935;

    return 0;
}

int RtmpClient::initSocket()
{
    _fd = socket(AF_INET,SOCK_STREAM,0);
    if (_fd == -1) {
        return -1;
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(_port);
    server_addr.sin_addr.s_addr=inet_addr(_ip.c_str());

    if(::connect(_fd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
    {
	return -1;
    }

    return 0;
}

int RtmpClient::handShake()
{
    uint8_t C0 = 3;
    uint8_t S0 = 0;

    int send_ret = -1;
    int recv_ret = -1;

    send_ret = send(_fd, (char*)&C0, sizeof(C0), 0);
    if (send_ret < 0) {
        return -1;
    }
    uint8_t C1[1536] = {
	    0, 0, 0, 0, //time
	    0, 0, 0, 0, //zero
    };
    srand(0xDEADC0DE);
    for (int i = 8; i < 1536; i++) {
        C1[i] = rand();
    }
    cout << "send C1" << endl;
    send_ret = send(_fd, (char*)&C1, 1536, 0);
    if (send_ret < 0) {
        return -1;
    }
    cout << "recv S0" << endl;
    recv_ret = recv(_fd, (char*)&S0, sizeof(S0), 0);
    if (recv_ret < 0) {
        return -1;
    }
    cout << "recv S1" << endl;
    uint8_t S1[1536] = {0};
    recv_ret = recv(_fd, (char*)&S1, 1536, 0);
    if (recv_ret < 0) {
        return -1;
    }
    cout << "recv S2" << endl;
    uint8_t S2[1536] = {0};
    recv_ret = recv(_fd, (char*)&S2, 1536, 0);
    if (recv_ret < 0) {
        return -1;
    }
    send_ret = send(_fd, (char*)&S1, 1536, 0);
    if (send_ret < 0) {
        return -1;
    }

    return 0;
}

int RtmpClient::connect()
{
    RTMPPacket packet;
    packet.channel = RTMP_channel_invoke;
    packet.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.headerType = RTMP_HEADER_LARGE;
    packet.timestamp = 0;
    packet.hasAbsTimestamp = true;
    packet.messageStreamId = 0;

    amfEncoder encoder;

    //command name:connect
    encoder.EncodeString(RTMP_CMD_connect);
    //Ìí¼ÓÖ´ÐÐID
    encoder.EncodeNumber(++_numInvokes);
    //command object
    //object Îª×Ö·û´®×ökey£¬ÆäËûÖµ×övalueµÄ¼üÖµ¶Ô
    encoder.AppendByte(AMF_object);
    //app
    encoder.EncodeNamedString(connect_CMD_app, "live"/*_app.c_str()*/);
    //flashver
    encoder.EncodeNamedString(connect_CMD_flashver, "LNX 9,0,124,2");
    //swfUrl
    encoder.EncodeNamedString(connect_CMD_swfUrl, "");
    //tcUrl
    encoder.EncodeNamedString(connect_CMD_tcUrl, "rtmp://127.0.0.1/live"/*_url.c_str()*/);
    encoder.EncodeNamedBool(connect_CMD_fpad, false);
    encoder.EncodeNamedNumber(connect_CMD_capabilities, 15.0);
    encoder.EncodeNamedNumber(connect_CMD_audioCodecs, 3191.0);
    encoder.EncodeNamedNumber(connect_CMD_videoCodecs, 252);
    encoder.EncodeNamedNumber(connect_CMD_videoFunction, 1.0);
    //obj end
    encoder.EncodeInt24(AMF_object_end);
    int tmpLength;
    packet.body = encoder.getAmfData(tmpLength);
    packet.messageLength = tmpLength;
    cout << "send packet" << endl;
    int ret = sendPacket(packet);
    //free encoderµÄÎö¹¹º¯Êý»áÊÍ·Åamf buffer
    cout << "send ret: " << ret << endl;
    RTMPPacket rPacket1;
    recvPacket(rPacket1);

    cout << "packet type: " << rPacket1.packetType << endl;
    cout << "head type:   " << rPacket1.headerType << endl;
    cout << "body:        " << rPacket1.body       << endl;
    cout << "decode BW:   " << AMF_DecodeInt32(rPacket1.body) << endl;

    RTMPPacket rPacket2;
    recvPacket(rPacket2);

    cout << "packet type: " << rPacket2.packetType << endl;
    cout << "head type:   " << rPacket2.headerType << endl;
    cout << "body:        " << rPacket2.body       << endl;
    cout << "decode BW:   " << AMF_DecodeInt32(rPacket2.body) << endl;

    RTMPPacket rPacket3;
    recvPacket(rPacket3);

    cout << "packet type: " << rPacket3.packetType << endl;
    cout << "head type:   " << rPacket3.headerType << endl;
    cout << "body:        " << rPacket3.body       << endl;
    cout << "decode chunk size: " << AMF_DecodeInt32(rPacket3.body) << endl;
    _chunkSize = AMF_DecodeInt32(rPacket3.body);

    RTMPPacket rPacket4;
    recvPacket(rPacket4);

    cout << "packet type: " << rPacket4.packetType << endl;
    cout << "head type:   " << rPacket4.headerType << endl;
    //cout << "body:        " << rPacket4.body       << endl;

    RTMPPacket rPacket5;
    recvPacket(rPacket5);

    cout << "packet type: " << rPacket5.packetType << endl;
    cout << "head type:   " << rPacket5.headerType << endl;
    cout << "body:        " << rPacket5.body       << endl;

    RTMPPacket packet5;
    packet5.channel = RTMP_channel_control;
    packet5.packetType = RTMP_PACKET_TYPE_SERVER_BW;
    packet5.headerType = RTMP_HEADER_LARGE;
    packet5.timestamp = 0;
    packet5.hasAbsTimestamp = true;
    packet5.messageStreamId = 0;

    amfEncoder encoder1;
    encoder1.EncodeInt32(AMF_DecodeInt32(rPacket1.body));
    int size = 0;
    packet5.body = encoder1.getAmfData(size);
    packet5.messageLength = size;
    if (0 != sendPacket(packet5)) {
	cout << "send error" << endl;
        return -1;
    }
 /*   RTMPPacket rPacket6;
    recvPacket(rPacket6);

    cout << "packet type: " << rPacket6.packetType << endl;
    cout << "head type:   " << rPacket6.headerType << endl;
    cout << "body:        " << rPacket6.body       << endl;*/

    return 0;
}

int RtmpClient::createStream()
{
    RTMPPacket packet;
    packet.channel = RTMP_channel_invoke;
    packet.headerType = RTMP_HEADER_MEDIUM;
    packet.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.timestamp = 0;
    packet.hasAbsTimestamp = false;
    packet.messageStreamId = 0;

    amfEncoder encoder;
    encoder.EncodeString(RTMP_CMD_createStream);
    encoder.EncodeNumber(++_numInvokes);
    encoder.AppendByte(AMF_null);
    int size = 0;
    packet.body = encoder.getAmfData(size);
    packet.messageLength = size;
    if (0 != sendPacket(packet))
    {
            return -1;
    }

    RTMPPacket rPacket5;
    recvPacket(rPacket5);

    cout << "packet type: " << rPacket5.packetType << endl;
    cout << "head type:   " << rPacket5.headerType << endl;
    cout << "body:        " << rPacket5.body       << endl;

    return 0;
}

int RtmpClient::publish()
{
    //release stream
    cout << "release stream" << endl;
    RTMPPacket packet;
    packet.channel = RTMP_channel_invoke;
    packet.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.headerType = RTMP_HEADER_LARGE;
    packet.timestamp = 0;
    packet.hasAbsTimestamp = true;
    packet.messageStreamId = 0;

    amfEncoder encoder;
    encoder.EncodeString(RTMP_CMD_releaseStream);
    encoder.EncodeNumber(++_numInvokes);
    encoder.AppendByte(AMF_null);
    encoder.EncodeString("test");
    int size = 0;
    packet.body = encoder.getAmfData(size);
    packet.messageLength = size;
    if (0 != sendPacket(packet)) {
        return -1;
    }

    //FCPublish
    cout << "FCPublish" << endl;
    RTMPPacket packet1;
    packet1.channel = RTMP_channel_invoke;
    packet1.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet1.headerType = RTMP_HEADER_LARGE;
    packet1.timestamp = 0;
    packet1.hasAbsTimestamp = true;
    packet1.messageStreamId = 0;

    amfEncoder encoder1;
    encoder1.EncodeString(RTMP_CMD_FCPublish);
    encoder1.EncodeNumber(++_numInvokes);
    encoder1.AppendByte(AMF_null);
    encoder1.EncodeString("test");
    size = 0;
    packet1.body = encoder1.getAmfData(size);
    packet1.messageLength = size;
    if (0 != sendPacket(packet1)) {
	return -1;
    }

    //create stream
    cout << "create stream" << endl;
    RTMPPacket packet2;
    packet2.channel = RTMP_channel_invoke;
    packet2.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet2.headerType = RTMP_HEADER_LARGE;
    packet2.timestamp = 0;
    packet2.hasAbsTimestamp = true;
    packet2.messageStreamId = 0;

    amfEncoder encoder2;
    encoder2.EncodeString(RTMP_CMD_createStream);
    encoder2.EncodeNumber(++_numInvokes);
    encoder2.AppendByte(AMF_null);
    size = 0;
    packet2.body = encoder2.getAmfData(size);
    packet2.messageLength = size;
    if (0 != sendPacket(packet2)) {
	return -1;
    }

    //_result
    cout << "_result" << endl;
    RTMPPacket rPacket;
    recvPacket(rPacket);

    cout << "packet type: " << rPacket.packetType << endl;
    cout << "head type:   " << rPacket.headerType << endl;
    cout << "body:        " << rPacket.body       << endl;
    AMFObject obj;
    int iRet = AMF_Decode(&obj, rPacket.body, rPacket.messageLength, false);
    if (0 > iRet)
        {
                LOGE("decode amf object failed:" << rPacket.body);
                AMF_Clear(&obj);
                return iRet;
        }
    string method = AMF_GetPropByIndex(&obj, 0)->vu.strValue;
    int transcationId = AMF_GetPropByIndex(&obj, 1)->vu.number;
    _streamid = AMF_GetPropByIndex(&obj, 3)->vu.number;

    cout << method << endl;
    cout << transcationId << endl;
    cout << _streamid << endl;

    //publish
    cout << "publish" << endl;
    RTMPPacket packet3;
    packet3.channel = RTMP_channel_invoke;
    packet3.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet3.headerType = RTMP_HEADER_LARGE;
    packet3.timestamp = 0;
    packet3.hasAbsTimestamp = true;
    packet3.messageStreamId = _streamid;

    amfEncoder encoder3;
    encoder3.EncodeString(RTMP_CMD_publish);
    encoder3.EncodeNumber(++_numInvokes);
    encoder3.AppendByte(AMF_null);
    encoder3.EncodeString("test");
    encoder3.EncodeString("live");
    size = 0;
    packet3.body = encoder3.getAmfData(size);
    packet3.messageLength = size;
    if (0 != sendPacket(packet3)) {
	return -1;
    }

    //set chunk size
    cout << "set chunk size" << endl;
    RTMPPacket packet4;
    packet4.channel = RTMP_channel_control;
    packet4.packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
    packet4.headerType = RTMP_HEADER_LARGE;
    packet4.timestamp = 0;
    packet4.hasAbsTimestamp = true;
    packet4.messageStreamId = 0;

    amfEncoder encoder4;
    encoder4.EncodeInt32(_chunkSize);
    size = 0;
    packet4.body = encoder4.getAmfData(size);
    packet4.messageLength = size;
    if (0 != sendPacket(packet4)) {
	return -1;
    }

    //_result
    RTMPPacket rPacket2;
    recvPacket(rPacket2);
    //_result
    RTMPPacket rPacket3;
    recvPacket(rPacket3);
    //onFCPublish
    RTMPPacket rPacket4;
    recvPacket(rPacket4);
    //onStatus
    cout << "onStatus" << endl;
    RTMPPacket rPacket1;
    recvPacket(rPacket1);

    cout << "packet type: " << rPacket1.packetType << endl;
    cout << "head type:   " << rPacket1.headerType << endl;
    cout << "body:        " << rPacket1.body       << endl;

    AMFObject obj1;
    iRet = AMF_Decode(&obj1, rPacket1.body, rPacket1.messageLength, false);
    if (0 > iRet)
        {
                LOGE("decode amf object failed:" << rPacket1.body);
                AMF_Clear(&obj1);
                return iRet;
        }
    method = AMF_GetPropByIndex(&obj1, 0)->vu.strValue;
    transcationId = AMF_GetPropByIndex(&obj1, 1)->vu.number;

    cout << method << endl;
    cout << transcationId << endl;

    AMFObject objStatus;
                        objStatus = AMF_GetPropByIndex(&obj1, 3)->vu.object;
                        //5¸ö²Å¶Ô
                        for (auto i : objStatus.objectProps)
                        {
                                if (RTMP_CMD_code == i->name)
                                {
                                        string code = i->vu.strValue;
					cout << "code: " << code << endl;
                                }
                                else if (RTMP_CMD_level == i->name)
                                {
                                        string level = i->vu.strValue;
					cout << "level: " << level << endl;
                                }
                                else if (RTMP_CMD_description == i->name)
                                {
                                        string description = i->vu.strValue;
					cout << "description: " << description << endl;
                                }
                        }

    return 0;
}

int RtmpClient::sendFlvFile()
{
    FILE* fp = fopen(_filename.c_str(), "rb");
    if (!fp) {
	cout << "open flv file failed" << endl;
        return -1;
    }
    fseek(fp, 9, SEEK_SET); //skip flv header
    fseek(fp, 4, SEEK_CUR); //skip pre tag size

    while (true) {
        int type = 0;
	int datalength = 0;
	uint32_t time = 0;
	int streamid = 0;

        if(!Read8(type, fp)){
	    cout << "failed to read type" << endl;
	    break;
	}
	std::cout<<"type: "<<type<<std::endl;
	if(!Read24(datalength, fp)){
            std::cout<<"Failed to read datalength"<<std::endl;
            break;
        }
	std::cout<<"datalength: "<<datalength<<std::endl;
        if(!ReadTime(time, fp)){
            std::cout<<"Failed to read time"<<std::endl;
            break;
        }
        std::cout<<"time: "<<time<<std::endl;
        if(!Read24(streamid, fp)){
            std::cout<<"Failed to read streamid"<<std::endl;
            break;
        }
        std::cout<<"streamid: "<<streamid<<std::endl;
        if(type != 0x08 && type != 0x09){
            fseek(fp, datalength + 4, SEEK_CUR);
            continue;
        }
	char* buf = new char[datalength];
	if(fread(buf, 1, datalength, fp) != datalength){
            std::cout<<"Failed to read body"<<std::endl;
            break;
        }
        int alldatalength = 0;
        if(!Read32(alldatalength, fp)){
            std::cout<<"Failed to read all data length"<<std::endl;
            break;
        }
	cout << "send flv data" << endl;
        RTMPPacket packet;
        packet.channel = (type == 0x08 ? 4 : 5);
        packet.packetType = type;
        packet.headerType = RTMP_HEADER_LARGE;
        packet.timestamp = time;
        packet.hasAbsTimestamp = true;
        packet.messageStreamId = _streamid;
        
        packet.body = buf;
        packet.messageLength = datalength;
        if (0 != sendPacket(packet)) {
            return -1;
        }
	delete[] buf;
	usleep(40 * 1000);
    }
    stopPublish();
}

int RtmpClient::stopPublish()
{
    //FCUnpublish
    cout << "FCUnpublish" << endl;
    RTMPPacket packet;
    packet.channel = RTMP_channel_invoke;
    packet.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.headerType = RTMP_HEADER_LARGE;
    packet.timestamp = 0;
    packet.hasAbsTimestamp = true;
    packet.messageStreamId = _streamid;

    amfEncoder encoder;
    encoder.EncodeString(RTMP_CMD_FCUnpublish);
    encoder.EncodeNumber(++_numInvokes);
    encoder.AppendByte(AMF_null);
    encoder.EncodeString("test");
    int size = 0;
    packet.body = encoder.getAmfData(size);
    packet.messageLength = size;
    if (0 != sendPacket(packet)) {
        return -1;
    }

    //close stream
    cout << "close stream" << endl;
    RTMPPacket packet1;
    packet1.channel = RTMP_channel_invoke;
    packet1.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet1.headerType = RTMP_HEADER_LARGE;
    packet1.timestamp = 0;
    packet1.hasAbsTimestamp = true;
    packet1.messageStreamId = _streamid;

    amfEncoder encoder1;
    encoder1.EncodeString(RTMP_CMD_deleteStream);
    encoder1.EncodeNumber(++_numInvokes);
    encoder1.AppendByte(AMF_null);
    encoder1.EncodeString("test");
    size = 0;
    packet1.body = encoder1.getAmfData(size);
    packet1.messageLength = size;
    if (0 != sendPacket(packet1)) {
        return -1;
    }

    RTMPPacket rPacket1;
    recvPacket(rPacket1);

    cout << "packet type: " << rPacket1.packetType << endl;
    cout << "head type:   " << rPacket1.headerType << endl;
    cout << "body:        " << rPacket1.body       << endl;

    AMFObject obj1;
    int iRet = AMF_Decode(&obj1, rPacket1.body, rPacket1.messageLength, false);
    if (0 > iRet)
    {
        LOGE("decode amf object failed:" << rPacket1.body);
        AMF_Clear(&obj1);
        return iRet;
    }
    string method = AMF_GetPropByIndex(&obj1, 0)->vu.strValue;
    cout << method << endl;
}

int RtmpClient::play()
{
    //create stream
    cout << "create stream" << endl;
    RTMPPacket packet;
    packet.channel = RTMP_channel_invoke;
    packet.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.headerType = RTMP_HEADER_LARGE;
    packet.timestamp = 0;
    packet.hasAbsTimestamp = true;
    packet.messageStreamId = 0;

    amfEncoder encoder;
    encoder.EncodeString(RTMP_CMD_createStream);
    encoder.EncodeNumber(++_numInvokes);
    encoder.AppendByte(AMF_null);
    int size = 0;
    packet.body = encoder.getAmfData(size);
    packet.messageLength = size;
    if (0 != sendPacket(packet)) {
        return -1;
    }

    //_result
    cout << "_result" << endl;
    RTMPPacket rPacket;
    recvPacket(rPacket);

    cout << "packet type: " << rPacket.packetType << endl;
    cout << "head type:   " << rPacket.headerType << endl;
    cout << "body:        " << rPacket.body       << endl;
    AMFObject obj;
    int iRet = AMF_Decode(&obj, rPacket.body, rPacket.messageLength, false);
    if (0 > iRet)
    {
        LOGE("decode amf object failed:" << rPacket.body);
        AMF_Clear(&obj);
        return iRet;
    }
    string method = AMF_GetPropByIndex(&obj, 0)->vu.strValue;
    int transcationId = AMF_GetPropByIndex(&obj, 1)->vu.number;
    _streamid = AMF_GetPropByIndex(&obj, 3)->vu.number;

    cout << method << endl;
    cout << transcationId << endl;
    cout << _streamid << endl;

    //getStreamLength
    cout << "getStreamLength" << endl;
    RTMPPacket packet1;
    packet1.channel = RTMP_channel_audioVideo;
    packet1.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet1.headerType = RTMP_HEADER_LARGE;
    packet1.timestamp = 0;
    packet1.hasAbsTimestamp = true;
    packet1.messageStreamId = 0;

    amfEncoder encoder1;
    encoder1.EncodeString(RTMP_CMD_getStreamLength);
    encoder1.EncodeNumber(++_numInvokes);
    encoder1.AppendByte(AMF_null);
    encoder1.EncodeString("test");
    size = 0;
    packet1.body = encoder1.getAmfData(size);
    packet1.messageLength = size;
    if (0 != sendPacket(packet1)) {
        return -1;
    }

    //play
    cout << "play" << endl;
    RTMPPacket packet2;
    packet2.channel = RTMP_channel_audioVideo;
    packet2.packetType = RTMP_PACKET_TYPE_INVOKE;
    packet2.headerType = RTMP_HEADER_LARGE;
    packet2.timestamp = 0;
    packet2.hasAbsTimestamp = true;
    packet2.messageStreamId = _streamid;

    amfEncoder encoder2;
    encoder2.EncodeString(RTMP_CMD_play);
    encoder2.EncodeNumber(++_numInvokes);
    encoder2.AppendByte(AMF_null);
    encoder2.EncodeString("test");
    encoder2.EncodeNumber(-2000);
    size = 0;
    packet2.body = encoder2.getAmfData(size);
    packet2.messageLength = size;
    if (0 != sendPacket(packet2)) {
        return -1;
    }

    // Set Buffer Length
    cout << "Set Buffer Length" << endl;
    RTMPPacket packet3;
    packet3.channel = RTMP_channel_control;
    packet3.packetType = RTMP_PACKET_TYPE_CONTROL;
    packet3.headerType = RTMP_HEADER_LARGE;
    packet3.timestamp = 0;
    packet3.hasAbsTimestamp = true;
    packet3.messageStreamId = _streamid;

    amfEncoder encoder3;
    encoder3.EncodeNumber(3);
    size = 0;
    packet3.body = encoder3.getAmfData(size);
    packet3.messageLength = size;
    if (0 != sendPacket(packet3)) {
        return -1;
    }

    //begin 1
    cout << "recv begin 1" << endl;
    RTMPPacket rPacket1;
    recvPacket(rPacket1);

    cout << AMF_DecodeInt32(rPacket1.body) << endl;

    //onstatus
    cout << "onstatus reset/start" << endl;
    RTMPPacket rPacket21;
    recvPacket(rPacket21);
    RTMPPacket rPacket2;
    recvPacket(rPacket2);
    cout << "packet type: " << rPacket2.packetType << endl;
    cout << "head type:   " << rPacket2.headerType << endl;
    cout << "body:        " << rPacket2.body       << endl;
    AMFObject obj2;
    iRet = AMF_Decode(&obj2, rPacket2.body, rPacket2.messageLength, false);
    if (0 > iRet)
    {
        LOGE("decode amf object failed:" << rPacket2.body);
        AMF_Clear(&obj2);
        return iRet;
    }
    method = AMF_GetPropByIndex(&obj2, 0)->vu.strValue;
    cout << method << endl;

    //RtmpSampleAccess
    cout << "RtmpSampleAccess" << endl;
    RTMPPacket rPacket3;
    recvPacket(rPacket3);
    cout << "packet type: " << rPacket3.packetType << endl;
    cout << "head type:   " << rPacket3.headerType << endl;
    cout << "body:        " << rPacket3.body       << endl;
    AMFObject obj3;
    iRet = AMF_Decode(&obj3, rPacket3.body, rPacket3.messageLength, false);
    if (0 > iRet)
    {
        LOGE("decode amf object failed:" << rPacket3.body);
        AMF_Clear(&obj3);
        return iRet;
    }
    method = AMF_GetPropByIndex(&obj3, 0)->vu.strValue;
    cout << method << endl;

    FILE* fp = fopen("test.flv", "wb+");
    if (!fp) {
        cout << "can not open output file" << endl;
	return -1;
    }
    char flv_header[] = {
        'F', 'L', 'V', // Signatures "FLV"
        (char)0x01, // File version (for example, 0x01 for FLV version 1)
        (char)0x01, // 4, audio; 1, video; 5 audio+video.
        (char)0x00, (char)0x00, (char)0x00, (char)0x09, // DataOffset UI32 The length of this header in bytes
	(char)0x00, (char)0x00, (char)0x00, (char)0x00 // pre tag size
    };
    fwrite(flv_header, 1, 13, fp); // flv header

    //onmetadata
    cout << "onmetadata" << endl;
    RTMPPacket rPacket51;
    recvPacket(rPacket51);
    RTMPPacket rPacket5;
    recvPacket(rPacket5);
    cout << "packet type: " << rPacket5.packetType << endl;
    cout << "head type:   " << rPacket5.headerType << endl;
    cout << "body:        " << rPacket5.body       << endl;
    AMFObject obj5;
    iRet = AMF_Decode(&obj5, rPacket5.body, rPacket5.messageLength, false);
    if (0 > iRet)
    {
        LOGE("decode amf object failed:" << rPacket5.body);
        AMF_Clear(&obj5);
        return iRet;
    }
    method = AMF_GetPropByIndex(&obj5, 0)->vu.strValue;
    cout << method << endl;

    char tagType = 0x12;
    char tagSize[3] = {0};
    char* tmp = (char*)&rPacket5.messageLength;
    tagSize[2] = *tmp++;
    tagSize[1] = *tmp++;
    tagSize[0] = *tmp;
    char tagTime[3] = {0};
    char tagExtendTime = 0x00;
    char tagStreamId[3] = {0};
    fwrite(&tagType, 1, 1, fp); //type
    fwrite(tagSize, 1, 3, fp); //size
    fwrite(tagTime, 1, 3, fp); //time
    fwrite(&tagExtendTime, 1, 1, fp); //extend time
    fwrite(tagStreamId, 1, 3, fp); //streamid
    fwrite(rPacket5.body, 1, rPacket5.messageLength, fp);
    int length = rPacket5.messageLength + 11;
    tmp = (char*)&length;
    char preTagSize[4] = {0};
    preTagSize[3] = *tmp++;
    preTagSize[2] = *tmp++;
    preTagSize[1] = *tmp++;
    preTagSize[0] = *tmp;
    fwrite(preTagSize, 1, 4, fp);

    //video/audio ??
    while (true) {
        RTMPPacket rPacket4;
        recvPacket(rPacket4);

        if (rPacket4.packetType == 8 || rPacket4.packetType == 9) {
            cout << "video or audio" << endl;
	    tagType = rPacket4.packetType;
	    fwrite(&tagType, 1, 1, fp);
            char* tmp = (char*)&rPacket4.messageLength;
	    tagSize[2] = *tmp++;
	    tagSize[1] = *tmp++;
	    tagSize[0] = *tmp;
	    fwrite(tagSize, 1, 3, fp);
	    tmp = (char*)&rPacket4.timestamp;
	    cout << rPacket4.timestamp << endl;
	    tagTime[2] = *tmp++;
	    tagTime[1] = *tmp++;
	    tagTime[0] = *tmp;
            fwrite(tagTime, 1, 3, fp);
	    tagExtendTime = (rPacket4.timestamp >> 24) & 0xff;
	    fwrite(&tagExtendTime, 1, 1, fp);
	    fwrite(tagStreamId, 1, 3, fp);
	    fwrite(rPacket4.body, 1, rPacket4.messageLength, fp);
	    length = rPacket4.messageLength + 11;
	    tmp = (char*)&length;
	    preTagSize[3] = *tmp++;
	    preTagSize[2] = *tmp++;
	    preTagSize[1] = *tmp++;
	    preTagSize[0] = *tmp++;
	    fwrite(preTagSize, 1, 4, fp);
        } else {
	    cout << "not audio or video" << endl;
	    break;
	}
	usleep(30*1000);
    }

    return 0;
}

int RtmpClient::sendPacket(RTMPPacket& packet)
{
    int baseHeaderSize = 1;
    if (packet.channel > 319) {
        baseHeaderSize = 3;
    } else if (packet.channel > 63) {
        baseHeaderSize = 2;
    }
    char tmp8 = 0;
    amfEncoder dataSend;
    tmp8 = RTMP_HEADER_LARGE << 6; //type 0
    if (baseHeaderSize > 1) {
        tmp8 = tmp8 | (baseHeaderSize - 1);
    } else {
        tmp8 = (tmp8 | packet.channel);
    }
    dataSend.AppendByte(tmp8);
    if (baseHeaderSize > 1) {
        int tmpChannel = packet.channel - 64;
	dataSend.AppendByte(tmpChannel & 0xff);
	if (baseHeaderSize == 3) {
	    dataSend.AppendByte(tmpChannel >> 8);
	}
    }
    int sendTime = packet.timestamp;
    dataSend.EncodeInt24(sendTime >= 0xffffff ? 0xffffff : sendTime);
    dataSend.EncodeInt24(packet.messageLength);
    dataSend.AppendByte(packet.packetType);
    dataSend.EncodeInt32LittleEndian(packet.messageStreamId);
    if (sendTime >= 0xffffff) {
        dataSend.EncodeInt32(sendTime);
    }
    int lastSendSize = 0;
    int bodyCur = 0;
    int firstChunkSize = packet.messageLength > _chunkSize ? _chunkSize : packet.messageLength;
    lastSendSize = packet.messageLength;

    dataSend.AppendByteArray(packet.body, firstChunkSize);
    int sendSize = 0;
    char *pData = dataSend.getAmfData(sendSize);
    if (sendSize != socketSend(pData, sendSize)) {
        return -1;
    }
    bodyCur += firstChunkSize;
    while (bodyCur != packet.messageLength) {
        dataSend.reset();
	int lastDataSize = packet.messageLength - bodyCur;
	if (lastDataSize > _chunkSize) {
	    lastDataSize = _chunkSize;
	}
	tmp8 = 0x3 << 6; //type 3
	if (baseHeaderSize == 1) {
	    tmp8 = tmp8 | packet.channel;
	    dataSend.AppendByte(tmp8);
	} else if (baseHeaderSize > 1){
	    tmp8 = tmp8 | (baseHeaderSize - 1);
	    dataSend.AppendByte(tmp8);
	    int tmpChannel = packet.channel - 64;
	    dataSend.AppendByte(tmpChannel & 0xff);
	    if (baseHeaderSize == 3) {
	        dataSend.AppendByte(tmpChannel >> 8);
	    }
	}
	dataSend.AppendByteArray(packet.body + bodyCur, lastDataSize);
	char *pData = dataSend.getAmfData(sendSize);
	if (sendSize != socketSend(pData, sendSize)) {
	    return -1;
	}
	bodyCur += lastDataSize;
    }

    return 0;
}

int RtmpClient::socketSend(const char * buf, int len)
{
    int iRet;
    int curPos = 0;
    int againTimes = 0;
    while(curPos<len) {
        iRet = write(_fd, buf + curPos, len - curPos);
	if (iRet != len - curPos) {
	    return -1;
	}
	curPos += iRet;
    }
    return len;
}

int RtmpClient::recvPacket(RTMPPacket& packet)
{
    int iRet = recvChunk(packet);
    if (iRet != 0) {
        return iRet;
    }
    while (packet.messageLength != packet.bytesRead) {
        iRet = recvChunk(packet);
	if (iRet != 0) {
	    return iRet;
	}
    }
    return 0;
}

int RtmpClient::recvChunk(RTMPPacket& packet_)
{
    int iRet = 0;
    int recvRet = 0;
    int recvSize = 0;
    unsigned char tmp8 = 0;
    unsigned short tmp16 = 0;
    unsigned int tmp32 = 0;
    char tmpChArr[3];

    do {
        if (packet_.body) {
	    free(packet_.body);
	    packet_.body = nullptr;
	}
	packet_.messageLength = 0;
	packet_.bytesRead = 0;
	recvSize = 1;
	recvRet = recv(_fd, (char*)&tmp8, recvSize, 0);
	if (recvRet != recvSize) {
	    cout << "recv fmt failed" << endl;
	    iRet = -1;
	    break;
	}
        packet_.headerType = RTMP_HEADER_TYPE((tmp8 & 0xc0) >> 6);
	packet_.channel = tmp8 & 0x3f;
	cout << "packet_.channel is: " << packet_.channel << endl;
	if (0 == packet_.channel) {
	    recvSize = 1;
            recvRet = recv(_fd, (char*)&tmp8, recvSize, 0);

	    if (recvRet != recvSize) {
	        iRet = -1;
	        break;
	    }
	    packet_.channel = tmp8 + 64;
	} else if (1 == packet_.channel) {
	    recvSize = 2;
	    recvRet = recv(_fd, (char*)&tmp16, recvSize, 0);
	    if (recvRet != recvSize) {
	        iRet = -1;
	        break;
	    }
	    tmp16 = (((tmp16 >> 8) & 0xFF) | ((tmp16 & 0xFF) << 8));
	    packet_.channel = tmp16 + 64;
	}
	if (packet_.headerType == RTMP_HEADER_MINIMUM) { // type 3
	    bool lastPacketRecvEnd = false;
	    auto recvIt = _cachePkt.find(packet_.channel); //缓存中是否存在包
	    if (recvIt == _cachePkt.end()) {
		cout << "channel not find in cache: " << packet_.channel << endl;
	        lastPacketRecvEnd = true;
	    }
	    cout << "packet_.headerType is RTMP_HEADER_MINIMUM" << endl;
	    if (lastPacketRecvEnd) { //一个message的第一个chunk
	        auto lastInfo = _channelPkt.find(packet_.channel); //是否已收到过包
		if (lastInfo == _channelPkt.end()) { //第一个包不能是type 3
		    cout << "第一个包不能是type3" << endl;
		    iRet = -1;
		    break;
		}
		packet_.hasAbsTimestamp = false;
		packet_.timestamp = _channelPkt[packet_.channel].timestamp;
		packet_.messageLength = _channelPkt[packet_.channel].messageLength;
		packet_.packetType = _channelPkt[packet_.channel].packetType;
		packet_.messageStreamId = _channelPkt[packet_.channel].messageStreamId;
		cout << "RTMP_HEADER_MINIMUM messageLength is: " << packet_.messageLength << endl;
                if (packet_.messageLength > _chunkSize) { //不支持第一个包大于128，可以考虑支持
		    cout << "messageLength 大于 _chunkSize" << endl;
		    iRet = -1;
		    break;
		} else {
		    recvSize = packet_.messageLength;
		    packet_.body = (char*)malloc(packet_.messageLength);
		    recvRet = recv(_fd, packet_.body, recvSize, 0);
		    if (recvRet != recvSize) {
		        iRet = -1;
			break;
		    }
                    packet_.bytesRead = recvRet;
		}
	    } else {
	        RTMPPacket &cachedPacket = _cachePkt[packet_.channel];
		int lastedBytes = cachedPacket.messageLength - cachedPacket.bytesRead;

		if (lastedBytes > _chunkSize) {
		    recvSize = _chunkSize;
		} else {
		    recvSize = lastedBytes;
		}
		/*recvRet = recv(_fd, cachedPacket.body + cachedPacket.bytesRead, recvSize, 0);
		cout << "recvRet: " << recvRet << endl;
		cout << "recvSize" << recvSize << endl;
		cout << "lastedBytes" << lastedBytes << endl;
		if (recvRet != recvSize) {
		    iRet = -1;
		    break;
		}*/
		int recvByteNum = 0;
		int toRead = recvSize;
		do {
		    recvRet = recv(_fd, cachedPacket.body + cachedPacket.bytesRead, toRead, 0);
		    recvByteNum += recvRet;
		    cachedPacket.bytesRead += recvRet;
                    toRead -= recvRet;
		} while (recvByteNum != recvSize);
		//cachedPacket.bytesRead += recvSize;

		cout << "cachedPacket.bytesRead: " << cachedPacket.bytesRead << endl;
		cout << "cachedPacket.messageLength" << cachedPacket.messageLength << endl;
		if (cachedPacket.bytesRead == cachedPacket.messageLength) {
		    packet_ = cachedPacket;
		    _cachePkt.erase(packet_.channel);
		    if (false == packet_.hasAbsTimestamp) {
		        packet_.timestamp += _channelPkt[packet_.channel].timestamp;
		    }
		    _channelPkt[packet_.channel] = packet_;
		} else {
		    packet_.bytesRead = cachedPacket.bytesRead;
		    packet_.messageLength = cachedPacket.messageLength;
		}
	    }
	} else {
	    if (packet_.headerType == RTMP_HEADER_LARGE) { //type 0
	        packet_.hasAbsTimestamp = true;
	    } else {
	        packet_.hasAbsTimestamp = false;
		cout << "channel is: " << packet_.channel << endl;
		auto lastIt = _channelPkt.find(packet_.channel);
		if (lastIt == _channelPkt.end()) { //第一个包的type必须是0
		    iRet = -1;
		    cout << "第一个包的type必须是0" << endl;
		    break;
		}
		auto lastThisCahnnelPkt = _channelPkt[packet_.channel];
		packet_.messageStreamId = lastThisCahnnelPkt.messageStreamId;
		if (packet_.headerType == RTMP_HEADER_SMALL) { //type 2
		    packet_.messageLength = lastThisCahnnelPkt.messageLength;
		    packet_.packetType = lastThisCahnnelPkt.packetType;
		}
	    }
		if (packet_.headerType == RTMP_HEADER_LARGE ||
				packet_.headerType == RTMP_HEADER_MEDIUM ||
				packet_.headerType == RTMP_HEADER_SMALL)
		{
		    char tmpChArr[3];
		    recvSize = recv(_fd, tmpChArr, 3, 0);
		    if (recvSize != 3) {
		        iRet = -1;
			break;
		    }
		    packet_.timestamp = AMF_DecodeInt24(tmpChArr);
		}
		if (packet_.headerType == RTMP_HEADER_LARGE ||
				packet_.headerType == RTMP_HEADER_MEDIUM) {
		    char tmpChArr[3];
		    recvSize = recv(_fd, tmpChArr, 3, 0);
		    if (recvSize != 3) {
		        iRet = -1;
			break;
		    }
                    packet_.messageLength = AMF_DecodeInt24(tmpChArr);
		    if (packet_.messageLength == 0) {
		        iRet = -1;
			break;
		    }
		    recvSize = recv(_fd, (char*)&tmp8, 1, 0);
		    if (recvSize != 1) {
		        iRet = -1;
			break;
		    }
                    packet_.packetType = tmp8;
		}
		if (packet_.headerType == RTMP_HEADER_LARGE) {
		    char tmpChArr[4];
		    recvSize = recv(_fd, tmpChArr, 4, 0);
		    if (recvSize != 4) {
		        iRet = -1;
			break;
		    }
                    packet_.messageStreamId = AMF_DecodeInt32LE(tmpChArr);
		}
		if (packet_.timestamp == 0xffffff) {
		    if (4 != recv(_fd, (char*)&packet_.timestampExtension, 4, 0)) {
		        iRet = -1;
			break;
		    }
		    packet_.timestampExtension = htonl(packet_.timestampExtension);
		}
		cout << "packet_.messageLength: " << packet_.messageLength << endl;
		packet_.body = (char*)malloc(packet_.messageLength);
		packet_.bytesRead = 0;
		if (_chunkSize < packet_.messageLength - packet_.bytesRead) {
		    recvSize = _chunkSize;
		} else {
		    recvSize = packet_.messageLength - packet_.bytesRead;
		}
		if (recvSize != 0) {
		    /*recvRet = recv(_fd, packet_.body + packet_.bytesRead, recvSize, 0);
		    if (recvSize != recvRet) {
		        iRet = -1;
			break;
		    }
                    packet_.bytesRead += recvRet;*/

		    int recvByteNum = 0;
                    int toRead = recvSize;
                    do {
                        recvRet = recv(_fd, packet_.body + packet_.bytesRead, toRead, 0);
                        recvByteNum += recvRet;
                        packet_.bytesRead += recvRet;
                        toRead -= recvRet;
                    } while (recvByteNum != recvSize);
		}
		cout << "packet_.bytesRead: " << packet_.bytesRead << endl;
		cout << "packet_.messageLength: " << packet_.messageLength << endl;
		if (packet_.bytesRead != packet_.messageLength) {
		    cout << "cache channel: " << packet_.channel << endl;
		    _cachePkt[packet_.channel] = packet_;
		    _cachePkt[packet_.channel].body = (char*)malloc(packet_.messageLength);
		    memcpy(_cachePkt[packet_.channel].body, packet_.body, packet_.bytesRead);
		} else {
		    if (false == packet_.hasAbsTimestamp) {
		        packet_.timestamp += _channelPkt[packet_.channel].timestamp;
		    }
		    _channelPkt[packet_.channel] = packet_;
		}
	}
    } while (0);

    return iRet;
}
