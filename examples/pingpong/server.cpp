#include <iostream>
#include <mutex>
#include "eventloop.h"
#include "WrapTCPService.h"

std::mutex g_mutex;
int total_recv = 0;
int total_client_num = 0;

void onSessionClose(TCPSession::PTR session)
{
    g_mutex.lock();
    total_client_num--;
    g_mutex.unlock();
}

int onSessionMsg(TCPSession::PTR session, const char* buffer, int len)
{
    session->send(buffer, len);
    g_mutex.lock();
    total_recv += len;
    g_mutex.unlock();
    return len;
}

int main(int argc, char **argv)
{
    int thread_num = atoi(argv[1]);
    int port_num = atoi(argv[2]);

    WrapServer::PTR server = std::make_shared<WrapServer>();

    server->setDefaultEnterCallback([](TCPSession::PTR session){
        session->setCloseCallback(onSessionClose);
        session->setDataCallback(onSessionMsg);

        g_mutex.lock();
        total_client_num++;
        g_mutex.unlock();
    });

    server->startListen(port_num);
    server->startWorkThread(thread_num);

    EventLoop mainLoop;
    mainLoop.restoreThreadID();

    while (true)
    {
        mainLoop.loop(1000);
        g_mutex.lock();
        cout << "total recv : " << (total_recv / 1024) / 1024 << " M /s, of client num:" << total_client_num << endl;
        total_recv = 0;
        g_mutex.unlock();
    }
}