/*
	�����������: ����� �����
	����: 23.12.18
	��������: ������������� http web server ���������� �� �������� LUA.
*/

#ifndef _MAIN_H_
#define _MAIN_H_

// LUA �������
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

// ����������� ������� ��� ������ � ����
#include <sdkddkver.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <locale>
#include <time.h>
#include <winsock.h>
#include <thr\xthread>
#include "resource.h"

// ���������� ������������ ��� std
using namespace std;

// ������������ ���������� �������
#define USING_THREADS 512
// ������ ������ ��� ����� ������ (������ 128 ����)
#define SIZEOF_RECV_BUFFER 128

// ��������� ������ ��� �������� � ���������� � ����� �����
struct thread_data{
	char addr[36]; // IP �����
	SOCKET client; // ����� �������
	int thread_id; // ID ���������� ������
}; 

// LUA state
lua_State *L;
// ��������� ������ ������
WSADATA wsd;
// ����� ������
SOCKET server;
// ��������� ����� �������
sockaddr_in localaddr;
// ���������� ���������� ������ ������� ���������� ������ ����� �������� � �������� ������� ��� �������� ��� ��������� ����������
_Thrd_t run_thread;
// ���������� ������ ������� ������� ���������� ���������� ���������� �������
_Thrd_t client_threads[USING_THREADS];
// ������ ���������� ��� �������
thread_data pthread_data[USING_THREADS];
// ������ ������������� ��� ������ ������� ���������� ������������� ���� ��� ������ ������
bool client_threads_inuse[USING_THREADS];
// ������������ ��� ���������� ������ ������� ���������� ������������� ������
bool run_thread_inuse;
// ���������� ��������� ������� ( ���� > USING_THREADS �� �� 0 )
int created_threads;
// ���� � ����� � ���������� �������
char TempPath[260];


// ������ LUA
void CreateLUA(void);
// ���������� LUA
void RefreshLUA(void);
// �������� LUA
void DestroyLUA(void);
// ������� LUA ��� �������� ����� ������� � ���������� Content-type: X/Y
int Lua_SendData(lua_State *st);
// ������� LUA ��� �������� ������� HTML ��������
int Lua_SendHTMLPage(lua_State *st);
// ������� LUA ��� �������� ������� HTML ����
int Lua_SendHTMLCode(lua_State *st);
// ������� LUA ��� �������� ������� ��������
int Lua_SendImage(lua_State *st);
// ������� LUA ��� �������� ������� �����
int Lua_SendFile(lua_State *st);
// ������� LUA ������� ��������� ���������� � �������� � ������ ���� ���������� Keep-Alive
int Lua_CloseConnection(lua_State *st);
// ��������� ������
void CreateWebServer(unsigned short port);
// ��������� ������
void DestroyWebServer(void);
// ������� ��� �������� ����� ������� � ���������� Content-type: X/Y
void SendData(SOCKET client,const char *path, char *content_type);
// ������� ��� �������� ������� HTML ��������
void SendHTMLPage(SOCKET client,const char *path,bool keep_alive);
// ������� ��� �������� ������� HTML ����
void SendHTMLCode(SOCKET client,char *code,bool keep_alive);
// ������� ��� �������� ������� ��������
void SendImage(SOCKET client,const char *path);
// ������� ��� �������� ������� �����
void SendFile(SOCKET client,const char *path,char *filename);
// ������� �� ������ �� ������ ��� ������������ GET /X HTTP ... � ��������� ������ X
string ParseRequest(string request,char *what);
// ���������� url ������ (HEX) � �������
string UrlDecode(string &SRC);
// ���������� ����� �� ������ Content-Length: X
int Parse_GetContentLength(string str);
// ��������� ����� ������� ���������� ������ ����� �������� � �������� ������� ��� �������� ��� ��������� ����������
int NetThread(void *lpParam);
// ����� ������� ���������� ���������� ���������� �������
int ClientThread(void *lpParam);

#endif