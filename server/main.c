#include <stdio.h>
#include "server.h"
#include "comment.h"
#include "data_base.h"

// 服务器读出数据，存在另外一个人的表中，那人登入先读表中的数据

// 定义个记录个在线的用户的结构体数组，数据里要有名字，fd

// int clientFdArr[1024]; // 用于存登入客户的fb，以便于服务器转发遍历
// int connected = 0;     // 下标

sqlite3 *pDB = NULL; // 数据库结构体指针，一般绿色的字体表结构体

struct zai_xiang_yonhu deng_lu_user[100];
int zaixiang_count = 0;

// void add_or_Delet_NewClient(int clientFd, int isAdd)
// {
//      if (isAdd == 1) // 添加
//      {
//           // printf("handleEvent收到连接事件,调用添加用户的函数,总用户数更新为：%d\n", connected);
//           clientFdArr[connected++] = clientFd; // 先用后加
//      }
//      else // 删除
//      {
//           // 还需要从客户端数组中删除该套接字FD
//           for (size_t i = 0; i < connected; i++)
//           {
//                // 判断被失效的套接字是不是客户端数组中的第i个元素，如果是的就从数组中删除它
//                if (clientFd == clientFdArr[i])
//                {
//                     // 从要删除的位置的后一个开始，逐个往前移动
//                     for (int j = i + 1; j < connected; j++) // i为删除位置
//                     {
//                          printf("handleEvent发现有套接字失效,删除了失效套接字\n");
//                          clientFdArr[j - 1] = clientFdArr[j];
//                     }
//                     // 总数量-1
//                     connected--;
//                }
//           }
//      }
// }

void Delet_tui_chu_Client(int clientFd)
{
     for (size_t i = 0; i < zaixiang_count; i++)
     {
          // 判断被失效的套接字是不是客户端数组中的第i个元素，如果是的就从数组中删除它
          if (clientFd == deng_lu_user[i].fd)
          {
               // 从要删除的位置的后一个开始，逐个往前移动
               for (int j = i + 1; j < zaixiang_count; j++) // i为删除位置
               {
                    printf("handleEvent发现有套接字失效,删除了失效套接字\n");
                    deng_lu_user[j - 1] = deng_lu_user[j];
               }
               // 总数量-1
               zaixiang_count--;
          }
     }
     printf("quit\n");
}

void sever_printf(char *pBuf, struct localAddr *la, int clientfd)
{
     // printf("用户IP:%s 端口:%u发来数据:%s\n", la->szIP, la->port, pBuf);
     struct netNode *p_msg = (struct netNode *)pBuf;

     if (p_msg->type == REGISTER) // ----------------------------------注册---------------------------------------
     {
          struct userInfo *p_user = (struct userInfo *)p_msg->content;
          // printf("有客户端发来注册消息，用户名:%s 密码:%s 电话:%s\n", p_user->name, p_user->pwd, p_user->tel);
          char sql[256];
          sprintf(sql, "select name from user where name='%s'", p_user->name);
          struct queryResult *pQR = query_db(pDB, sql);  
          if (pQR->row != 0)
          {
               p_msg->type = 102; // 该名被注册过了
               goto T;
          }

          char buf1[1024];
          sprintf(buf1, "INSERT INTO user VALUES ('%s','%s','%s')", p_user->name, p_user->pwd, p_user->tel);
          char *pSqlInsert = buf1;
          if (1 == exec_sql(pDB, pSqlInsert))
          {
               // printf("插入数据成功\n");
               p_msg->type = OK; // 注册成功
               char sql[256];
               sprintf(sql, "create table %s_chat(send,receive,text,time1 DATETIME,flag)", p_user->name); //----------------聊天表
               exec_sql(pDB, sql);

               char sql2[256];
               sprintf(sql2, "create table %s_sqf(shen_qing_ren,zhuan_tai)", p_user->name); //--------申请好友表----
               exec_sql(pDB, sql2);

               char sql3[256];
               sprintf(sql3, "create table %s_friend(w_friend)", p_user->name); //--------好友表-----
               exec_sql(pDB, sql3);
          }
          else
          {
               // printf("插入数据失败\n");
               p_msg->type = NO;
          }
     T:
          write(clientfd, p_msg, 1024);
     }
     else if (p_msg->type == LOGIN) // -------------------------------登录---------------------------------
     {
          // 把通信结构题里面的内容还原成用户信息
          // 告诉电脑地址里面不是字符，而是一个用户，所以要转成用户信息类型的地址
          struct userInfo *p_user = (struct userInfo *)p_msg->content;
          //  printf("收到用户登录消息,我应该到这里查询数据库，看看里面是否有用户名是：%s 密码是：%s的用户\n", p_user->name,p_user->pwd);
          char sql[256];
          // 构建查询用户名和密码的SQL语句p_user
          sprintf(sql, "select name pwd from user where name='%s' and pwd='%s'", p_user->name, p_user->pwd); //"select *"表所有列用*
                                                                                                             // 表中列名 列名      表名       列名           列名
          // 在数据库pDB中执行SQL查询语句
          struct queryResult *pQR = query_db(pDB, sql);
          // printf("%d\n",pQR->row);
          // 如果一条记录都没有，说明用户传过来的用户名和密码在数据库中不存在
          if (pQR->row == 0) // 根据row和col的值来判断用户存不存在，存在行列中必有一个值
          {
               // printf("用户名或者密码错误\n");
               p_msg->type = NO;
          }
          else
          {
               // printf("登录成功\n");
               p_msg->type = OK;
               //  add_or_Delet_NewClient(clientfd, 1);

               deng_lu_user[zaixiang_count].fd = clientfd;
               memcpy(deng_lu_user[zaixiang_count].name, p_user->name, 128); // 字符赋值是cpy
               printf("当前在线人数%d\n", zaixiang_count + 1);
               // printf("%d\n",deng_lu_user[zaixiang_count].fd);
               zaixiang_count++;
          }
          write(clientfd, p_msg, 1024);
     }
     else if (p_msg->type == GAI_MI) //--------------------------------修改密码------------------------------------
     {
          struct gai_mi *p_gai_mi = (struct gai_mi *)p_msg->content;
          char sql[256];
          sprintf(sql, "select name pwd from user where name='%s' and pwd='%s'", p_gai_mi->yuan_name, p_gai_mi->yuan_pwd); //"select *"表所有列用*                                                                                                    // 表中列名 列名      表名       列名           列名
          struct queryResult *pQR = query_db(pDB, sql);
          if (pQR->row == 0)
          {
               p_msg->type = NO;
          }
          else
          {
               p_msg->type = OK;
               char sql1[256];
               sprintf(sql1, "UPDATE user SET pwd = '%s' WHERE name = '%s'", p_gai_mi->xin_pwd, p_gai_mi->yuan_name);
               struct queryResult *pQR = query_db(pDB, sql1);
          }
          write(clientfd, p_msg, 1024);
     }

     else if (p_msg->type == ALL_CHAT) //-----------------------------群聊------------------------------------------
     {
          // printf("%d\n",clientfd);
          char buf1[1024];
          for (int j = 0; j < zaixiang_count; j++)
          {
               if (deng_lu_user[j].fd == clientfd)
               {
                    sprintf(buf1, "用户%s发来的群聊消息:%s\n", deng_lu_user[j].name, p_msg->content);
               }
          }
          for (size_t i = 0; i < zaixiang_count; i++) // 群发消息
          {
               printf("用户%s得到了消息%d\n", deng_lu_user[i].name, deng_lu_user[i].fd);
               // 如果不是消息的发送者，就可以把消息转发给这个套接字
               if (deng_lu_user[i].fd != clientfd)
               {
                    // 直接把通信结构体转发出去，里面携带了客户端输入的聊天信息
                    write(deng_lu_user[i].fd, buf1, 1024);
               }
          }
     }
     else if (p_msg->type == dan_CHAT) // --------------------------私聊--------------------------------------------------
     {
          struct dan_chat_node *p_cn = (struct dan_chat_node *)p_msg->content;
          // 在登录数组中去找到名字为pCN->dest的用户，并向该用户转发pMsg结构体即可

          char sql[256];
          sprintf(sql, "select w_friend from %s_friend where w_friend ='%s'", p_cn->self_name, p_cn->dest_name); //"select *"表所有列用*                                                                                                    // 表中列名 列名      表名       列名           列名
          struct queryResult *pQR5 = query_db(pDB, sql);
          if (pQR5->row == 0)   
          {                     //-------------------------------------没加对方好友-----------------------
               char str11[20];  
               sprintf(str11, "%s不是你的好友,请先加好友\n", p_cn->dest_name);
               for (size_t i = 0; i < zaixiang_count; i++)
               {
                    if (strcmp(deng_lu_user[i].name, p_cn->self_name) == 0)
                    {
                         write(deng_lu_user[i].fd, str11, 1024);
                    }
               }
          }
          else
          {
               char buf1[1024];     
               sprintf(buf1, "用户%s发来的私聊消息:%s\n", p_cn->self_name, p_cn->chat_text);

               for (size_t i = 0; i < zaixiang_count; i++)   //-----------------对方在线且也进入私聊---------------------
               {
                    if (strcmp(deng_lu_user[i].name, p_cn->dest_name) == 0)
                    {
                         write(deng_lu_user[i].fd, buf1, 1024);
                         goto nn;
                    }
               }   
               //-----------------------------------对方不在线，发到对方聊天表中-----------------------------
               char sql[256];
               sprintf(sql, "INSERT INTO %s_chat VALUES('%s','%s','%s',datetime('now','+8 hours'),'%s')", p_cn->dest_name, p_cn->self_name, p_cn->dest_name, p_cn->chat_text,"no");
               char *pSqlInsert = sql;
               exec_sql(pDB, pSqlInsert);   
               
               char strr[256]; 
               sprintf(strr, "用户%s不在线,消息存入对方的聊天表中\n", p_cn->dest_name);
                for (size_t i = 0; i < zaixiang_count; i++)
               {
                    if (strcmp(deng_lu_user[i].name, p_cn->self_name) == 0)
                    {
                         write(deng_lu_user[i].fd, strr, 1024);
                    }
               }
          }
           nn:
           printf(" ");

     }
     else if (p_msg->type == GET_CHAT) //--------------------------查看消息-------------------------------------
     {
         struct cha_kan_msg *p = (struct cha_kan_msg *)p_msg->content;
          char sql[256];
          sprintf(sql, "select * from %s_chat where flag='%s'", p->deng_lu_u, p->flag);
          struct queryResult *pQR = query_db(pDB, sql);
          int recordNum = pQR->row;
          printf("%d\n",recordNum);
          printf("%s\n",p->deng_lu_u);
          memcpy(p_msg->content, &recordNum, sizeof(recordNum));
          write(clientfd, p_msg, 1024); // 把消息数量发给客户端

                struct re_msg p2;
              for (size_t i = 5; i < pQR->row * pQR->col + pQR->col; i = i + 5)
              {
                    strcpy(p2.send_name, pQR->ppResult[i]); // 二级指针也相当于二维数组，指针的解应用可以像数组那样用
                    strcpy(p2.receive_name, pQR->ppResult[i+1]);
                    strcpy(p2.chat_text, pQR->ppResult[i+2]);
                    strcpy(p2.datatime, pQR->ppResult[i+3]);
                    strcpy(p2.flag, pQR->ppResult[i+4]);
                    memcpy(p_msg->content, &p2, sizeof( struct re_msg));
                     write(clientfd, p_msg, 1024);
                     if ((i + 1) % 5== 0)
                    {
                         printf("\n");
                    }
                  // printf("%s %s %s %s %s\n",pQR->ppResult[i],pQR->ppResult[i+1],pQR->ppResult[i+2],pQR->ppResult[i+3],pQR->ppResult[i+4]);
              }
          
     }
     else if(p_msg->type == chu_li_msg)    //---------------------处理未读消息---------------------------------
     {
          printf("11111\n");
          struct chuli_msg  *p1 = (struct chuli_msg  *)p_msg->content;
          char sql[256];
          sprintf(sql, "select flag from %s_chat where flag ='%s'", p1->deng_lu_u, p1->old_flag); //"select *"表所有列用*                                                                                                    // 表中列名 列名      表名       列名           列名
          struct queryResult *pQRs = query_db(pDB, sql);
          if (pQRs->row == 0)
          {
               p_msg->type = NO;
          }
          else
          {
               char sqls[256];
               sprintf(sqls, "UPDATE %s_chat SET flag ='%s' WHERE receive ='%s' and flag='%s'", p1->deng_lu_u, p1->new_flag,p1->deng_lu_u,"no");
               exec_sql(pDB, sqls);
                p_msg->type = OK;
          }
           write(clientfd, p_msg, 1024);
     }
     else if (p_msg->type == shen_qing_friend) //------------------申请加好友------------------------------------
     {
          struct shen_qingfred *p_sqf = (struct shen_qingfred *)p_msg->content;
          char sql[256];
          sprintf(sql, "select name  from user where name='%s'", p_sqf->duifang_name); //"select *"表所有列用*                                                                                                      // 表中列名 列名      表名       列名           列名
          struct queryResult *pQR = query_db(pDB, sql);
          if (pQR->row == 0)
          {
               p_msg->type = NO;
          }
          else
          {
               p_msg->type = OK;
               char buf1[1024];
               sprintf(buf1, "INSERT INTO %s_sqf VALUES ('%s','%s')", p_sqf->duifang_name, p_sqf->sq_name, p_sqf->zhuang_tai);
               exec_sql(pDB, buf1);
          }
          write(clientfd, p_msg, 1024);
     }

     else if (p_msg->type == can_kan_frend) //----------------------查看加好友申请-------------------------
     {
          struct chao_kan_fred *p = (struct chao_kan_fred *)p_msg->content;
          char sql[256];
          sprintf(sql, "select * from %s_sqf where zhuan_tai='%s'", p->deng_lu_name, p->zhuang_tai);
          struct queryResult *pQR2 = query_db(pDB, sql);
          int recordNum = pQR2->row;
          printf("%d\n", recordNum);
          memcpy(p_msg->content, &recordNum, sizeof(recordNum));
          write(clientfd, p_msg, 1024);
          //     printf("%s\n",p->zhuang_tai);
          //      printf("%s\n", p_msg->content);
          // printf("%d\n",pQR2->row * pQR2->col + pQR2->col);
          struct shen_qingfred p1;
          for (int i = 2; i < pQR2->row * pQR2->col + pQR2->col; i = i + 2)
          {
               strcpy(p1.sq_name, pQR2->ppResult[i]);
               strcpy(p1.zhuang_tai, pQR2->ppResult[i + 1]);

               memcpy(p_msg->content, &p1, sizeof(struct shen_qingfred));
               write(clientfd, p_msg, 1024);
               printf("%s %s\n", pQR2->ppResult[i], pQR2->ppResult[i + 1]);
          }
     }
     else if (p_msg->type == chu_li_frend) //-----------------------处理好友申请-----------------------
     {

          struct accepet_friend *p1 = (struct accepet_friend *)p_msg->content;
          char sql[256];
          sprintf(sql, "select shen_qing_ren from %s_sqf where shen_qing_ren ='%s'", p1->c_deng_lu_ren, p1->tonyi_name); //"select *"表所有列用*                                                                                                    // 表中列名 列名      表名       列名           列名
          struct queryResult *pQR5 = query_db(pDB, sql);
          printf("1111\n");
          if (pQR5->row == 0)
          {
               p_msg->type = NO;
          }
          else
          {
               p_msg->type = OK;
               char sql1[256];
               sprintf(sql1, "UPDATE %s_sqf SET zhuan_tai = '%s' WHERE shen_qing_ren = '%s'", p1->c_deng_lu_ren, "yes", p1->tonyi_name);
               exec_sql(pDB, sql1);

               char sqlq[256];
               sprintf(sqlq, "INSERT INTO %s_friend VALUES('%s')", p1->c_deng_lu_ren, p1->tonyi_name);   //你同意加好友
               exec_sql(pDB, sqlq);

               char sqla[256];
               sprintf(sqla, "INSERT INTO %s_friend VALUES('%s')", p1->tonyi_name, p1->c_deng_lu_ren);  //你同意了，对方把你加进去
               exec_sql(pDB, sqla);


          }
          write(clientfd, p_msg, 1024);
     }

     else if(p_msg->type == count_friend_and_msg)   //--------------读未读消息数和申请好友数--------------------------
     {
          struct strat_count *p12 = (struct strat_count *)p_msg->content;
          char sql[256];
          sprintf(sql, "select * from %s_sqf where zhuan_tai='%s'", p12->deng_lu_u, "no");
          struct queryResult *pQR = query_db(pDB, sql);
          int n2 = pQR->row;               //好友申请数

           char sql2[256];
          sprintf(sql2, "select * from %s_chat where flag='%s'", p12->deng_lu_u, "no");
          struct queryResult *pQR22 = query_db(pDB, sql2);
          int n1= pQR22->row;           //未读消息数
     
          // printf("%d\n",pQR->row);
          // printf("%d\n",pQR22->row);
          struct re_num re_n;
          re_n.n1=n1;
          re_n.n2=n2;
          memcpy(p_msg->content, &re_n, sizeof( struct re_num) );
          write(clientfd, p_msg, 1024);
         // printf("11111\n");
     }

     else if(p_msg->type==tui_chu_deng_lu)  //-----------------------退出登入-----------------------------------------
     {
          for (size_t i = 0; i < zaixiang_count; i++)
          {
               // 判断被失效的套接字是不是客户端数组中的第i个元素，如果是的就从数组中删除它
               if (clientfd == deng_lu_user[i].fd)
               {
                    // 从要删除的位置的后一个开始，逐个往前移动
                    for (int j = i + 1; j < zaixiang_count; j++) // i为删除位置
                    {
                         printf("handleEvent发现有套接字失效,删除了失效套接字\n");
                         deng_lu_user[j - 1] = deng_lu_user[j];
                    }
                    zaixiang_count--;
                    printf("有用户退出服务器\n");   
               }
          }
     }


}

int main()
{
     pDB = openDatabase("user_Data.db"); // openDatabase()创建一个数据库 ，参数为要创建的库名,返回值为指向数据库的指针

     char *pSql = "create table user(name,pwd,tel)"; // create table 表名（第几列的名）用于创建数据库中的表名和个列的名 创建了3列第1列name 第二列pwd 第3列tel
     if (1 == exec_sql(pDB, pSql))                   // exec_sql(pDB, pSql) sqlite3_exec函数用来执行SQL语句,参数2字符指针，是要执行的操作语句
     {
          printf("创建数据表成功\n");               //c\创建表名后在，创建会把之前写入的覆盖掉
     }

     char *pSql = "select * from user order by tel";
     struct queryResult *pQR = query_db(pDB, pSql);
     printf("一共有%d条记录%d\n", pQR->row, pQR->col);
     for (size_t i = 0; i < pQR->row * pQR->col + pQR->col; i++)
     {
          printf("%s ", pQR->ppResult[i]);
          if ((i + 1) % 3 == 0)
          {
               printf("\n");
          }
     }




     int epObj = init_server("192.168.217.128", 9001);
      server_handl_eEvent(epObj, sever_printf, Delet_tui_chu_Client); // 参数2.3都为函数地址（函数名），如果函数参数是指针就要传地址
     return 0;
}

