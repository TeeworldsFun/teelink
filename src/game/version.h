/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VERSION_H
#define GAME_VERSION_H
#include "generated/nethash.cpp"
#define GAME_VERSION "0.6.¾"
#define GAME_NETVERSION "0.6 " GAME_NETVERSION_HASH
static const char GAME_RELEASE_VERSION[8] = {'0', '.', '6', '.', '2', 0};

#define DDRACE_VERSIONNR 16031 // Old DDRace Version only for >16 players support
/*
1 - 3.5
2 - 3.5.1
3 - 3.5.2
4 - 3.5.3
5 - 3.6
6 - 3.6.1
7 - 3.7
8 - 3.8
9 - 3.8.1
10 - 3.9
11 - 3.9.1
12 - 4.0 [Final]
13 - 4.0.1
14 - 4.0.2
15 - 4.0.3
*/
#define HCLIENT_VERSION_CODE 15
#define HCLIENT_VERSION "4.0.3"

#define TEELINK_VERSION_CODE "flyer"
#define TEELINK_VERSION "0.1.0"
#endif
