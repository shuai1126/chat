#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <iostream>
#include <thread>
using std::cout,std::endl;

int ss;
struct sockaddr_in client_addr;
socklen_t length = sizeof(client_addr);
int conns[2] = {};//定义数组来存放套接字
int z = 0;
void thread_fn()
{
    //成功返回非负，出错返回-1
    int conn = accept(ss, (struct sockaddr*)&client_addr, &length);
    if( conn < 0 )
    {
        perror("connect");
        exit(1);
    }
    conns[z] = conn;//保存到临时数组
    z++;

    fd_set rfds;
    struct timeval tv;//获取系统时间
    int retval, maxfd;
    while(1)
    {
        FD_ZERO(&rfds); //可读文件描述符集合清空
        FD_SET(0, &rfds);//把标准输入的文件描述符加入到集合中
        maxfd = 0;
        FD_SET(conn, &rfds);//把当前连接的文件描述符加入到集合中
        if(maxfd < conn)//找出文件描述符集合中最大的文件描述符
        {
            maxfd = conn;
        }
        tv.tv_sec = 5;//超时时间5秒
        tv.tv_usec = 0;

        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if(retval == -1)
        {
            cout<<"select出错，客户端程序退出"<<endl;
            break;
        }
        else if(retval == 0)
        {
            continue;
        }
        else
        {
            /*客户端发来了消息*/
            if(FD_ISSET(conn,&rfds))//判断conn是否在rfds中如果在返回非零，不再返回0
            {
                char buffer[1024];
                memset(buffer, 0 ,sizeof(buffer));//清空buffer
                int len = recv(conn, buffer, sizeof(buffer), 0);//把接收到的数据存放于buffer中
                if(strcmp(buffer, "exit\n") == 0)//如果接受到的是空
                    break;
                cout<< "来自客户端的消息："<<buffer<<endl;
            }
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds))
            {
                char buf[1024];
                fgets(buf, sizeof(buf), stdin);//每次读取一行数据存放在buf中
                for(int i=0; i<z; i++)
                {
                    send(conns[i], buf, sizeof(buf), 0);
                    cout <<  "发送成功!" << endl;
                }
            }
        }
    }
    close(conn);
}

int main()
{
    ss = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(1234);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1)
    {
        perror("bind");
        exit(1);
    }
    if(listen(ss, 20) == -1)
    {
        perror("listen");
        exit(1);
    }
    std::thread t(thread_fn);
    std::thread t1(thread_fn);
    t.join();
    t1.join();
    close(ss);
    return 0;
}