#include "server.h"




struct localAddr *netToLocal(struct sockaddr_in *pAddr)
{
    struct localAddr *p = malloc(sizeof(struct localAddr)); // malloc在堆上的，出函数内存不会消亡  
    p->port = ntohs(pAddr->sin_port); 
    inet_ntop(AF_INET, &pAddr->sin_addr.s_addr, p->szIP, 16);
    return p;
}

struct localAddr *la;
void client_addr_zhuanghuan(struct msgInfo *p)
{
        la=netToLocal(&p->addr);
}


// 定义一个1024个元素的事件结构体数组
struct epoll_event evtArr[1024];

int init_TcpSocket(char *s_IP, u_int16_t prot) // 客户端连接服务器 创建监听套接字
{
     unsigned int ip;
     // 把字符串IP转为网络（大端）字节序
     inet_pton(AF_INET, s_IP, &ip);
     struct sockaddr_in addr;
     addr.sin_family = AF_INET;   // IPV4地址族
     addr.sin_port = htons(prot); // 转成网络字节序的端口号
     addr.sin_addr.s_addr = ip;   // 网络字节序IP地址

     // 如果要通信，就需要创建socket，返回SOCKET的文件描述符
     // 参数1：地址族IPV4
     // 参数2：流式SOCKET(tcp协议)
     // 参数3：自动网络协议
     int fd = socket(AF_INET, SOCK_STREAM, 0);

     // 绑定电脑上的一个端口
     // 参数1：socket
     // 参数2：网络地址，为了满足历史遗留问题，需要把地址转为struct sockaddr *
     // 参数3：结构体的长度
     bind(fd, (struct sockaddr *)&addr, sizeof(addr));

     // 启动监听，监听有没有人连链接我，最多允许5个客户端同时排队连接，超过的直接返回连接错误
     listen(fd, 5);
     return fd;
}

int listenFd = 0;

int init_server(char *s_IP, u_int16_t prot)
{
     listenFd = init_TcpSocket(s_IP, prot);
     // epoll类似于select，是帮助我们检测套接字的
     // 创建一个epoll对象，参数为epoll对象要管理的套接字数量
     int epObj = epoll_create(1024);

     // 定义一个事件结构体，里面需要填写我们关心的事件，例如可读事件/可写事件等。。。
     struct epoll_event ee;
     ee.events = EPOLLIN;   // 填写我们关心的事件，EPOLLIN是可读事件
     ee.data.fd = listenFd; // 填写需要关心可读事件的套接字，这里填写的是监听套接字
     // 把监听套接字和它对应的事件结构体加入到EPOLL对象中进行管理（检测）
     epoll_ctl(epObj, EPOLL_CTL_ADD, listenFd, &ee);

     return epObj;
}

//服务器处理事件
void server_handl_eEvent(int epObj, void (*s_print)(char *,struct localAddr *,int),void(*pdeng_lu_client)(int)) // 参数2为一个函数地址(就是函数名)
{
     while (1)
     {
          // epoll_wait等待epObj所管理的套接字上发生了感兴趣事件，如果某个套接字发生了对应的事件，epoll_wait会把发生事件的套接字存入数组中
          // 该套接字对应的事件结构体存入到第二个参数的数组中
          // 返回值：返回当前epoll对象中有多少个fd发生了可读事件，也就此时有多少个fd对应的结构体被计入到了第二个参数的数组中
          int ret = epoll_wait(epObj, evtArr, 1024, -1);

          for (size_t i = 0; i < ret; i++)
          {
               if (evtArr[i].data.fd == listenFd) // 如果套接字等于监听套接字，需要接受客户端连接
               {
                    // 用来存放客户端的地址
                    struct sockaddr_in client_addr;
                    // 客户端的地址长度
                    socklen_t len = sizeof(client_addr);
                    // 接受客户端的连接并返回新的用来和客户端通信的套接字
                    // 因为fdNew是一个通信套接字，它上面也有可能产生可读事件。例如：客户端发消息过来
                    // 所以也需要把它放到集合里让select监控
                    int fdNew = accept(listenFd, (struct sockaddr *)&client_addr, &len);

                    if(fdNew == -1)
                    {
                        printf("客户端连接错误\n");      
                    }
                    else
                    {

                    // pdeng_lu_client(fdNew,1);    //函数指针的引用

                    struct epoll_event ee;   //局部变量
                    ee.events = EPOLLIN; // 填写我们关心的事件，EPOLLIN是可读事件
                    ee.data.fd = fdNew;  // 填写需要关心可读事件的套接字，这里填写的是监听套接字
                    // 通信套接字和它对应的事件结构体加入到EPOLL对象中进行管理（检测）
                    epoll_ctl(epObj, EPOLL_CTL_ADD, fdNew, &ee);

                    struct msgInfo *p=malloc(sizeof(struct msgInfo));
                    p->addr=client_addr;
                    client_addr_zhuanghuan(p);   //把网地址转换为本机地址的函数

                    }
               }
               else // 通信套接字 //进行通信 收
               {
                    char buf[1024];
                    recv(evtArr[i].data.fd, buf, 1024, 0);   //最后一个参数默认为0   或用read(evtArr[i].data.fd,buf,1024);
                    if (ret == 0)
                    {
                         // 因为客户端已经被关闭，所以这边也需要被关闭，
                         close(evtArr[i].data.fd);    //套接字也是文件标识符（文件），不用也要关闭

                         //再把套接字数组中的那删除，不进行通信
                         // 定义一个事件结构体，里面需要填写我们关心的事件，例如可读事件/可写事件等。。。
                         struct epoll_event ee;           //在作用域内，为局部变量
                         ee.events = EPOLLIN;            // 填写我们关心的事件，EPOLLIN是可读事件
                         ee.data.fd = evtArr[i].data.fd; // 填写需要关心可读事件的套接字，这里填写的是监听套接字
                         
                         // 要从epoll中移除掉该套接字
                         epoll_ctl(epObj, EPOLL_CTL_DEL, evtArr[i].data.fd, &ee);  //参数2为删除 把通信套接字和它对应的事件结构体加入到EPOLL对象中进行管理（检测

                         pdeng_lu_client(evtArr[i].data.fd);   //函数指针的引用,从客户数组中删除一个客户

                         printf("有一个客户端被关了,我们把它的套接字关闭,同时从epoll中移除了它\n");
                    }
                    else  
                    {
                         s_print(buf,la,evtArr[i].data.fd); // 调用外面传入的函数，函数指针的引用，打印服务器接收的数据
                    }  
                    
               }
               
          }
     }
}
