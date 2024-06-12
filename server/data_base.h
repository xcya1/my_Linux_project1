#ifndef __DATA_BASE_H
#define __DATA_BASE_H

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include "comment.h"


sqlite3 * openDatabase(char * pDatabaseName);
int exec_sql(sqlite3 * pDB , char * pSql);
struct queryResult * query_db(sqlite3 * pDB , char * pSql);



#endif