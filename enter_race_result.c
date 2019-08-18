#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

/* compile with gcc -o enter_race_result enter_race_result.c sqlite3.c -lpthread -ldl */
/* run ./enter_race_result R5 */

sqlite3 *db;
char *zErrMsg = 0;

void run_sql_cmd (char *sql_statement)
{
  int rc;
  rc = sqlite3_exec(db, sql_statement, NULL, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}

int main(int argc, char **argv)
{
  char db_name[25];
  char file_name[25];
  char sql_cmd[400];
  FILE *file;
  char name[25];
  char team[25];
  char car[25];
  char tot_time[25];
  char best_lap[25];
  char event[25];
  int rc;

  sprintf(db_name,"%s.db",argv[1]);
  rc = sqlite3_open(db_name, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
  
  sprintf(sql_cmd,"create table t_race_result (DRIVER varchar(30),TEAM varchar(30),MANUFACTURER varchar(30),RACE varchar(10),TOTAL_TIME varchar(15),BEST_LAP_TIME varchar(15),EVENTS smallint,RESULT_POS smallint,WIN_POINTS smallint,TEAM_POINTS smallint,RACE_POINTS smallint)");
  run_sql_cmd(sql_cmd);

  sprintf(file_name,"%s.txt",argv[1]);
  file = fopen(file_name, "r");
  while (!feof(file)) {
    fscanf(file, "%s %s %s %s %s %s\n",name, team, car, tot_time, best_lap, event);
    if (!strncmp(name,"//",2))
    {
      if (!strcmp(event,"NO-SHOW"))
        break;
    }
    else
    {
      sprintf(sql_cmd,"INSERT INTO t_race_result(DRIVER,TEAM,MANUFACTURER,RACE,TOTAL_TIME,BEST_LAP_TIME,EVENTS) VALUES('%s','%s','%s','%s','%s','%s',%s)", name, team, car, argv[1], tot_time, best_lap, event);
      run_sql_cmd(sql_cmd);
    }
  }
  fclose(file);

  sqlite3_close(db);
  return 0;
}
