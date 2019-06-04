#ifndef Client_H
#define Client_H

/*
*函数名 ： Client函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/10/30
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：

*/




#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include <vector>

using namespace std;


/*
* 服务器数据结构体
*/
typedef struct  ClientInformation{
	// 服务器端口号
	long Server_port;
	// 服务器IP地址
	char Server_ipaddr[20]; 
	// 服务器fd Server_fds = -1 表示已经端口连接
	int Server_fds;
}ClientInfo;


/*
*函数名：    Client_write  
*功能：  客户端写   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
const char* data				- 数据
*返回值：	
int								- 0表发送成功
*/
int Client_write(ClientInfo Data_ClientInfo, const char* data);



/*
*函数名：    Client_read  
*功能：  客户端读   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
*返回值：	
char*							- 返回字符串
*/
char* Client_read(ClientInfo Data_ClientInfo);



/*
*函数名：    Client_close  
*功能：  关闭客户端   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
*返回值：	
int								- 0表成功
*/
int Client_close(ClientInfo Data_ClientInfo);


/*
*函数名：    Client_open  
*功能：  创建客户端，连接服务器   
*函数参数：  
long port		- 服务器端口号
char* addr		- 服务IP地址
*返回值：	
int				- 0表连接成功
*/
int Client_open(long port,const char* ipaddr);



#endif


