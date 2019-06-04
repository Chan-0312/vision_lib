#ifndef Server_H
#define Server_H

/*
*函数名 ： Select服务器函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/6/21
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
	2018.7.3： 修改服务器发送数据的逻辑关系。
	2018.7.4： 将原来的单机器人通信升级为多机器人同时通信。
	2018.7.4： 定义Data_ServerInfo服务器数据结构体，外部引用。
	2018.8.11：加入服务器关闭函数。
	2018.8.30：读写去除了Clinet_amount变量的限制。
	2018.12.21: 修改关闭服务器成功后置0
	2019.3.27: 解决服务器重新启动select failure错误的文件描述符的BUG
*/



#include <stdio.h>
#include <netinet/in.h>   //for souockaddr_in
#include <sys/types.h>      
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

//for select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include <strings.h>   //for bzero
#include <string.h>

#include <pthread.h>



#define BUFF_SIZE 1024	// 缓存大小
#define backlog 10		// 最大连接请求队列数
#define CLI_NUM 10		// 最大客户端数

/*
* 服务器数据结构体
*/
typedef struct  ServerInformation{
	// 服务器端口号
	int Server_port;
	// 服务器IP地址
	char Server_ipaddr[20]; 
	// 服务器状态		0 无 1连接 -1 断开
	int Server_state;
	// 客户端连接总数	
	int Clinet_amount;
	// 通信客户端状态 		       0 无 1 连接 -1 断开
	int Clinet_state[CLI_NUM];
	// 通信客户端读状态	       0 无 1 可读
	int Clinet_r[CLI_NUM];
	// 通信客户端写状态	       0 无 1 可写
	int Clinet_w[CLI_NUM];
	// 读写缓存数据
	char rwdata[CLI_NUM][BUFF_SIZE];
}ServerInfo;

 /*
*函数名：    Server_write  
*功能：  Server写  
*函数参数：  
int num			- 客户端号
				- 如num = -1 表示全部连接的客户端都发送。
char* data		- 数据
*返回值：	
int				- 0表成功
*/
int Server_write(int num, const char* data);

/*
*函数名：    Server_read  
*功能：  Server读 （阻塞读） 
*函数参数：  
int num			- 客户端号
*返回值：	
char* data		- 数据
*/
char* Server_read(int num);

/*
*函数名：    Server_open  
*功能：  Server线程服务器   
*函数参数：  
int port		- 服务器端口号
char* addr		- 服务IP地址（可省略，省略时系统自动识别192.168.*.*地址）
*返回值：	
int				- 0表成功
*/ 
int Server_open(int port, const char* ipaddr= "0");

/*
*函数名：    Server_close  
*功能：  关闭Server线程服务器   
*函数参数：  
void 			- 空
*返回值：	
int				- 0表成功
*/ 
int Server_close(void);


/*
*函数名：    Server_arg  
*功能：  Server线程函数   
*函数参数：  
void *arg		- 线程参数传递结构体
*返回值：	
void*			- 空
*/  
static void* Server_arg(void *arg);

/*
*函数名：    Server  
*功能：  开启Select服务器   
*函数参数：  
int port		- 服务器端口号
char* addr		- 服务IP地址（可省略，省略时系统自动识别192.168.*.*地址）
*返回值：	
int				- 死循环不返回
*/  
static int Server(int port,const char* ipaddr = "0");


#endif


