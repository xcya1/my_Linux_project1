#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "../server/comment.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

// 一个终端一个客户
//  客户端用进程,子进程会复制父进程

char currentUserName[128] = {0};


int main()
{
     // 服务器的地址
     unsigned int ip;
     // 把字符串IP转为网络（大端）字节序
     inet_pton(AF_INET, "192.168.217.128", &ip);
     struct sockaddr_in addr;
     addr.sin_family = AF_INET;   // IPV4地址族
     addr.sin_port = htons(9001); // 转成网络字节序的端口号
     addr.sin_addr.s_addr = ip;   // 网络字节序IP地址

     // 如果要通信，就需要创建socket，返回SOCKET的文件描述符
     // 参数1：地址族IPV4
     // 参数2：流式SOCKET(tcp协议)
     // 参数3：自动网络协议
     int fd = socket(AF_INET, SOCK_STREAM, 0);

     // 连接服务器
     // 参数1：用来通信的SOCEKT
     // 参数2：服务器的地址
     // 参数3：服务器的地址结构体长度
     if (-1 == connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
     {
          printf("连接服务器出错，服务器没有开或者断网了?\n");
     }
     while (1)
     {

          struct netNode msg;
          printf("1.注册 2.登录 3.修改密码 \n"); 
          char buf[8];
          scanf("%s", buf);
          if (strcmp(buf, "1") == 0) // ------------------------------------------注册----------------------------------------------------------
          {
               // 把通信用的结构体的类型设置为注册
               // 该结构体还有1023个字节没有使用（content）
               msg.type = REGISTER;

               // 定义一个用户结构体
               struct userInfo user;

               printf("请输入用户名\n");
               scanf("%s", user.name);
               printf("请输入密码\n");
               scanf("%s", user.pwd);
               printf("请输入手机号\n");
               scanf("%s", user.tel);
               memcpy(msg.content, &user, sizeof(user)); // msg.content字符串，名字就表地址 // 把结构体的内容全部发给服务器  // 把用户信息结构体里面的所有字节全部拷贝到通信结构体的CONTENT中

               write(fd, &msg, 1024);
               read(fd, &msg, 1024);
               if (msg.type == 102)
               {
                    printf("该用户名存在,请重新注册\n");
               }
               if (msg.type == OK)
               {
                    printf("注册成功\n");
               }
               if (msg.type == NO)
               {
                    printf("注册失败\n");
               }
               sleep(delay);
               system("clear");
          }
          else if (strcmp(buf, "2") == 0) //------------------------------- 登录--------------------------------------
          {
               struct userInfo user; // 局部变量，出作用域消亡
               printf("请输入用户名\n");
               scanf("%s", user.name);
               printf("请输入密码\n");
               scanf("%s", user.pwd);

               strcpy(currentUserName, user.name); // 记录单前登入用户

               msg.type = LOGIN;
               memcpy(msg.content, &user, sizeof(user));
               write(fd, &msg, 1024);

               read(fd, &msg, 1024);
               if (msg.type == OK)
               {
                    
                    printf("登入成功\n");
                     sleep(delay);
                     system("clear");//******


 

                    while (1)  //----------------------------------二级菜单-------------------------------------
                    {
                        
                    //------------------记数申请好友数和消息数------------------------------------------
                         int num1=0;
                         int num2=0;

                         struct strat_count p_one;
                         msg.type = count_friend_and_msg;                                        
                         strcpy(p_one.deng_lu_u, currentUserName); // 当前用户的名字
                         memcpy(msg.content, &p_one, sizeof(struct strat_count));
                         write(fd, &msg, 1024);
                         read(fd, &msg, 1024);
                         
                        struct re_num *p=(struct re_num*)msg.content;
                        num1=p->n1;
                        num2=p->n2;
                    //     printf("%d",num1);
                    //     printf("%d",num2); 
                         //------------------记数申请好友数和消息数------------------------------------------

                         printf("4.群聊 5.私聊 6.查看消息(%d未读消息) 7.申请加好友 8.查看好友申请(有%d个新申请) 9.退出登录 10.刷新\n",num1,num2);
                         char buf1[8];
                         scanf("%s", buf1);
                         if (strcmp(buf1, "4") == 0) //------------------------- 群聊 ------------------------------  //要都有用户进入聊天室才能有聊天显示，否则会程序会阻塞在大while scanf("%s", buf);那里，不会进入到子中
                         {
                              msg.type = ALL_CHAT;
                              
                              int pid = fork();
                              if (pid > 0)
                              {
                                   while (1)
                                   {
                                        printf("---请输入要发的消息(输入quit退出群聊)---:\n");
                                        scanf("%s", msg.content);
                                        if (strcmp("quit", msg.content) == 0) // 退出群聊
                                        {
                                             kill(pid, SIGKILL); // 杀死字进程
                                             break;              // 父进程结束        回到大while
                                        }
                                        else
                                        {
                                             write(fd, &msg, 1024);
                                        }
                                   }
                              }
                              else
                              {
                                   while (1)
                                   {
                                        char buf[1024];
                                        int ret = read(fd, buf, 1024);
                                        if (ret == 0 || ret == -1)
                                        {
                                             break;
                                        }
                                        printf("%s\n", buf);
                                        // struct netNode* pNode = (struct netNode*)buf;
                                        // printf("%s\n", pNode->content);
                                   }
                              }
                              system("clear"); //***************

                         }
                         else if (strcmp(buf1, "5") == 0) //---------------------------------私聊-----------------------------
                         {
                              msg.type = dan_CHAT;
                              struct dan_chat_node cn;

                              strcpy(cn.self_name, currentUserName);
                              printf("请输入对方的名字\n");
                              scanf("%s", cn.dest_name);

                              int pid = fork();
                              if (pid > 0)
                              {
                                   while (1)
                                   {
                                        // char strr[12];

                                        printf("-----请输入想说的话 (输入quit退出私聊)-----\n");

                                        scanf("%s", cn.chat_text);
                                        if (strcmp("quit", cn.chat_text) == 0) // 退出聊天
                                        {

                                             kill(pid, SIGKILL); // 杀死字进程
                                             break;              // 父进程结束        回到大while
                                        }
                                        else
                                        {
                                             memcpy(msg.content, &cn, sizeof(cn));
                                             write(fd, &msg, 1024);
                                        }
                                   }
                              }
                              else
                              {
                                   while (1)
                                   {

                                        char buf[1024];
                                        int ret = read(fd, buf, 1024);
                                        if (ret != 0 || ret != -1)
                                        {
                                             printf("%s\n", buf);
                                        }
                                        else
                                        {
                                             break;
                                        }
                                        // printf("-----输入quit退出私聊-----\n");
                                        // printf("请输入对方的名字\n");
                                   }
                              }
                              system("clear");//*************
                         }
                         else if (strcmp(buf1, "6") == 0) //----------------------------查看消息-------------------------------
                         {
                              msg.type = GET_CHAT; // 消息类型为获取消息
                              struct cha_kan_msg p6;
                              strcpy(p6.deng_lu_u, currentUserName); // 当前用户的名字
                              strcpy(p6.flag, "no");
                              memcpy(msg.content, &p6, sizeof(struct cha_kan_msg));
                              write(fd, &msg, 1024);
                              read(fd, &msg, 1024);
                              int num = *(int *)msg.content;
                              printf("服务器说我有%d条消息\n", num);
                              printf("-----------------未读消息-----------------\n");
                              printf("发送者    接收者     内容     时间     状态\n");
                              for (size_t i = 0; i < num; i++)
                              {
                                   // 读取服务器发来的具体消息内容
                                   read(fd, &msg, MSG_BUF_SIZE);
                                   struct re_msg *pr = (struct re_msg *)msg.content;
                                   printf("%s       %s     %s  %s  %s\n", pr->send_name, pr->receive_name, pr->chat_text, pr->datatime, pr->flag);
                                   if ((i + 1) % 5 == 0)
                                   {
                                        printf("\n");
                                   }
                              }
                              printf("1.全部读取\n");
                              printf("2.退出\n");
                              char key2[8];
                              scanf("%s", key2);
                              if (strcmp(key2, "1") == 0)
                              {
                                   
                                   msg.type = chu_li_msg;
                                   struct chuli_msg pp;
                                   strcpy(pp.deng_lu_u, currentUserName); // 当前用户的名字
                                   strcpy(pp.old_flag, "no");
                                   strcpy(pp.new_flag, "yes");

                                   memcpy(msg.content, &pp, sizeof(struct chuli_msg));
                                  write(fd, &msg, 1024);
                                  read(fd, &msg, 1024);
                                   if (msg.type == OK)
                                   {
                                        printf("所有消息已读\n");
                                   }

                              }
                              sleep(delay);
                              system("clear");//**********************************
                         }
                         else if (strcmp(buf1, "7") == 0) //------------------------申请加好友---------------
                         {
                              msg.type = shen_qing_friend;
                              struct shen_qingfred sqf;
                              printf("输入要加的好友名\n");
                              scanf("%s", sqf.duifang_name);
                              strcpy(sqf.sq_name, currentUserName);
                              strcpy(sqf.zhuang_tai, "no");

                              memcpy(msg.content, &sqf, sizeof(sqf));

                              write(fd, &msg, 1024);
                              read(fd, &msg, 1024);
                              if (msg.type == OK)
                              {
                                   printf("申请加好友成功\n");
                              }
                              else
                              {
                                   printf("服务器中没有该用户\n");
                              }
                              sleep(delay);
                              system("clear");//******
                         }
                         else if (strcmp(buf1, "8") == 0) //------------------------查看好友和处理好友---------------
                         {
                              msg.type = can_kan_frend;
                              struct chao_kan_fred ckf;
                              strcpy(ckf.deng_lu_name, currentUserName); // 当前用户的名字
                              strcpy(ckf.zhuang_tai, "no");
                              memcpy(msg.content, &ckf, sizeof(ckf));
                              write(fd, &msg, MSG_BUF_SIZE); // 给服务器发送一个获取信息的消息
                              read(fd, &msg, MSG_BUF_SIZE);  // 获得服务器返回的记录数量
                              int num = *(int *)msg.content;
                              printf("服务器说我有%d条消息\n", num);
                              for (size_t i = 0; i < num; i++)
                              {
                                   // 读取服务器发来的具体消息内容
                                   read(fd, &msg, MSG_BUF_SIZE);
                                   struct shen_qingfred *mr = (struct shen_qingfred *)msg.content;
                                   printf("申请人 状态\n");
                                   printf("%s     %s\n", mr->sq_name, mr->zhuang_tai);
                              }
                              printf("------------------------\n");
                              printf("1.处理申请\n");
                              printf("2.退出\n");
                              printf("------------------------\n");
                              char key[8];
                              scanf("%s", key);
                              if (strcmp(key, "1") == 0)
                              {
                                   msg.type = chu_li_frend;
                                   struct accepet_friend p;
                                   printf("请输入同意的申请人的姓名\n");
                                   scanf("%s", p.tonyi_name);
                                   strcpy(p.c_deng_lu_ren, currentUserName);
                                   memcpy(msg.content, &p, sizeof(struct accepet_friend));
                                   write(fd, &msg, MSG_BUF_SIZE);

                                   read(fd, &msg, MSG_BUF_SIZE);
                                   if (msg.type == OK)
                                   {
                                        printf("添加好友成功\n");
                                   }
                                   else
                                   {
                                        printf("没有该用户申请\n");
                                   }
                              }
                              else
                              {
                              }
                              sleep(delay);
                              system("clear");//******

                         }
                         else if (strcmp(buf1, "9") == 0) //------------------------退出登入返回一级菜单---------------
                         {
                              msg.type = tui_chu_deng_lu;
                               write(fd, &msg, MSG_BUF_SIZE);
                              break;
                         }
                         else if (strcmp(buf1, "10") == 0)
                         {



                         }





                    } //---------------------------------------------------二级菜单------------------------------------------------
               }
               else
               {
                    printf("用户名或密码不正确\n");
               }
                sleep(delay);
               system("clear");//****

          }

          else if (strcmp(buf, "3") == 0) // ------------------------------------修改密码--------------------------------
          {
               int i = 0;
          N:
               printf("-----------------\n");
               printf("1.登入修改密码\n");
               printf("2.退出\n");
               printf("-----------------\n");
               scanf("%d", &i);
               if (i == 1)
               {
                    struct gai_mi pwd;
                    // struct userInfo user; // 局部变量，出作用域消亡
                    printf("请输入用户名\n");
                    scanf("%s", pwd.yuan_name);
                    printf("请输入原密码\n");
                    scanf("%s", pwd.yuan_pwd);
                    printf("请输入新密码\n");
                    scanf("%s", pwd.xin_pwd);

                    msg.type = GAI_MI;
                    memcpy(msg.content, &pwd, sizeof(pwd));
                    write(fd, &msg, 1024);

                    read(fd, &msg, 1024);
                    if (msg.type == OK)
                    {
                         printf("修改密码成功\n");
                    }
                    else
                    {
                         printf("用户名或密码不正确，请重新登入\n");
                         goto N;
                    }
               }
               else
               {
               }
                sleep(delay);
               system("clear");//****

          }
     }

     return 0;
}