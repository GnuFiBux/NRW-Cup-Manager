# NRW-Cup-Manager
An simrace evaluation tool

Description
===========

This program is calculating a ranking list. It was designed an written to support the evaluation for the NRW Manufacturer Cup, which is a simracing competition.

Version History
===============

0.1 this version
At this point of time time this is a pure command line tool.
This is the first release of this tool after initial testing. This version has only been developed and tested for Linux.

Installation
============

In order to have this tool available, you need to download the sources.
This will create a folder "NRW Cup Manager" on your system.
The "NRW Cup Manager" folder will include
- a this Readme.txt file
- the Source directory

To build the executables, create a Build directory (anywhere you like), change to that directory and run
- cmake <path_to>/Source
- make

This will create three executable files
- enter_race_result
- evaluate_race_result
- export_evaluation

Alternatively, you can build directly in the Source directory, using following commands for linux:
- gcc -o enter_race_result enter_race_result.c sqlite3.c -lpthread -ldl
- gcc -o evaluate_race_result evaluate_race_result.c sqlite3.c -lpthread -ldl
- gcc -o export_evaluation export_evaluation.c sqlite3.c -lpthread -ldl

Note that using cmake is the preferred method.

Usage
=====

1. Entering the race result
----------------------------

To enter the race result run
<path_to>/enter_race_result <name of the input data file>

Example:
<path_to>/enter_race_result R4
This will read the from a file that must be name "R4.txt" and that has to be created before running this command.
The result will be written into an sqlite database file called "R4.db" that with a table called t_race_result.

The input data file (in this expample R4.txt) has to have following format:

driver-name	team	manufacturer	time-1	time-2	flag

Example:

driver-1	team-1	manufacturer-1	00:16:00.361	00:03:08.000	0
driver-2	team-2	manufacturer-2	00:16:01.356	00:03:07.158	0
//-------	----------	------------	------------	------------	JOKER
driver-3	team-3	manufacturer-3	00:19:00.000	09:00:00.000	4
driver-4	team-4	manufacturer-4	00:19:00.000	09:00:00.000	4
//------	----------	------------	------------	------------	NO-SHOW
driver-5	team-5	manufacturer-5	00:19:00.000	09:00:00.000	0
driver-6	team-6	manufacturer-6	00:19:00.000	09:00:00.000	0

Note that lines after "NO-SHOW" will be ignored.

View the result of this example by running
sqlite3 R4.db
.header on
.mode column
select * from t_race_result where race = 'R4' order by result_pos asc;

Quit the sqlite3 sesssion with ctrl-D.

2. Evaluating the race result
-----------------------------

To evaluate the race result run
<path_to>/evaluate_race_result <sqlite database file>

Example:
<path_to>/evaluate_race_result R4
This will read the from the "R4.db" database file that was created or updated with the command enter_race_result.
This will popolate the columns RESULT_POS, WIN_POINTS, TEAM_POINTS, and RACE_POINTS, and will create/update the tables called t_team_result and t_cup_result.

sqlite3 R4.db
.header on
.mode column
select * from t_team_result where race = 'R4';
select * from t_cup_result order by tot_cup_pts desc;

Quit the sqlite3 sesssion with ctrl-D.

2. Export the evaluation
------------------------

Currently the export only supports the format required to publish the result on the GTRP community platform (www.gtrp.de).

To export the race result and evaluation run
<path_to>/export_evaluation <database filename without extension> "<title text>" > <target file name>

Example:
<path_to>/export_evaluation R4 "Hauptsaison 2019 Runde 2 - R4" > R4.gtrp.txt
This will create a text file called R4.gtrp.txt .
If no target file is given then the result will be printed on the standard output (i.e. printed on the screen).
