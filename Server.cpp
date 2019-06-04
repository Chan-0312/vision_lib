#include "Server.hpp"

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

// 外部定义服务器数据结构体
extern ServerInfo Data_ServerInfo;

// 客户端数组
int client_fds[CLI_NUM];

// debug	1 输出调试信息
int debug = 0;

// 服务器线程
pthread_t Server_t;

// 服务器fd
int ser_souck_fd;	



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
int Server_write(int num,const char* data)
{
	if (num >= 0)
	{
		if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_state[num] == 1))
		{
			
			memset(Data_ServerInfo.rwdata[num],0,sizeof(Data_ServerInfo.rwdata[num]));
			strcpy(Data_ServerInfo.rwdata[num],data);
			Data_ServerInfo.Clinet_w[num] = 1;
			
			if(client_fds[num] != 0)
			{
				send(client_fds[num], Data_ServerInfo.rwdata[num], BUFF_SIZE, 0);
				if (debug != 0)
					printf("Write[%d]:%s\n",num,Data_ServerInfo.rwdata[num]);
				Data_ServerInfo.Clinet_w[num] = 0;
			}
			else
			{
				printf("Write[%d]:Send failed!\n",num);
			}
			
			return 0;
		}
		else
		{
			printf("Unable to write, Please confirm the client connection\n");
			return 1;
		}
	}
	else if (num == -1)
	{
		if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_amount > 0) )
		{
			for (int i = 0; i < CLI_NUM; i++)
			{
				memset(Data_ServerInfo.rwdata[i],0,sizeof(Data_ServerInfo.rwdata[i]));
				strcpy(Data_ServerInfo.rwdata[i],data);
				Data_ServerInfo.Clinet_w[i] = 1;
				/*
				while(1)
				{
					if (Data_ServerInfo.Clinet_w[i] == 0)
						break;
				}
				*/
				
				if(client_fds[i] != 0)
				{
					send(client_fds[i], Data_ServerInfo.rwdata[i], BUFF_SIZE, 0);
					if (debug != 0)
						printf("Write[%d]:%s\n",i,Data_ServerInfo.rwdata[i]);
					Data_ServerInfo.Clinet_w[i] = 0;
				}
				else
				{
					printf("Write[%d]:Send failed!\n",i);
				}
				
				
			}
			return 0;
		}
		else
		{
			printf("Unable to write, Please confirm the client connection\n");
			return 1;
		}
	}
	else
	{
		printf("Incoming parameter error\n");
		return 1;
	}
}

/*
*函数名：    Server_read  
*功能：  Server读 （阻塞读） 
*函数参数：  
int num			- 客户端号
*返回值：	
char* data		- 数据
*/
char* Server_read(int num)
{
	if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_state[num] == 1))
	{
		while(1)
		{
			if (Data_ServerInfo.Clinet_r[num] == 1)
			{
				Data_ServerInfo.Clinet_r[num] = 0;
				return Data_ServerInfo.rwdata[num];
			}
			usleep(1000);
			printf("Reread!\n");
			//usleep(10000);
		}
	}
	else
	{
		printf("Unable to read, Please confirm the client connection\n");
		return NULL;
	}
}

                
/*
*函数名：    Server_open  
*功能：  Server线程服务器   
*函数参数：  
int port		- 服务器端口号
char* addr		- 服务IP地址（可省略，省略时系统自动识别192.168.*.*地址）
*返回值：	
int				- 0表成功
*/  
int Server_open(int port, const char* ipaddr)
{	
	Data_ServerInfo.Server_port = port;
	
	if (ipaddr[0] != '0')
	{
		memset(Data_ServerInfo.Server_ipaddr,0,sizeof(Data_ServerInfo.Server_ipaddr));
		strcpy(Data_ServerInfo.Server_ipaddr,ipaddr);
	}
	else
	{
		memset(Data_ServerInfo.Server_ipaddr,0,sizeof(Data_ServerInfo.Server_ipaddr));
		strcpy(Data_ServerInfo.Server_ipaddr,"0"); 
	}
	
	// 创建线程
	if(pthread_create(&Server_t, NULL, Server_arg, NULL) == -1)	
	{
		perror("Pthread_create() error");
		exit(1);
	}
	return 0;
}

/*
*函数名：    Server_close  
*功能：  关闭Server线程服务器   
*函数参数：  
void 			- 空
*返回值：	
int				- 0表成功	1-表失败
*/ 
int Server_close(void)
{
	if (Data_ServerInfo.Server_state == 1)
	{
		Data_ServerInfo.Server_state = -1;
		return 0;
	}
	else
		return 1;
}


/*
*函数名：    Server_arg  
*功能：  Server线程函数   
*函数参数：  
void *arg		- 线程参数传递结构体
*返回值：	
void*			- 空
*/  
static void* Server_arg(void *arg)
{	
	Server(Data_ServerInfo.Server_port,Data_ServerInfo.Server_ipaddr);	
	pthread_join(Server_t,0);
	pthread_exit(0);
}

/*
*函数名：    Server  
*功能：  开启Select服务器   
*函数参数：  
int port		- 服务器端口号
char* addr		- 服务IP地址（可省略，省略时系统自动识别192.168.*.*地址）
*返回值：	
int				- 死循环不返回
*/  
static int Server(int port,const char* ipaddr)
{
	// 初始化ServerInfo结构体
	Data_ServerInfo.Server_state = 0;
	Data_ServerInfo.Clinet_amount = 0;
	for (int i = 0; i < CLI_NUM ; i++)
	{
		Data_ServerInfo.Clinet_state[i] = 0;
		Data_ServerInfo.Clinet_r[i] = 0;
		Data_ServerInfo.Clinet_w[i] = 0;
		memset(Data_ServerInfo.rwdata[i],0,sizeof(Data_ServerInfo.rwdata[i]));
	}

    int i;   
    char input_message[BUFF_SIZE];
    char resv_message[BUFF_SIZE];

    //creat socket
    if( (ser_souck_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        perror("creat failure");
        return -1;
    } 
	// socket通信设置
	struct sockaddr_in ser_addr;
	ser_addr.sin_family= AF_INET;			//IPV4
	ser_addr.sin_port = htons(port);		// 端口号

	if (ipaddr[0] == '0')					// 自动获取IP地址
	{
		 // 获取本机IP
		char *myip;
		struct ifreq myip_buf[INET_ADDRSTRLEN];
		struct ifconf ifc;
		int intrface, retn = 0;
	    ifc.ifc_len = sizeof(myip_buf);
	    ifc.ifc_buf = (caddr_t)myip_buf;
	    if (!ioctl(ser_souck_fd, SIOCGIFCONF, (char *)&ifc))
	    {
	     	intrface = ifc.ifc_len/sizeof(struct ifreq);
	    	while (intrface-- > 0)
	        {
	       		if (!(ioctl(ser_souck_fd, SIOCGIFADDR, (char *)&myip_buf[intrface])))
	         	{
	         		myip=(inet_ntoa(((struct sockaddr_in*)(&myip_buf[intrface].ifr_addr))->sin_addr));
	       			if (!strncmp(myip, "192.168.",8)) 
						break;
	         	}
	   		}
		}
		ipaddr = myip;	
	}
	
	ser_addr.sin_addr.s_addr = inet_addr(ipaddr);	//指定IP地址

	//强制释放被占用端口
	int opt = 1;
	if (setsockopt(ser_souck_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0 )		
		perror("Setsockopt failed!");

    //bind soucket
    if(bind(ser_souck_fd, (const struct sockaddr *)&ser_addr,sizeof(ser_addr)) < 0)
    {
        perror("bind failure");
        return -1;
    }

    //listen
    if(listen(ser_souck_fd, backlog) < 0) 
    {
        perror("listen failure"); 
        return -1;
    }

    //fd_set
    fd_set ser_fdset;
    int max_fd=1;
    struct timeval mytime;
	// 进入循环等待客户端连接
	Data_ServerInfo.Server_state = 1;
	printf("Server IP Address:%s\n",ipaddr);
	printf("Port:%d\n",port);
    printf("wait for client connnect!\n");

    while(1)
    {
		//usleep(500);
		if (Data_ServerInfo.Server_state == -1)	// 关闭服务器标志
		{
			for(int i = 0;i < CLI_NUM;i++)	// 关闭客户端
			{
				if (Data_ServerInfo.Clinet_state[i] == 1)
				{
					close(client_fds[i]);
					client_fds[i] = 0;
					Data_ServerInfo.Clinet_state[i] = 0;
				}
			}
			// 关闭服务器
			close(ser_souck_fd);
			Data_ServerInfo.Server_state = 0;
			break;
		}
		
        mytime.tv_sec=3;		//3
        mytime.tv_usec=0;		//0

        FD_ZERO(&ser_fdset);

        //add standard input
        //FD_SET(0,&ser_fdset);
		
        if(max_fd < 0)
        {
            max_fd=0; 
        }

        //add serverce
        FD_SET(ser_souck_fd,&ser_fdset);
        if(max_fd < ser_souck_fd)
        {
            max_fd = ser_souck_fd;
        }

        //add client 
        for(i=0;i<CLI_NUM;i++)  //用数组定义多个客户端fd
        {
            if(client_fds[i]!=0) 
            {
                FD_SET(client_fds[i],&ser_fdset);
                if(max_fd < client_fds[i])
                {
                    max_fd = client_fds[i]; 
                }
            }
        }
		/*
		// 可写
		for (i = 0; i < CLI_NUM; i++)
		{
			if (Data_ServerInfo.Clinet_w[i] == 1)
			{
				if(client_fds[i] != 0)
				{
					send(client_fds[i], Data_ServerInfo.rwdata[i], sizeof(Data_ServerInfo.rwdata[i]), 0);
					if (debug != 0)
						printf("Write[%d]:%s\n",i,Data_ServerInfo.rwdata[i]);
					memset(Data_ServerInfo.rwdata[i],0,sizeof(Data_ServerInfo.rwdata[i]));
					Data_ServerInfo.Clinet_w[i] = 0;
				}
				else
				{
					printf("Write[%d]:Send failed!\n",i);
				}
			}
		}
		*/
		
        //select多路复用
        int ret = select(max_fd + 1, &ser_fdset, NULL, NULL, &mytime);

        if(ret < 0)    
        {   
        	if (debug != 0)
            	perror("select failure\n");    
            continue;    
        }    
        else if(ret == 0)
        {
        	//if (debug != 0)
            	//printf("Server time out!\n");
            continue;
        }
        else
        {
        	/*
            if(FD_ISSET(0,&ser_fdset)) //标准输入是否存在于ser_fdset集合中（也就是说，检测到输入时，做如下事情）
            {
                printf("send message to");
                bzero(input_message,BUFF_SIZE);
                fgets(input_message,BUFF_SIZE,stdin);

                for(i=0;i<CLI_NUM;i++)
                {
                    if(client_fds[i] != 0)
                    {
                        printf("client_fds[%d]=%d\n", i, client_fds[i]);
                        send(client_fds[i], input_message, BUFF_SIZE, 0);
                    }
                }

            }
			*/	
			
            if(FD_ISSET(ser_souck_fd, &ser_fdset)) 
            {
                struct sockaddr_in client_address;
                socklen_t address_len;
                int client_sock_fd = accept(ser_souck_fd,(struct sockaddr *)&client_address, &address_len);
                if(client_sock_fd > 0)
                {
                    int flags=-1;
                    //一个客户端到来分配一个fd，CLI_NUM=3，则最多只能有三个客户端，超过4以后跳出for循环，flags重新被赋值为-1
                    for(i=0;i<CLI_NUM;i++)
                    {
                        if(client_fds[i] == 0)
                        {
                            flags=i; 
                            client_fds[i] = client_sock_fd;
                            break;
                        }
                    }

                    if (flags >= 0)
                    {
                    	Data_ServerInfo.Clinet_amount++;
						Data_ServerInfo.Clinet_state[flags] = 1;
						if (debug != 0)
                        	printf("new user client[%d] add sucessfully!\n",flags);

                    }
                    else //flags=-1		//连接数超过上限
                    {   
						printf("Too many client connections\n");
                    }
                }    
            }

        }

        // 可读
        for(i=0; i<CLI_NUM; i++)
        {
            if(client_fds[i] != 0)
            {
                if(FD_ISSET(client_fds[i],&ser_fdset))
                {
                    bzero(resv_message,BUFF_SIZE);
                    int byte_num=read(client_fds[i],resv_message,BUFF_SIZE);
                    if(byte_num > 0)
                    {
                    	if (debug != 0)
                        	printf("message form client[%d]:%s\n", i, resv_message);
						// 可读	
						memset(Data_ServerInfo.rwdata[i],0,sizeof(Data_ServerInfo.rwdata[i]));
						strcpy(Data_ServerInfo.rwdata[i],resv_message);
						Data_ServerInfo.Clinet_r[i] = 1;
                    }
                    else if(byte_num < 0)
                    {
                    	if (debug != 0)
                        	printf("rescessed error!\n");
                    }
                    //某个客户端退出
                    else  //cancel fdset and set fd=0
                    {
                    	Data_ServerInfo.Clinet_amount--;
						Data_ServerInfo.Clinet_state[i] = -1;
						if (debug != 0)
                        	printf("clien[%d] exit!\n",i);
                        FD_CLR(client_fds[i], &ser_fdset);
                        client_fds[i] = 0;
                        continue;  //这里如果用break的话一个客户端退出会造成服务器也退出。  
                    }
                }
            }
        }    
    }
    return 0;
}

