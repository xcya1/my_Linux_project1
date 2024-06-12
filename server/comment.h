#ifndef _COMMENT_H
#define _COMMENT_H
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <pthread.h>
// #include <malloc.h>
 #include <string.h>
// #include <sys/select.h>
// #include <sys/time.h>
// #include <sys/epoll.h>

#define MSG_BUF_SIZE 1024

#define delay 1

//消息的意图/类型
#define REGISTER 1 //注册
#define LOGIN 2    //登录
#define GAI_MI 3   //改密
#define ALL_CHAT 4   //群聊
#define dan_CHAT 5   //私聊
#define GET_CHAT 6   //查看消息
#define chu_li_msg 61  //处理未读消息

// #define weidu_xiaoxi 99 //查看未读消息

#define shen_qing_friend 7     //申请加好友
#define can_kan_frend 8       //查看申请好友
#define chu_li_frend 81      //处理好友申请

#define tui_chu_deng_lu 9  //退出登入

#define count_friend_and_msg  20  //记未读消息数和申请好友数（气泡显示）
#define count_msg 21



#define OK 100
#define NO 101
#define cunzai 102 //用户存在

//------------------------------

struct shen_qingfred 
{
     char duifang_name[128];
     char sq_name[128];
     char zhuang_tai[20];
     
};

struct chao_kan_fred
{
     char deng_lu_name[128];
     char zhuang_tai[12];

};

struct accepet_friend
{
     char c_deng_lu_ren[128];
     char tonyi_name[128];

};




//-----------------------------------


 struct netNode     //消息
{
     char type;//消息的类型
     char content[MSG_BUF_SIZE-1];//消息的内容
};

struct gai_mi
{
     //char type;
     char yuan_name[128];
     char yuan_pwd[128];
     char xin_pwd[128];
};

struct userInfo    //注册信息
{
     char name[128];//用户的账号名
     char pwd[128];//密码
     char tel[12];//手机号
};

 struct queryResult    //查询数据库中某个表的结果，查询结果会返回3个数，可定义个结构体来接数据
{
     char ** ppResult;
     int row;
     int col;
};



struct zai_xiang_yonhu   //在线用户
{
     int fd;
     char name[128];
};

 struct dan_chat_node  //私聊
{
     char self_name[128];//自己的名字
     char dest_name[128];//对方的名字
     char chat_text[700];//聊天的内容
};

struct cha_kan_msg
{
     char deng_lu_u[128];
     char flag[12];
};


 struct re_msg  //查看消息
{
     //struct dan_chat_node cn;
     
     char send_name[128];
     char receive_name[128];
     char chat_text[700];//聊天的内容
     char datatime[32];
     char flag[12];
};

struct chuli_msg  //处理未读消息
{
     char deng_lu_u[128];
     char old_flag[12];
     char new_flag[12];    
};


struct strat_count
{
     char deng_lu_u[128];
};

struct re_num
{
     int n1;
     int n2;
     
};



#endif