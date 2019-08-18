#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"


/* compile with gcc -o export_evaluation export_evaluation.c sqlite3.c -lpthread -ldl */
/* run ./export_evaluation test.db "Hauptsaison 2019 Runde 1 - R3"*/


/* --- CLASS DB Manager -----------------------------------------------------*/

sqlite3 *db;
char *zErrMsg = 0;
int rc;
int position = 1;
int count_result=0;

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

static int gtrp_race_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  int events;

  printf("[tr]\n");

  printf("[td]%d[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td]",
   position++,
   col[DRIVER],
   col[TEAM],
   col[MANUFACTURER],
   col[TOTAL_TIME],
   col[WIN_POINTS],
   col[TEAM_POINTS],
   col[RACE_POINTS]);
  
  events = atoi(col[EVENTS]);
  switch (events)
  {
    case 1 : printf("[td]Markenwechsel[/td]");
             break;
    case 2 : printf("[td]Falscher Boxenstop[/td]");
             break;
    case 3 : printf("[td]Markenwechsel + Falscher Boxenstop[/td]");
             break;
    case 4 : printf("[td]Joker[/td]");
             break;
    default: break;
  }

  printf("\n[/tr]\n");
  return 0;
}

int db_query_gtrp_race_result(char *cmd)
{
  printf("[table=\"width: 500, class: grid, align: left\"]\n[tr]\n\
	  [td]Pos[/td]\n\
	  [td]Fahrer[/td]\n\
	  [td]Team[/td]\n\
	  [td]Hersteller[/td]\n\
	  [td]Zeit[/td]\n\
	  [td]Siegpkt[/td]\n\
	  [td]Teampkt[/td]\n\
	  [td]Rennpkt[/td]\n[/tr]\n");

  rc = sqlite3_exec(db, cmd, gtrp_race_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  printf("[/table]\n");
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int gtrp_team_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  int events;

  printf("[tr]\n");

  printf("[td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td]",
   col[0],
   col[2],
   col[3],
   col[4],
   col[5]);

  printf("\n[/tr]\n");
  return 0;
}

int db_query_gtrp_team_result(char *cmd)
{
  printf("[table=\"width: 500, class: grid, align: left\"]\n[tr]\n\
	  [td]Team[/td]\n\
	  [td]Siegpkt[/td]\n\
	  [td]Teampkt[/td]\n\
	  [td]Rennpkt[/td]\n\
	  [td]Ranglistenpkt[/td]\n[/tr]\n");

  rc = sqlite3_exec(db, cmd, gtrp_team_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  printf("[/table]\n");
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int gtrp_best_lap_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  int events;

  printf("[tr]\n");

  printf("[td]%d[/td][td]%s[/td][td]%s[/td][td]%s[/td][td]%s[/td]",
   position++,
   col[DRIVER],
   col[TEAM],
   col[MANUFACTURER],
   col[BEST_LAP_TIME]);

  printf("\n[/tr]\n");
  return 0;
}

int db_query_gtrp_best_lap_result(char *cmd)
{
  printf("[table=\"width: 500, class: grid, align: left\"]\n[tr]\n\
	  [td]Pos[/td]\n\
	  [td]Fahrer[/td]\n\
	  [td]Team[/td]\n\
	  [td]Hersteller[/td]\n\
	  [td]Beste Runde[/td]\n[/tr]\n");

  rc = sqlite3_exec(db, cmd, gtrp_best_lap_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  printf("[/table]\n");
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int gtrp_team_driver(void *NotUsed, int num_col, char **col, char **azColName)
{
  if (col[0] != NULL)
    printf("%s ",col[0]);

  return 0;
}

int db_query_team_driver(char *cmd)
{
  rc = sqlite3_exec(db, cmd, gtrp_team_driver, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return(rc);
}

/* --------------------------------------------------------------------------*/

static int gtrp_cup_result(void *NotUsed, int num_col, char **col, char **azColName)
{
  int i;
  char cmd[100];

  if (num_col != count_result+2)
   printf("=== WARNING: number of columns does not match number of races! ===\n");

  printf("[tr]\n");
  for (i=0; i<num_col; i++)
  {
    if (col[i] != NULL)
      printf("[td]%s[/td]",col[i]);
    else
      printf("[td][/td]");
    if (i==0)
    {
      printf("\n[td]");
      sprintf(cmd,"select distinct driver from t_race_result where team = '%s'",col[0]);
      db_query_team_driver(cmd);
      printf("[/td]\n");
    }
  }
  printf("\n[/tr]\n");
  return 0;
}

int db_query_gtrp_cup_result(char *cmd)
{
  printf("[table=\"width: 500, class: grid, align: left\"]\n\
[tr]\n\
    [td][/td]\n\
    [td][/td]\n\
    [td]Ranglistenpunkte[/td]\n\
[/tr]\n\
[tr]\n\
    [td]Hersteller[/td]\n\
    [td]Fahrer[/td]\n\
    [td]Total[/td]\n\
    [td][URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923381&viewfull=1#post1923381\"]R1[/URL][/td]\n\
    [td][URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923550&viewfull=1#post1923550\"]R2[/URL][/td]\n\
    [td][URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923642&viewfull=1#post1923642\"]R3[/URL][/td]\n\
    [td][URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923666&viewfull=1#post1923666\"]R4[/URL][/td]\n\
    [td][URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923849&viewfull=1#post1923849\"]R5[/URL][/td]\n\
[/tr]\n\n");

  rc = sqlite3_exec(db, cmd, gtrp_cup_result, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  printf("[/table]\n");

  printf("\n\
(J) = Joker\n\
(B) = Quereinsteiger-Bonus\n\
\n\
Links zu den Ergebnissen von vorherigen Durchgängen:\n\n\
[URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1924129&viewfull=1#post1924129\"]Hauptsaison 2019 Runde 1 - Abschlussrangliste[/URL] (nach unten scrollen)\n\
[URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1923203&viewfull=1#post1923203\"]2019 Mai Grp3[/URL] (nach unten scrollen)\n\
[URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1922507&viewfull=1#post1922507\"]Test Saison April 2019[/URL]\n\n");

  return(rc);
}

/* --------------------------------------------------------------------------*/

void export_gtrp(char* title, char *race)
{
  char query_cmd[200];

  printf("Ergebnis und Wertung - %s:\n\n",title);
  printf("Ergebnis und Zwischenwertung:\n\n");
  position = 1;
  sprintf(query_cmd,"select * from t_race_result where race = '%s' order by RESULT_POS",race);
  db_query_gtrp_race_result(query_cmd);

  printf("\nWertung:\n\n");
  sprintf(query_cmd,"select * from t_team_result where race = '%s'",race);
  db_query_gtrp_team_result(query_cmd);
  printf("\n[URL=\"http://www.gtrp.de/showthread.php?42257-NRW-Manufacturer-Cup&p=1921923&viewfull=1#post1921923\"]Aktuelle Rangliste[/URL]\n");

  printf("\n[B]Startreihenfolge im nächsten Rennen gemäß bester Runde:[/B]\n\n");
  position = 1;
  sprintf(query_cmd,"select * from t_race_result where race = '%s' order by BEST_LAP_TIME asc",race);
  db_query_gtrp_best_lap_result(query_cmd);

  printf("\n[B]Aktuelle Rangliste[/B]\n\n");
  sprintf(query_cmd,"select count(distinct race) from t_team_result");
  db_query_count(query_cmd);
  sprintf(query_cmd,"select * from t_cup_result order by TOT_CUP_PTS desc");
  db_query_gtrp_cup_result(query_cmd);
}

/* --------------------------------------------------------------------------*/

int
main (int   argc,
      char *argv[])
{
  char db_name[25];

  sprintf(db_name,"%s.db",argv[1]);
  db_man_init(db_name);

  /* ------------------------------------------------------------------------*/
  /* export to GTRP format                                                   */
  /* ------------------------------------------------------------------------*/

  export_gtrp(argv[2],argv[1]);

  return 0;
}
