#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"


/* compile with gcc -o evaluate_race_result evaluate_race_result.c sqlite3.c -lpthread -ldl */
/* run ./evaluate_race_result R1*/


/* --- CLASS DB Manager -----------------------------------------------------*/

sqlite3 *db;
char *zErrMsg = 0;
int rc;
int count_result;
int max_race_points;

enum
{
   DRIVER,
   TEAM,
   MANUFACTURER,
   RACE,
   TOTAL_TIME,
   BEST_LAP_TIME,
   EVENTS,
   RESULT_POS,
   WIN_POINTS,
   TEAM_POINTS,
   RACE_POINTS,
   N_COLUMNS
};

/* --------------------------------------------------------------------------*/

int db_man_init(char* db_name)
{
  rc = sqlite3_open(db_name, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
  return(0);
}

/* --------------------------------------------------------------------------*/

int db_put(char *cmd)
{
  rc = sqlite3_exec(db, cmd, NULL, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int get_count(void *NotUsed, int argc, char **argv, char **azColName){
  count_result = atoi(argv[0]);
  return 0;
}

int db_query_count(char *cmd)
{
  rc = sqlite3_exec(db, cmd, get_count, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_win_points(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];
  int event;

  event = atoi(col[EVENTS]);

  if (event < 4)
  {
    sprintf(cmd,"select count(\"DRIVER\") from (select * from t_race_result where race = '%s' and result_pos > '%s' and team != '%s')\n",col[RACE],col[RESULT_POS],col[TEAM]);
    db_query_count(cmd);

    sprintf(cmd,"update t_race_result set win_points = %d WHERE race = '%s' and result_pos = '%s'\n",count_result,col[RACE],col[RESULT_POS]);
    db_put(cmd);
  }
  else
  {
    sprintf(cmd,"update t_race_result set win_points = %d WHERE race = '%s' and result_pos = '%s'\n",0,col[RACE],col[RESULT_POS]);
    db_put(cmd);
  }

  return 0;
}

int db_query_win_points(char *cmd)
{
  rc = sqlite3_exec(db, cmd, process_win_points, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_team_points(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];

  sprintf(cmd,"select count(\"DRIVER\") from (select * from t_race_result where race = '%s' and team = '%s' and driver != '%s')\n",col[RACE],col[TEAM],col[DRIVER]);
  db_query_count(cmd);

/* for the time being no team points!  
  if (count_result > 0)
    sprintf(cmd,"update t_race_result set team_points = %d WHERE race = '%s' and result_pos = '%s'\n",1,col[RACE],col[RESULT_POS]);
  else */
    sprintf(cmd,"update t_race_result set team_points = %d WHERE race = '%s' and result_pos = '%s'\n",0,col[RACE],col[RESULT_POS]);

  db_put(cmd);

  return 0;
}

int db_query_team_points(char *cmd)
{
  rc = sqlite3_exec(db, cmd, process_team_points, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_race_points(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];

  count_result = atoi(col[WIN_POINTS]) + atoi(col[TEAM_POINTS]);

  sprintf(cmd,"update t_race_result set race_points = %d WHERE race = '%s' and result_pos = '%s'\n",count_result,col[RACE],col[RESULT_POS]);

  db_put(cmd);
  
  return 0;
}

int db_query_race_points(char *cmd)
{
  rc = sqlite3_exec(db, cmd, process_race_points, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_team_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];

  sprintf(cmd,"insert into t_team_result(TEAM,RACE,WIN_POINTS,TEAM_POINTS,RACE_POINTS) values ('%s','%s',%d,%d,%d)\n",col[0],col[1],atoi(col[2]),atoi(col[3]),atoi(col[4]));

  db_put(cmd);
  
  return 0;
}

int db_query_team_result(char *cmd)
{
  rc = sqlite3_exec(db, cmd, process_team_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_cup_points(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];
  int r_points;
  int c_points;

  r_points = atoi(col[4]);
  c_points = count_result;

  if (r_points == max_race_points)
    c_points = count_result;
  else
  {
    max_race_points = r_points;
    count_result--;
    c_points = count_result;
  }

  sprintf(cmd,"update t_team_result set cup_points = %d WHERE team = '%s' and race = '%s'\n",c_points,col[0],col[1]);

  db_put(cmd);
  
  return 0;
}

int db_query_cup_points(char *cmd, char *race)
{
  char query[200];
  /* get the maximum number of race points achieved in this race */
  sprintf(query,"select max(RACE_POINTS) from t_team_result where race = '%s'\n",race);
  db_query_count(query);
  max_race_points = count_result;
  /* set the count result to the number of max cup points */
  count_result = 25;

  /* now walk throught teams */
  rc = sqlite3_exec(db, cmd, process_cup_points, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_result_pos(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];

  sprintf(cmd,"update t_race_result set result_pos = %d where total_time = '%s'\n",
          ++count_result,col[TOTAL_TIME]);

  db_put(cmd);
  
  return 0;
}

int db_query_result_pos(char *cmd)
{
  /* now walk throught teams */
  rc = sqlite3_exec(db, cmd, process_result_pos, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_cup_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[200];

  /* check if the team is already inserted into the cup result */
  count_result = 0;
  sprintf(cmd,"select count(TEAM) from t_cup_result where TEAM = '%s'\n",col[0]);
  db_query_count(cmd);

  if (!count_result)
  {
    sprintf(cmd,"insert into t_cup_result(TEAM,%s) values ('%s','%s')\n",col[1],col[0],col[2]);
    db_put(cmd);
  }
  else
  {
    sprintf(cmd,"update t_cup_result set %s = %s WHERE team = '%s'",col[1],col[2],col[0]);
    db_put(cmd);
  }

  return 0;
}

int db_query_cup_result(char *cmd)
{
  /* now walk throught teams */
  rc = sqlite3_exec(db, cmd, process_cup_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int process_cup_totals(void *NotUsed, int num_col, char **col, char **azColName)
{
  char cmd[400];
  int old_cup_total=0;
  int new_cup_result=0;

  if (col[1] != NULL)
    old_cup_total = atoi(col[1]);
  if (col[2] != NULL)
    new_cup_result = atoi(col[2]);

  sprintf(cmd,"update t_cup_result set TOT_CUP_PTS = %d WHERE team = '%s'", old_cup_total + new_cup_result, col[0]);
  db_put(cmd);

  return 0;
}

int db_query_cup_totals(char *cmd)
{
  /* now walk throught teams */
  rc = sqlite3_exec(db, cmd, process_cup_totals, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

int
main (int   argc,
      char *argv[])
{
  char db_name[25];
  char query_cmd[400];

  sprintf(db_name,"%s.db",argv[1]);
  if (db_man_init(db_name))
    return(1);

  /* ------------------------------------------------------------------------*/
  /* confirm the position by evaluating the events                           */
  /* ------------------------------------------------------------------------*/
  
  count_result = 0;
  
  sprintf(query_cmd,"select * from t_race_result where race = '%s' and EVENTS = 0 order by TOTAL_TIME asc",argv[1]);
  db_query_result_pos(query_cmd);

  sprintf(query_cmd,"select * from t_race_result where race = '%s' and EVENTS > 0 and EVENTS < 4 order by TOTAL_TIME asc",argv[1]);
  db_query_result_pos(query_cmd);

  /*sprintf(query_cmd,"select * from t_race_result where race = '%s' and EVENTS = 3 order by TOTAL_TIME asc",argv[1]);
  db_query_result_pos(query_cmd);*/

  sprintf(query_cmd,"select * from t_race_result where race = '%s' and EVENTS = 4 order by TOTAL_TIME asc",argv[1]);
  db_query_result_pos(query_cmd);

  sprintf(query_cmd,"select * from t_race_result where race = '%s' and EVENTS > 4 order by TOTAL_TIME asc",argv[1]);
  db_query_result_pos(query_cmd);

  /* ------------------------------------------------------------------------*/
  /* add a column for the win points,                                       */
  /* sort the result by total time and walkthrough the list one by one       */
  /* ------------------------------------------------------------------------*/

  sprintf(query_cmd,"select * from t_race_result where race = '%s' order by result_pos asc",argv[1]);
  db_query_win_points(query_cmd);

  /* ------------------------------------------------------------------------*/
  /* add a column for the team points,                                       */
  /* walkthrough the list driver by driver and collect the team points       */
  /* ------------------------------------------------------------------------*/

  sprintf(query_cmd,"select * from t_race_result where race = '%s'",argv[1]);
  db_query_team_points(query_cmd);

  /* ------------------------------------------------------------------------*/
  /* add a column for the race points,                                       */
  /* walkthrough the list driver by driver and create the race points        */
  /* ------------------------------------------------------------------------*/

  sprintf(query_cmd,"select * from t_race_result where race = '%s'",argv[1]);
  db_query_race_points(query_cmd);

  /* ------------------------------------------------------------------------*/
  /* add a table for the team result,                                        */
  /* query the team result and store it into the new table                   */
  /* ------------------------------------------------------------------------*/

  db_put("create table t_team_result (TEAM varchar(30), RACE varchar(30), WIN_POINTS smallint, TEAM_POINTS smallint, RACE_POINTS smallint, CUP_POINTS smallint)");
  sprintf(query_cmd,"select TEAM, RACE, SUM(WIN_POINTS),SUM(TEAM_POINTS),SUM(RACE_POINTS) from t_race_result where race = '%s' group by TEAM order by SUM(RACE_POINTS) desc",argv[1]);
  db_query_team_result(query_cmd);

  /* ------------------------------------------------------------------------*/
  /* add a column for the cup points to the team result,                     */
  /* walkthrough the list team by team and create the cup points             */
  /* ------------------------------------------------------------------------*/

  sprintf(query_cmd,"select * from t_team_result where race = '%s'", argv[1]);
  db_query_cup_points(query_cmd,argv[1]);

  /* ------------------------------------------------------------------------*/
  /* add a table for the cup result,                                        */
  /* query the cup result and store it into the new table                   */
  /* ------------------------------------------------------------------------*/

  db_put("create table t_cup_result (TEAM varchar(30), TOT_CUP_PTS smallint)");

  sprintf(query_cmd,"alter table t_cup_result add \"%s\"", argv[1]);
  db_put(query_cmd);

  sprintf(query_cmd,"select TEAM, RACE, CUP_POINTS from t_team_result where race = '%s' group by TEAM order by CUP_POINTS desc", argv[1]);
  db_query_cup_result(query_cmd);

  sprintf(query_cmd,"select TEAM, TOT_CUP_PTS, %s from t_cup_result",argv[1]);
  db_query_cup_totals(query_cmd);

  return 0;
}
