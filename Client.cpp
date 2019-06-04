#include "Client.hpp"

/*
*函数名 ： Client函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/10/30
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：

*/


// 外部定义客户端数据结构体
extern vector<ClientInfo> VData_ClientInfo;


/*
*函数名：    Client_write  
*功能：  客户端写   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
const char* data				- 数据
*返回值：	
int								- 0表发送成功
*/
int Client_write(ClientInfo Data_ClientInfo, const char* data)
{
	if (Data_ClientInfo.Server_fds != -1)
	{
		send(Data_ClientInfo.Server_fds, data, strlen(data), 0);
		return 0;
	}
	else
	{
		printf("Unable to write, Please confirm the client connection\n");
		return -1;
	}
		
}

/*
*函数名：    Client_read  
*功能：  客户端读   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
*返回值：	
char*							- 返回字符串
*/
char* Client_read(ClientInfo Data_ClientInfo)
{
	if (Data_ClientInfo.Server_fds != -1)
	{
		static char buf[512];
		memset(buf,0,sizeof(buf));
		read(Data_ClientInfo.Server_fds,buf,sizeof(buf));
		return buf;
	}
	else
	{
		printf("Unable to read, Please confirm the client connection\n");
		return NULL;
	}
}

/*
*函数名：    Client_close  
*功能：  关闭客户端   
*函数参数：  
ClientInfo Data_ClientInfo		- 客户端数据结构体
*返回值：	
int								- 0表成功
*/
int Client_close(ClientInfo Data_ClientInfo)
{
	if (Data_ClientInfo.Server_fds != -1)
	{
		for(int i = 0; i < VData_ClientInfo.size(); i++)
		{
			if (VData_ClientInfo[i].Server_fds == Data_ClientInfo.Server_fds)
			{
				VData_ClientInfo[i].Server_fds = -1;
				close(Data_ClientInfo.Server_fds);
				return 0;
			}
		}
	}
	
	printf("Do not close the client repeatedly\n");
	return -1;
}




/*
*函数名：    Client_open  
*功能：  创建客户端，连接服务器   
*函数参数：  
long port		- 服务器端口号
char* addr		- 服务IP地址
*返回值：	
int				- 0表连接成功
*/
int Client_open(long port, const char* ipaddr)
{
    int s_fd;	
	struct sockaddr_in s_in;
	memset((void *)&s_in,0,sizeof(s_in));
	
    s_in.sin_family = AF_INET;
    s_in.sin_port = htons(port);
    inet_pton(AF_INET,ipaddr,(void *)&s_in.sin_addr);

    s_fd = socket(AF_INET,SOCK_STREAM,0);
	
    /*
    面向连接的客户程序使用connect函数来配置socket并与远端服务器建立一个TCP连接
    int connect(int sockfd, struct sockaddr *serv_addr,int addrlen)
    sockfd是socket函数返回的socket描述符；
    serv_addr是包含远端主机IP地址和端口号的指针；
    addrlen是远端地址结构的长度
    */		
	if(connect(s_fd,(struct sockaddr *)&s_in,sizeof(s_in)) < 0)
	{
		perror("bind failure");
		return -1;
	}


	if (VData_ClientInfo.size() > 0)
	{
		int flag = -1;
		for(int i = 0; i < VData_ClientInfo.size(); i++)
		{
			if (VData_ClientInfo[i].Server_fds == -1)
			{
				flag = i;
				break;
			}
		}
		if (flag != -1)
		{
			VData_ClientInfo[flag].Server_fds = s_fd;
			memset(VData_ClientInfo[flag].Server_ipaddr,0,sizeof(VData_ClientInfo[flag].Server_ipaddr));
			strcpy(VData_ClientInfo[flag].Server_ipaddr,ipaddr);
			VData_ClientInfo[flag].Server_port = port;
		}
		else
		{
			ClientInfo Data_ClientInfo;
			Data_ClientInfo.Server_fds = s_fd;
			memset(Data_ClientInfo.Server_ipaddr,0,sizeof(Data_ClientInfo.Server_ipaddr));
			strcpy(Data_ClientInfo.Server_ipaddr,ipaddr);
			Data_ClientInfo.Server_port = port;
			VData_ClientInfo.push_back(Data_ClientInfo);
		}

	}
	else
	{
		ClientInfo Data_ClientInfo;
		Data_ClientInfo.Server_fds = s_fd;
		memset(Data_ClientInfo.Server_ipaddr,0,sizeof(Data_ClientInfo.Server_ipaddr));
		strcpy(Data_ClientInfo.Server_ipaddr,ipaddr);
		Data_ClientInfo.Server_port = port;
		VData_ClientInfo.push_back(Data_ClientInfo);
	}

	return 0;
}

