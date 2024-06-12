#include "data_base.h"



//数据库编译要加库-lsqlite3

// 打开数据库，数据库不存在就会自动创建
sqlite3 * openDatabase(char * pDatabaseName)
{
     sqlite3 *db = NULL;
     int data = -1;

     // 打开指定的数据库文件，如果不存在将创建一个同名的数据库文件
     data = sqlite3_open(pDatabaseName, &db);    //返回值小于0的创建失败
     if (data < 0)
     {
          printf("create testsqlite failure:%s\n", sqlite3_errmsg(db)); // 创建失败
          return NULL;
     }
     else
     {
          printf("create testsqlite successfuly\n"); // 创建成功
          //    sqlite3_close(db);//关闭数据库
          return db;
     }
}


//在pDB指向的数据库中创建表格
int exec_sql(sqlite3 * pDB , char * pSql)
{
     char * pErrMsg=NULL;
     //参数1：数据库的指针
     //参数2：要执行的SQL语句
     //参数3：一个函数的地址，当执行完参数2指定的SQL语句后，sqlite3_exec会调用该函数，如果有需要就写上，不需要就写成NULL
     //参数4：传给参数3指定的函数的参数
     //参数5：如果执行过程中出错，就把错误信息传个该参数
     //执行上面的SQL语句
     int ret=sqlite3_exec(pDB,pSql,NULL,NULL,&pErrMsg);
     if(ret != SQLITE_OK)
     {
          //如果错误内容是提示表格已经存在，那么我们这里不算错误，直接返回1
          if(strcmp(pErrMsg,"table user already exists")==0)
          {
               return 1;
          }
          printf("执行SQL语句失败:%s\n",pErrMsg);
          return 0;
     }
     return 1;
}

struct queryResult * query_db(sqlite3 * pDB , char * pSql)
{
     struct queryResult * pQR = malloc(sizeof( struct queryResult));
     char * pErrMsg;
     // sqlite3_get_table用来获取数据表中的内容
     // 参数1：数据库的指针
     // 参数2：SQL的查询语句
     // 参数3：查询的结果
     // 参数4：结果的行数
     // 参数5：结果的列数
     // 参数6：查询出错时的错误信息
     int ret = sqlite3_get_table(pDB,pSql,&pQR->ppResult,&pQR->row,&pQR->col,&pErrMsg);
     if(ret != SQLITE_OK)
     {
          printf("执行查询语句出错：%s",pErrMsg);
          return NULL;
     }
     //返回查询后的结果结构体
     return pQR;
}