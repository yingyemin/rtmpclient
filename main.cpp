#include <iostream>

using namespace std;

#include "rtmpclient.h"

void check(int code)
{
    if (code != 0) {
        exit(0);
    }
}

int main(int argc, char** argv)
{
    RtmpClient client(argv[1], argv[2]);
    cout << "parse url" << endl;
    check(client.parseUrl());
    cout << "init socket" << endl;
    check(client.initSocket());
    cout << "handshake" << endl;
    check(client.handShake());
    cout << "connect" << endl;
    check(client.connect());
    //cout << "publish" << endl;
    //check(client.publish());
    //cout << "send flv file" << endl;
    //check(client.sendFlvFile());
    
    cout << "play" << endl;
    check(client.play());
    return 0;
}
