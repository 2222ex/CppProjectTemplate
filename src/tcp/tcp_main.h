#pragma
#include "base/stdafx.h"

#include "tcp_client.h"
#include "tcp_server.h"

void tcp_main()
{
    std::thread t1(run_tcp_server);
    std::thread t2(run_tcp_client);

    t2.join();
    t1.join();
}