#pragma once

#include <string>

const int RTMP_default_chunk_size = 128;        //Ä¬ÈÏ¿é³¤¶È£¬²»°üÀ¨Êý¾ÝÍ·£¬×îÐ¡128£¬×î´ó0xffff
const int RTMP_better_chunk_size = 0x1000;  //128Ì«Ð¡
const int RTMP_max_chunk_size = 0xffff;
const int RTMP_buffer_cache_size = 16384;       //tcp»º´æ
const int RTMP_default_buffer_ms = 3000;

const int RTMP_channel_control = 0x02;
const int RTMP_channel_invoke = 0x03;
const int RTMP_channel_SendLive = 0x04;
const int RTMP_channel_SendPlayback = 0x05;
const int RTMP_channel_audioVideo = 0x08;

const std::string RTMP_CMD__result = "_result";
const std::string RTMP_CMD_connect = "connect";
const std::string RTMP_CMD_publish = "publish";
const std::string RTMP_CMD_pause = "pause";
const std::string RTMP_CMD_seek = "seek";
const std::string RTMP_CMD_createStream = "createStream";
const std::string RTMP_CMD_play = "play";
const std::string RTMP_CMD_releaseStream = "releaseStream";
const std::string RTMP_CMD_FCPublish= "FCPublish";
const std::string RTMP_CMD_FCUnpublish= "FCUnpublish";
const std::string RTMP_CMD_FCSubscribe = "FCSubscribe";
const std::string RTMP_CMD_onBWDone = "onBWDone";
const std::string RTMP_CMD_onStatus = "onStatus";
const std::string RTMP_CMD_onFCSubscribe = "onFCSubscribe";
const std::string RTMP_CMD_onFCUnsubscribe = "onFCUnsubscribe";
const std::string RTMP_CMD__checkbw = "_checkbw";
const std::string RTMP_CMD__onbwcheck = "_onbwcheck";
const std::string RTMP_CMD__onbwdone = "_onbwdone";
const std::string RTMP_CMD__error = "_error";
const std::string RTMP_CMD_close = "close";
const std::string RTMP_CMD_code = "code";
const std::string RTMP_CMD_level = "level";
const std::string RTMP_CMD_description = "description";
const std::string RTMP_CMD_deleteStream = "deleteStream";
const std::string RTMP_CMD_playlist_ready = "playlist_ready";
const std::string RTMP_CMD_getStreamLength = "getStreamLength";

const std::string RTMP_status_NetStream_Failed = "NetStream.Failed";
const std::string RTMP_status_NetStream_Play_Failed = "NetStream.Play.Failed";
const std::string RTMP_status_NetStream_Play_StreamNotFound = "NetStream.Play.StreamNotFound";
const std::string RTMP_status_NetConnection_Connect_InvalidApp = "NetConnection.Connect.InvalidApp";
const std::string RTMP_status_NetConnection_Connect_Closed = "NetConnection.Connect.Closed";
const std::string RTMP_status_NetStream_Play_start = "NetStream.Play.Start";
const std::string RTMP_status_NetStream_Play_Complete = "NetStream.Play.Complete";
const std::string RTMP_status_NetStream_Play_Stop = "NetStream.Play.Stop";
const std::string RTMP_status_NetStream_Play_Reset = "NetStream.Play.Reset";
const std::string RTMP_status_NetStream_Seek_Notify = "NetStream.Seek.Notify";
const std::string RTMP_status_NetStream_Pause_Notify = "NetStream.Pause.Notify";
const std::string RTMP_status_NetStream_Play_PublishNotify = "NetStream.Play.PublishNotify";
const std::string RTMP_status_NetStream_Play_UnpublishNotify = "NetStream.Play.UnpublishNotify";
const std::string RTMP_status_NetStream_Publish_Start = "NetStream.Publish.Start";
const std::string RTMP_status_NetStream_Publish_Rejected = "NetStream.Publish.Rejected";
const std::string RTMP_status_NetStream_Publish_Denied = "NetStream.Publish.Denied";
const std::string RTMP_status_NetConnection_Connect_Rejected = "NetConnection.Connect.Rejected";

const std::string connect_CMD_app = "app";      //rtmp urlÖÐ¶Ë¿Úºóµ½ÏÂÒ»¸ö/ÎªÖ¹µÄ×Ö·û´®£¬ÉÏÃæÀý×ÓÖÐµÄtestapp
const std::string connect_CMD_flashver = "flashver";
const std::string connect_CMD_swfUrl = "swfUrl";
const std::string connect_CMD_tcUrl = "tcUrl";
const std::string connect_CMD_fpad = "fpad";
const std::string connect_CMD_capabilities = "capabilities";
const std::string connect_CMD_audioCodecs = "audioCodecs";
const std::string connect_CMD_videoCodecs = "videoCodecs";
const std::string connect_CMD_videoFunction = "videoFunction";

enum RTMP_PACKET_TYPE
{
        RTMP_PACKET_TYPE_CHUNK_SIZE         = 0x01,
        RTMP_PACKET_TYPE_ABORT              = 0x02,
        RTMP_PACKET_BYTES_READ_REPORT       = 0x03,
        RTMP_PACKET_TYPE_CONTROL            = 0x04,
        RTMP_PACKET_TYPE_SERVER_BW          = 0x05,
        RTMP_PACKET_TYPE_CLIENT_BW          = 0x06,
        RTMP_PACKET_TYPE_AUDIO              = 0x08,
        RTMP_PACKET_TYPE_VIDEO              = 0x09,
        RTMP_PACKET_TYPE_FLEX_STREAM_SEND   = 0x0F,     //amf3
        RTMP_PACKET_TYPE_FLEX_SHARED_OBJECT = 0x10,//amf3
        RTMP_PACKET_TYPE_FLEX_MESSAGE       = 0x11,             //amf3 message
        RTMP_PACKET_TYPE_INFO               = 0x12,//onMetaData info is sent as such
        RTMP_PACKET_TYPE_SHARED_OBJECT      = 0x13,
        RTMP_PACKET_TYPE_INVOKE             = 0x14,//amf0 message
        RTMP_PACKET_TYPE_FLASH_VIDEO        = 0x16
};

enum RTMP_CONTROL_TYPE
{
        RTMP_CTRL_INVALID             = -1,
        RTMP_CTRL_STREAMBEGIN         = 0,
        RTMP_CTRL_STREAMEOF           = 1,
        RTMP_CTRL_STREAMDRY           = 2,
        RTMP_CTRL_SET_BUFFERLENGHT    = 3,      //8×Ö½Ú  Ç°4¸ö×Ö½Ú±íÊ¾Á÷ID ºóËÄ¸ö×Ö½Ú±íÊ¾»º´æ³¤¶È
        RTMP_CTRL_STREAM_IS_RECORDED  = 4,
        RTMP_CTRL_PING_REQUEST        = 6,
        RTMP_CTRL_PING_RESPONSE       = 7,
        RTMP_CTRL_STREAM_BUFFER_EMPTY = 31,
        RTMP_CTRL_STREAM_BUFFER_READY = 32
};

struct RTMPChunkBasicHeader
{
        //´ó¶Ë
        unsigned char fmt;//°ü¸ñÊ½£º2bit
        unsigned char csId;//¿éÁ÷ID channel id £º6bit.0±íÊ¾2×Ö½ÚÐÎÊ½ 1±íÊ¾3×Ö½ÚÐÎÊ½ 2±»±£Áô 3-63¿ÉÖ±½ÓÐ´Èë¸ÃÖµ
        //Èç¹ûÊÇ¿ØÖÆÏûÏ¢£¬¿éÁ÷IDÎª2
        //StreamID=(ChannelID-4)/5+1
        //02 Ping ºÍByteReadÍ¨µÀ  control
        //03 InvokeÍ¨µÀ ÎÒÃÇµÄconnect() publish()ºÍ×Ô×ÖÐ´µÄNetConnection.Call() Êý¾Ý¶¼ÊÇÔÚÕâ¸öÍ¨µÀµÄ
        //04  AudioºÍVidioÍ¨µÀ
        //05 06 07 ·þÎñÆ÷±£Áô,¾­¹Û²ìFMS2ÓÃÕâÐ©ChannelÒ²ÓÃÀ´·¢ËÍÒôÆµ»òÊÓÆµÊý¾Ý
        unsigned char csId2byte;//2×Ö½ÚÐÎÊ½£¬¸Ã×Ö¶ÎµÄÖµÎªÁ÷ID-64£»
        unsigned short csId3byte;//3×Ö½ÚÐÎÊ½£¬¸Ã×Ö¶ÎµÄÖµÎªÁ÷ID-64;
};

enum RTMP_HEADER_TYPE
{
        RTMP_HEADER_LARGE,
        RTMP_HEADER_MEDIUM,
        RTMP_HEADER_SMALL,
        RTMP_HEADER_MINIMUM
};

//¸ù¾ÝBasicHeaderµÄfmtµÄÖµÓÐËÄÖÖ¸ñÊ½
//ÀàÐÍ0ÓÃÔÚÁ÷ÆðÊ¼Î»ÖÃ»òÊ±¼äÖØÖÃÊ±
//ÀàÐÍ1²»°üº¬ÏûÏ¢Á÷ID£¬ÓëÉÏÒ»¸ö¿éÍ¬ÑùµÄÁ÷ID
//ÀàÐÍ2²»°üº¬ÏûÏ¢³¤¶È£¬ÓëÉÏÒ»¸ö¿éÍ¬ÑùµÄÁ÷IDºÍÁ÷³¤¶È
//ÀàÐÍ3Ã»ÓÐÈÎºÎ×Ö¶Î
//chunk×é×°³ÉÏûÏ¢,chunkµÄÊý¾Ý¾ÍÊÇÏûÏ¢

struct RTMPMessageHeader
{
        unsigned char messageType;      //ÏûÏ¢ÀàÐÍ 1-6Îª¿ØÖÆÏûÏ¢ Í¬chunkµÄmessageType
        unsigned char payloadLength[3];//ÓÐÐ§¸ºÔØ³¤¶È
        unsigned int timestamp;         //Ê±¼ä
        unsigned char streamId[3];      //ÏûÏ¢Á÷ID Ð­Òé¿ØÖÆÏûÏ¢Ê±Îª0¡¢chunkµÄÁ÷idÎª2¡£  ÒôÆµÎª1 ÊÓÆµÎª2
};

struct RTMPPacket
{
        RTMPPacket() {
	    timestamp = 0;
	    messageLength = 0;
	    bytesRead = 0;
	    body = NULL;
	}
        RTMP_HEADER_TYPE headerType;

        bool    hasAbsTimestamp;
        int channel;

        RTMPChunkBasicHeader basicHeader;
        unsigned int timestamp;                         //0 1 2 0ÎªÊ±¼ä 1¡¢2ÎªÇ°Ò»¿éºÍµ±Ç°¿éµÄÊ±¼ä²î£¬Èç¹û´óÓÚ0xffffff£¬ÔòÕâÀïµÈÓÚ0xffffff£¬±íÃ÷ÓÃÀ©Õ¹Ê±¼ä 3byte
        unsigned int messageLength;                     //0 1   ÏûÏ¢³¤¶È                3byte
        unsigned char packetType;                       //0 1   ÏûÏ¢ÀàÐÍ                1byte
        unsigned int  messageStreamId;                  //0                     Ð¡¶Ë¡£ 4byte
        unsigned int timestampExtension;                        //À©Õ¹Ê±¼ä
        unsigned int bytesRead;
        char            *body;
};

