# this CMakeLists.txt file shall implement following compilations:

# gcc `pkg-config --cflags gtk+-3.0` -o enter_race_result enter_race_result.c `pkg-config --libs gtk+-3.0` sqlite3.c -lpthread -ldl

# gcc -o evaluate_race_result evaluate_race_result.c sqlite3.c -lpthread -ldl

# gcc -o export_evaluation export_evaluation.c sqlite3.c -lpthread -ldl

# create a build directory for building and go there and run:
# cmake <path>/Source
# make

cmake_minimum_required(VERSION 3.10)
project(NRW-Manufacturer-Cup-Manager)

#use pkg-config tool to detect gtk headers and link libs
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

#include gtk3 headers
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

#add other flags from gtk3
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

#sqlite
#set (LIB_DIR_LOCATION ${NAME}/lib)
#find_library(SQLite3 NAMES sqlite3 PATHS ${LIB_DIR_LOCATION})
#set(INCLUDE_DIRS ${NAME}/include) 

#pthreads
#set(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

#add executables
ADD_EXECUTABLE(enter_race_result enter_race_result.c sqlite3.c)
ADD_EXECUTABLE(evaluate_race_result evaluate_race_result.c sqlite3.c)
ADD_EXECUTABLE(export_evaluation export_evaluation.c sqlite3.c)

#link the target to the gtk+ libraries
TARGET_LINK_LIBRARIES(enter_race_result ${GTK3_LIBRARIES})
TARGET_LINK_LIBRARIES(evaluate_race_result ${GTK3_LIBRARIES})
TARGET_LINK_LIBRARIES(export_evaluation ${GTK3_LIBRARIES})

#link the target to pthreads and dl (-lpthread -ldl)
TARGET_LINK_LIBRARIES(enter_race_result Threads::Threads ${CMAKE_DL_LIBS})
TARGET_LINK_LIBRARIES(evaluate_race_result Threads::Threads ${CMAKE_DL_LIBS})
TARGET_LINK_LIBRARIES(export_evaluation Threads::Threads ${CMAKE_DL_LIBS})
