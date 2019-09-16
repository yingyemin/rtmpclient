#pragma once

#include <string>
#include <map>

#include "define.h"

using namespace std;

class RtmpClient
{
public:
    RtmpClient(const string& url, const string& filename);
    ~RtmpClient();

public:
    int parseUrl();
    int initSocket();
    int handShake();
    int connect();
    int createStream();
    int publish();
    int stopPublish();
    int play();
    int pause();
    int seek();
    int close();
    int sendFlvFile();

private:
    int sendPacket(RTMPPacket& packet);
    int socketSend(const char * buf, int len);
    int recvChunk(RTMPPacket& packet_);
    int recvPacket(RTMPPacket& packet);

private:
    string _url;
    string _filename;
    string _ip;
    string _proto;
    string _app;
    string _streamname;
    int    _port;
    int    _fd;
    int    _numInvokes;
    int    _chunkSize;
    int    _streamid;

    map<int, string> _method; //_transactionId-->method
    map<int, RTMPPacket> _cachePkt; //缓存正在接收的channel_id对应的包数据
    map<int, RTMPPacket> _channelPkt; //保存接受完成的channel_id对应的包数据
};

