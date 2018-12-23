/*
	Разработчик: Попов Павел
	Дата: 23.12.18
	Описание: многопоточный http web server работающий на скриптах LUA.
*/

#ifndef _MAIN_H_
#define _MAIN_H_

// LUA функции
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

// необходимые функции для работы с ними
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

// используем пространство имён std
using namespace std;

// максимальное количество потоков
#define USING_THREADS 512
// размер буфера для приёма данных (порция 128 байт)
#define SIZEOF_RECV_BUFFER 128

// структура данных для передачи в параметров в новый поток
struct thread_data{
	char addr[36]; // IP адрес
	SOCKET client; // номер клиента
	int thread_id; // ID созданного потока
}; 

// LUA state
lua_State *L;
// структура данных сокета
WSADATA wsd;
// сокет сервер
SOCKET server;
// локальный адрес сервера
sockaddr_in localaddr;
// дескриптор основоного потока каторый занимается приёмом новых клиентов и создания потоков для клиентов для обработки информации
_Thrd_t run_thread;
// дескриптор потока клиента который занимается обработкой информации клиента
_Thrd_t client_threads[USING_THREADS];
// массив парпметров для потоков
thread_data pthread_data[USING_THREADS];
// массив определителей для потока каторый определяет загруженность того или инного потока
bool client_threads_inuse[USING_THREADS];
// определитель для основоного потока каторый определяет загруженность потока
bool run_thread_inuse;
// количество созданных потоков ( если > USING_THREADS то на 0 )
int created_threads;
// путь в папке с временными файлами
char TempPath[260];


// создаёт LUA
void CreateLUA(void);
// перезапуск LUA
void RefreshLUA(void);
// удаление LUA
void DestroyLUA(void);
// функция LUA для отправки файла клиенту с параметром Content-type: X/Y
int Lua_SendData(lua_State *st);
// функция LUA для отправки клиенту HTML страницы
int Lua_SendHTMLPage(lua_State *st);
// функция LUA для отправки клиенту HTML кода
int Lua_SendHTMLCode(lua_State *st);
// функция LUA для отправки клиенту картинки
int Lua_SendImage(lua_State *st);
// функция LUA для отправки клиенту файла
int Lua_SendFile(lua_State *st);
// функция LUA которая закрывает соединение с клиентом в случае если соединение Keep-Alive
int Lua_CloseConnection(lua_State *st);
// открывает сервер
void CreateWebServer(unsigned short port);
// закрывает сервер
void DestroyWebServer(void);
// функция для отправки файла клиенту с параметром Content-type: X/Y
void SendData(SOCKET client,const char *path, char *content_type);
// функция для отправки клиенту HTML страницы
void SendHTMLPage(SOCKET client,const char *path,bool keep_alive);
// функция для отправки клиенту HTML кода
void SendHTMLCode(SOCKET client,char *code,bool keep_alive);
// функция для отправки клиенту картинки
void SendImage(SOCKET client,const char *path);
// функция для отправки клиенту файла
void SendFile(SOCKET client,const char *path,char *filename);
// убирает всё лишнее из строки где присутствует GET /X HTTP ... и оставляет только X
string ParseRequest(string request,char *what);
// декодирует url строку (HEX) в символы
string UrlDecode(string &SRC);
// возвращает число из строки Content-Length: X
int Parse_GetContentLength(string str);
// основоной поток каторый занимается приёмом новых клиентов и создания потоков для клиентов для обработки информации
int NetThread(void *lpParam);
// поток который занимается обработкой информации клиента
int ClientThread(void *lpParam);

#endif