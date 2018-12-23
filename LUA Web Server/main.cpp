//------------------------------------------------------------------------------------------------------------

#include "main.h"

//------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv){
	setlocale(LC_ALL,"RUSSIAN");

	if(!argv[1]){
		printf("Application::Ошибка: введите параметр: \"путь к скрипту\"...\n");
		system("PAUSE");
		return 0;
	}

	if(!argv[2]){
		printf("Application::Ошибка: введите параметр: \"путь к папке с временными файлами\"...\n");
		system("PAUSE");
		return 0;
	}

	if(!argv[3]){
		printf("Application::Ошибка введите параметр: \"порт сервера\"...\n");
		system("PAUSE");
		return 0;
	}

	CreateLUA();
	CreateWebServer(atoi(argv[3]));
	luaL_dofile(L,argv[1]);
	strcpy(TempPath,argv[2]);
	printf("Сервер запущен ...\nПуть к скрипту: %s\nВременные файлы: %s\nПорт сервера: %d\nКоманда !help - вызов справки\n\n",argv[1],argv[2],atoi(argv[3]));

	while(1){
		char cmd[128]; gets_s(cmd);
		if(strcmp(cmd,"!help")==0){
			printf("================Команды: ===================\n");
			printf(" >> !help - справка\n");
			printf(" >> !q / !quit - выход из программы\n");
			printf(" >> !restart - перезапуски сервера\n");
			printf(" >> !relua - перезапуск LUA скриптов\n");
			printf(" >> !exclua - выполнить код LUA\n");
			printf("============================================\n");
		}else if((strcmp(cmd,"!q")==0) || (strcmp(cmd,"!quit")==0)){
			exit(0);
		}else if(strcmp(cmd,"!restart")==0){
			printf(">> Перезапуск сервера ...\n");
			DestroyWebServer();
			DestroyLUA();
			CreateLUA();
			luaL_dofile(L,argv[1]);
			CreateWebServer(atoi(argv[3]));
			printf(">> Сервер перезапущен ...\nПуть к скрипту: %s\nПорт сервера: %d\nКоманда !help - вызов справки\n\n",argv[1],atoi(argv[3]));
		}else if(strcmp(cmd,"!relua")==0){
			printf(">> Перезапуск LUA скриптов ...\n");
			RefreshLUA();
			luaL_dofile(L,argv[1]);
			printf(">> LUA скрипты перезапущены\n\n");
		}else if(strcmp(cmd,"!exclua")==0){
			printf(">> LUA:\n");
			char luacode[1024]; gets_s(luacode);
			luaL_dostring(L,luacode);
			printf(">> LUA Выполнено\n\n");
		}else{
			printf(">> Неизвестная команда\n");
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------

void CreateLUA(void){
	L = luaL_newstate();
	luaopen_base(L);
	luaopen_math(L);
	luaopen_string(L);
	luaopen_table(L);
	luaL_openlibs(L);
	lua_register(L,"SendData",Lua_SendData);
	lua_register(L,"SendHTMLPage",Lua_SendHTMLPage);
	lua_register(L,"SendHTMLCode",Lua_SendHTMLCode);
	lua_register(L,"SendImage",Lua_SendImage);
	lua_register(L,"SendFile",Lua_SendFile);
	lua_register(L,"CloseConnection",Lua_CloseConnection);
	luaL_dostring(L,"function OnGetRequest(client,reqest,ipaddress) end");
	luaL_dostring(L,"function OnPostRequest(client,request,thread,ipaddress) end");
}

//------------------------------------------------------------------------------------------------------------

void RefreshLUA(void){
	DestroyLUA();
	CreateLUA();
}

//------------------------------------------------------------------------------------------------------------

void DestroyLUA(void){
	lua_close(L);
}

//------------------------------------------------------------------------------------------------------------

int Lua_SendData(lua_State *st){
	int top = lua_gettop(st);
	if(top == 3){
		char *content_type = (char*)lua_tostring(st,top);
		char *path = (char*)lua_tostring(st,top-1);
		SOCKET client = (SOCKET)lua_tonumber(st,top-2);
		SendData(client,path,content_type);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

int Lua_SendHTMLPage(lua_State *st){
	int top = lua_gettop(st);
	if(top == 3){
		char *path = (char*)lua_tostring(st,top);
		SOCKET client = (SOCKET)lua_tonumber(st,top-1);
		bool keep = lua_toboolean(st,top-2);
		SendHTMLPage(client,path,keep);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

int Lua_SendHTMLCode(lua_State *st){
	int top = lua_gettop(st);
	if(top == 3){
		char *code = (char*)lua_tostring(st,top);
		SOCKET client = (SOCKET)lua_tonumber(st,top-1);
		bool keep = lua_toboolean(st,top-2);
		SendHTMLCode(client,code,keep);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

int Lua_SendImage(lua_State *st){
	int top = lua_gettop(st);
	if(top == 2){
		char *path = (char*)lua_tostring(st,top);
		SOCKET client = (SOCKET)lua_tonumber(st,top-1);
		SendImage(client,path);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

int Lua_SendFile(lua_State *st){
	int top = lua_gettop(st);
	if(top == 3){
		char *filename = (char*)lua_tostring(st,top);
		char *path = (char*)lua_tostring(st,top-1);
		SOCKET client = (SOCKET)lua_tonumber(st,top-2);
		SendFile(client,path,filename);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

int Lua_CloseConnection(lua_State *st){
	int top = lua_gettop(st);
	if(top == 1){
		SOCKET client = (SOCKET)lua_tonumber(st,top);
		closesocket(client);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

void CreateWebServer(unsigned short port){
	memset(&server,0,sizeof(SOCKET));
	memset(&wsd,0,sizeof(WSADATA));
	memset(&localaddr,0,sizeof(sockaddr_in));
	created_threads = 0;
	run_thread_inuse = false;
	for(int i = 0;i<USING_THREADS;i++){
		client_threads_inuse[i] = false;
	}

	if(WSAStartup(MAKEWORD(2,2),&wsd) == 0){
		server = socket(AF_INET,SOCK_STREAM,0);
		if(server != INVALID_SOCKET){
			localaddr.sin_addr.s_addr = INADDR_ANY;
			localaddr.sin_family = AF_INET;
			localaddr.sin_port = htons(port);
			if(::bind(server,(sockaddr*)&localaddr,sizeof(localaddr)) == 0){
				listen(server,SOMAXCONN);
				run_thread_inuse = true;
				_Thrd_create(&run_thread,NetThread,NULL);
			}else{
				printf("WebServer::Ошибка: bind...\n");
				system("PAUSE");
				exit(0);
			}
		}else{
			printf("WebServer::Ошибка: socket...\n");
			system("PAUSE");
			exit(0);
		}
	}else{
		printf("WebServer::Ошибка: WSAStartup...\n");
		system("PAUSE");
		exit(0);
	}
}

//------------------------------------------------------------------------------------------------------------

void DestroyWebServer(void){
	run_thread_inuse = false;
	_Thrd_detach(run_thread);

	for(int i = 0;i<USING_THREADS;i++){
		if(client_threads_inuse[i]){
			client_threads_inuse[i] = false;
			_Thrd_detach(client_threads[i]);
		}
	}

	closesocket(server);
	WSACleanup();
}

//------------------------------------------------------------------------------------------------------------

void SendData(SOCKET client,const char *path, char *content_type){
	ifstream filestream(path,ios::in|ios::binary|ios::ate);
	if(!filestream){
		printf("WebServer::Ошибка: SendData %s - невозможно прочесть файл...\n",path);
		return;
	}

	int len = (int)filestream.tellg();
	filestream.seekg(0,ios::beg);
	char *mem = (char*)malloc(len);
	filestream.read(mem,len);
	filestream.close();

	time_t pTime;
	time(&pTime);
	tm *now = localtime(&pTime);
	char Date[256]; sprintf(Date,"Date: %.2d.%.2d.%.4d %.2d:%.2d:%.2d GMT\n",now->tm_mday,now->tm_mon+1,now->tm_year+1900,now->tm_hour,now->tm_min+1,now->tm_sec);
	char ContLen[128]; sprintf(ContLen,"Content-Length: %d\n",len);

	string http_head;
	http_head += "HTTP/1.1 200 OK\n";
	http_head += Date;
	http_head += "Server: Lua Web Server\n";
	http_head += "Content-Type:";
	http_head += content_type;
	http_head += "\n";
	http_head += ContLen;
	http_head += "Connection: close\n";
	http_head += "\n";

	send(client,http_head.c_str(),http_head.size(),0);
	send(client,mem,len,0);
	closesocket(client);

	free(mem);
}

//------------------------------------------------------------------------------------------------------------

void SendHTMLPage(SOCKET client,const char *path,bool keep_alive){
	ifstream filestream(path,ios::in|ios::binary|ios::ate);
	if(!filestream){
		printf("WebServer::Ошибка: SendHTMLPage %s - невозможно прочесть файл...\n",path);
		return;
	}

	int len = (int)filestream.tellg();
	filestream.seekg(0,ios::beg);
	char *mem = (char*)malloc(len);
	filestream.read(mem,len);
	filestream.close();

	time_t pTime;
	time(&pTime);
	tm *now = localtime(&pTime);
	char Date[256]; sprintf(Date,"Date: %.2d.%.2d.%.4d %.2d:%.2d:%.2d GMT\n",now->tm_mday,now->tm_mon+1,now->tm_year+1900,now->tm_hour,now->tm_min+1,now->tm_sec);
	char ContLen[128]; sprintf(ContLen,"Content-Length: %d\n",len);

	string http_head;
	http_head += "HTTP/1.1 200 OK\n";
	http_head += Date;
	http_head += "Server: Lua Web Server\n";
	http_head += "Content-Type: text/html\n";
	http_head += ContLen;
	if(!keep_alive){
		http_head += "Connection: close\n";
	}else{
		http_head += "Connection: Keep-Alive\n";
	}
	http_head += "\n";

	send(client,http_head.c_str(),http_head.size(),0);
	send(client,mem,len,0);

	if(!keep_alive){
		closesocket(client);
	}

	free(mem);
}

//------------------------------------------------------------------------------------------------------------

void SendHTMLCode(SOCKET client,char *code,bool keep_alive){
	time_t pTime;
	time(&pTime);
	tm *now = localtime(&pTime);
	char Date[256]; sprintf(Date,"Date: %.2d.%.2d.%.4d %.2d:%.2d:%.2d GMT\n",now->tm_mday,now->tm_mon+1,now->tm_year+1900,now->tm_hour,now->tm_min+1,now->tm_sec);
	char ContLen[128]; sprintf(ContLen,"Content-Length: %d\n",strlen(code));

	string http_head;
	http_head += "HTTP/1.1 200 OK\n";
	http_head += Date;
	http_head += "Server: Lua Web Server\n";
	http_head += "Content-Type: text/html\n";
	http_head += ContLen;
	if(!keep_alive){
		http_head += "Connection: close\n";
	}else{
		http_head += "Connection: Keep-Alive\n";
	}
	http_head += "\n";

	send(client,http_head.c_str(),http_head.size(),0);
	send(client,code,strlen(code),0);

	if(!keep_alive){
		closesocket(client);
	}
}

//------------------------------------------------------------------------------------------------------------

void SendImage(SOCKET client,const char *path){
	string img_type(path);
	reverse(img_type.begin(),img_type.end());
	char *type = strtok((char*)img_type.c_str(),".");
	string imgtype(type);
	reverse(imgtype.begin(),imgtype.end());

	ifstream filestream(path,ios::in|ios::binary|ios::ate);
	if(!filestream){
		printf("WebServer::Ошибка: SendImage %s - невозможно прочесть файл...\n",path);
		return;
	}

	int len = (int)filestream.tellg();
	filestream.seekg(0,ios::beg);
	char *mem = (char*)malloc(len);
	filestream.read(mem,len);
	filestream.close();

	time_t pTime;
	time(&pTime);
	tm *now = localtime(&pTime);
	char Date[256]; sprintf(Date,"Date: %.2d.%.2d.%.4d %.2d:%.2d:%.2d GMT\n",now->tm_mday,now->tm_mon+1,now->tm_year+1900,now->tm_hour,now->tm_min+1,now->tm_sec);
	char ContL[128]; sprintf(ContL,"Content-Type: image/%s\n",imgtype.c_str());
	char ContLen[128]; sprintf(ContLen,"Content-Length: %d\n",len);

	string http_head;
	http_head += "HTTP/1.1 200 OK\n";
	http_head += Date;
	http_head += "Server: Lua Web Server\n";
	http_head += "Accept-Ranges: bytes\n";
	http_head += ContL;
	http_head += ContLen;
	http_head += "\n";

	send(client,http_head.c_str(),http_head.size(),0);
	send(client,mem,len,0);
	closesocket(client);

	free(mem);
}

//------------------------------------------------------------------------------------------------------------

void SendFile(SOCKET client,const char *path,char *filename){
	string file_type(path);
	reverse(file_type.begin(),file_type.end());
	char *type = strtok((char*)file_type.c_str(),".");
	string filetype(type);
	reverse(filetype.begin(),filetype.end());

	ifstream filestream(path,ios::in|ios::binary|ios::ate);
	if(!filestream){
		printf("WebServer::Ошибка: SendFile %s - невозможно прочесть файл...\n",path);
		return;
	}

	int len = (int)filestream.tellg();
	filestream.seekg(0,ios::beg);
	char *mem = (char*)malloc(len);
	filestream.read(mem,len);
	filestream.close();

	time_t pTime;
	time(&pTime);
	tm *now = localtime(&pTime);
	char Date[256]; sprintf(Date,"Date: %.2d.%.2d.%.4d %.2d:%.2d:%.2d GMT\n",now->tm_mday,now->tm_mon+1,now->tm_year+1900,now->tm_hour,now->tm_min+1,now->tm_sec);
	char ConstDisp[256]; sprintf(ConstDisp,"Content-Disposition: attachment; filename=%s\n",filename);
	char ContL[128]; sprintf(ContL,"Content-Type: application/%s\n",filetype.c_str());
	char ContLen[128]; sprintf(ContLen,"Content-Length: %d\n",len);

	string http_head;
	http_head += "HTTP/1.1 200 OK\n";
	http_head += Date;
	http_head += "Server: Lua Web Server\n";
	http_head += "Accept-Ranges: bytes\n";
	http_head += ConstDisp;
	http_head += ContL;
	http_head += ContLen;
	http_head += "\n";

	send(client,http_head.c_str(),http_head.size(),0);
	send(client,mem,len,0);
	closesocket(client);

	free(mem);
}

//------------------------------------------------------------------------------------------------------------

string ParseRequest(string request,char *what){
	string ret(request);
	for(int i = 0;i<10;i++){
		ret.pop_back();
	} reverse(ret.begin(),ret.end());
	for(int i = 0;i<(int)strlen(what);i++){
		ret.pop_back();
	} reverse(ret.begin(),ret.end());
	string decoded = UrlDecode(ret);
	return decoded;
}

//------------------------------------------------------------------------------------------------------------

string UrlDecode(string &url){
	string ret; int n;
	for(int i = 0;i<(int)url.length();i++){
		if(int(url[i]) == 37){
			sscanf(url.substr(i+1,2).c_str(),"%x",&n);
			char ch = static_cast<char>(n);
			ret += ch;
			i = i+2;
		}else{
			ret += url[i];
		}
	} return ret;
}

//------------------------------------------------------------------------------------------------------------

int Parse_GetContentLength(string str){
	int ContentLength = -1;
	if(strncmp(str.c_str(),"Content-Length:",strlen("Content-Length:"))==0){
		string using_copy(str);
		char *ContentLengthCommand = strtok((char*)using_copy.c_str()," ");
		char *StrContentLength = strtok(NULL," ");
		ContentLength = atoi(StrContentLength);
	} return ContentLength;
}

//------------------------------------------------------------------------------------------------------------

int NetThread(void *lpParam){
	while(run_thread_inuse){
		sockaddr_in clientaddr;
		int iSize = sizeof(clientaddr);
		SOCKET client = accept(server, (sockaddr*)&clientaddr, &iSize);

		if(client_threads_inuse[created_threads]){
			client_threads_inuse[created_threads] = false;
			_Thrd_detach(client_threads[created_threads]);
		} pthread_data[created_threads].client = client; pthread_data[created_threads].thread_id = created_threads;
		strcpy(pthread_data[created_threads].addr,inet_ntoa(clientaddr.sin_addr));
		client_threads_inuse[created_threads] = true;
		_Thrd_create(&client_threads[created_threads],ClientThread,(void*)&pthread_data[created_threads]);

		created_threads++;
		if(created_threads >= USING_THREADS){
			created_threads = 0;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------

int ClientThread(void *lpParam){
	thread_data *pTData = (thread_data*)lpParam;
	char buff[SIZEOF_RECV_BUFFER];
	vector<char> pData;

	DOWNLOAD_THREAD:{
		while(int size = recv(pTData->client,buff,sizeof(buff),0)){
			for(int i = 0;i<size;i++){
				pData.push_back(buff[i]);
			} if(size < sizeof(buff)){
				break;
			}
		}

		istringstream stream(string(pData.begin(),pData.end()));
		string lines;
		while(getline(stream,lines)){
			int ContentLength = Parse_GetContentLength(lines);
			if(ContentLength != -1){
				if((int)pData.size() < ContentLength){
					goto DOWNLOAD_THREAD;
				}
			}
		} stream.clear();
	};

	istringstream stream(string(pData.begin(),pData.end()));
	string lines;
	while(getline(stream,lines)){
		if(strncmp(lines.c_str(),"GET /",strlen("GET /"))==0){
			string gr = ParseRequest(lines,(char*)"GET /");
			char intnum[8]; sprintf(intnum,"%d",pTData->client);
			string onrequest; onrequest += "OnGetRequest("; onrequest += intnum;
			onrequest += ",\""; onrequest += gr; onrequest += "\",\"";
			onrequest += pTData->addr; onrequest += "\")";
			luaL_dostring(L,onrequest.c_str());
			break;

		}else if(strncmp(lines.c_str(),"POST /",strlen("POST /"))==0){
			char tempath[260]; sprintf(tempath,"%s\\temp_%d.tmp",TempPath,pTData->thread_id);
			ofstream tempout(tempath,ios::out|ios::binary);
			if(tempout){
				tempout.write(pData.data(),pData.size());
				tempout.close();
			} string gr = ParseRequest(lines,(char*)"POST /");
			char intnum[8]; sprintf(intnum,"%d",pTData->client);
			string onrequest; onrequest += "OnPostRequest("; onrequest += intnum;
			onrequest += ",\""; onrequest += gr; onrequest += "\",";
			sprintf(intnum,"%d",pTData->thread_id);
			onrequest += intnum; onrequest += ",";
			onrequest += "\""; onrequest += pTData->addr; onrequest += "\")";
			luaL_dostring(L,onrequest.c_str());
			break;

		}else{
			closesocket(pTData->client);
		}
	} stream.clear();
	pData.erase(pData.begin(),pData.end());
	return 0;
}

//------------------------------------------------------------------------------------------------------------