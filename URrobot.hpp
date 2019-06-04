#ifndef URrobot_H
#define URrobot_H

/*
*函数名 ： UR机器人通信控制函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/4/26
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
	2018.7.3： 将以前程序自动读取数据，改为用户控制读取数据。
	2018.8.8:  修改robot_open函数，自动选择是否开启服务器的功能
	2018.8.25: 加入标定程序函数包括，Navy、Tsai算法等。
	2018.8.29：加入双目标定坐标转换公式。
	2018.8.30: 加入畸变转换函数Distor_Correc。
	2018.9.04：修改Pixel2XYZ_3d为最小二乘法计算。
	2018.9.07: 修改机器人xml配置文件为外部参数。
	2018.10.10：将之前通信等待时间变为1000us.
	2018.12.19：加入UR主机通信模式
	2018.12.20: 优化修改了通信的时间等待。
	2019.01.07：优化robot_slave_read函数，加入model参数的返回。
	2019.01.07：优化Pixel2Robot函数，加入了自动读取标定模式的参数。
	2019.01.09: 修改了calib_read_ExternalM函数，手眼标定通信方式。
	2019.01.15: 修改了Pixel2Robot函数，支持XYZ三个屏幕的坐标转换。
*/



#include <iostream>
#include <cmath>
#include <string>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

#include <net/if.h>
#include <sys/ioctl.h>


#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

#include "Server.hpp"
#include "Client.hpp"



using namespace cv;
using namespace std;

/*
* 机器人信息结构体
*/
typedef struct  RobotInformation{
	// 机器人位置Pose
	vector<double> Robot_pose;
	// 机器人请求事件号
	int Robot_event;
	// 机器人识别模式
	int Robot_model;
	// 机器人识别方向
	int Robot_direction;
	// 物体高度
	double Robot_OHeight;
}RobotInfo;


/*
*函数名：		ReadXml_mat  
*功能：		读取xml矩阵	 
*函数参数：  
const char *filename		- xml文件名
const char *datanama 		- 矩阵名			
Mat& data			- 存放返回矩阵
*返回值：	
int					- 0 读取成功
*/ 
static int ReadXml_mat(const      char *filename ,const char *datanama, Mat& data);

/*
*函数名：	 	RobotData2CoordMatrix  
*功能：		机器人数据转旋转矩阵	 
*函数参数：  
vector<double> rdata		- 机器人输入数据（x,y,z,rx,ry,rz）
Mat& Tr						- 存放返回矩阵
*返回值：	
void						- 无
*/ 
void RobotData2CoordMatrix(vector<double> rdata, Mat& obj);

/*
*函数名：    Pixel2Robot  
*功能：      像素坐标转机器人坐标   
*函数参数：  
int mode				- 转换模式
						0 眼在手上（转基坐标）
						1 眼在手上（转工具端坐标）
						2 眼在手外（转基坐标）
						-1 读M_MOD参数判断
Point2d coord			- 像素坐标
double z				- 物体高度
vector<double> rdata	- 机器人初始位置(除0模式时，该参数可忽略)
int xyz_mode			- 计算XYZ坐标的模式
						0 已知Z值，求XY坐标（默认）
						1 已知Y值，求XZ坐标
						2 已知X值，求YZ坐标
*返回值：	
Point2d					- 机器人坐标
*/  
Point2d Pixel2Robot(int mode, Point2d coord, double z, vector<double> rdata = vector<double>(),int xyz_mode=0);

/*
*函数名：    strsplit  
*功能：  字符串分割   
*函数参数：  
const string &s, 		- string字符串
const string &seperator - 分割符
*返回值：	
vector<string>			- 分割后的字符串向量
*/ 
static vector<string> strsplit(const string &s, const string &seperator);

/*
*函数名：    str2rdata  
*功能：  机器人字符串数据转数值   
*函数参数：  
string str		- string字符串
*返回值：	
vector<double>	- 数值
*/ 
vector<double> str2rdata(string str);

/*
*函数名：    robot_open  
*功能：     开启socket通信与机器人通信   
*函数参数：  
void	-空

*返回值：	
vector<double>	- 返回机器人初始位置姿态
*/
vector<double> robot_open(void);

/*
*函数名：    socket_close  
*功能：     关闭socket通信   
*函数参数：  
void	- 空
*返回值：	
void	- 空
*/
void socket_close(void);

/*
*函数名：    send_robot  
*功能：      发送数据给机器人   
*函数参数：  
vector<double> rdata	- 发送数据
*返回值：	
int					- 返回0表成功
*注意：
数据格式如下：
读取/控制机器人信息： 	0.0 		Flag
							Flag = 0.0 		读取当前机器人位置
							Flag = 1.0		设置当前位置为机器人的相对移动位置对比位置		
绝对移动位置：			1.0 x y z rx ry rz
相对移动位置：			2.0 x y z rx ry rz
IO控制：			3.0 IO0 IO1 IO2 IO3 IO4 IO5
AD控制：			4.0 AD0 AD1 AD2 AD3 AD4 AD5
触发事件：			5.0	x y angle RES RES 触发事件
*/  
int send_robot(vector<double> rdata);

/*
*函数名：	  robot_read  
*功能：      读取机器人ok数据   
*函数参数：  
void			- 空
*返回值：	
int				- 返回0表成功
*/ 
int robot_read(void);

/*
*函数名:		read_pose  
*功能：     	读取机器人当前位置   
*函数参数：  
void			- 空
*返回值：	
vector<double>	- 返回机器人当前位置姿态
*/
vector<double> read_pose(void);

/*
*函数名:		set_relative_pose  
*功能：  	   	设置当前位置为机器人相对移动位置   
*函数参数：  
void			- 空
*返回值：	
vector<double>	- 返回机器人当前位置姿态
*/
vector<double> set_relative_pose(void);

/*
*函数名:		robot_move  
*功能：     	机器人移动pose   
*函数参数：  
vector<double> rdata	- 移动pose (x,y,z,rx,ry,rz)
int mod					- 移动模式 0 绝对移动 1 相对移动
*返回值：	
int						- 返回0表示移动成功
*/
int robot_move(vector<double> rdata, int mod);

/*
*函数名:		robot_cmd  
*功能：     	机器人io/ad控制  
*函数参数：  
vector<double> rdata	- io/ad控制数据
*返回值：	
int						- 返回0表示控制成功
*/
int robot_cmd(vector<double> rdata,int mod);

/*
*函数名:		robot_event  
*功能：     	机器人   触发事件
*函数参数： 
vector<double> rdata	- (x y angle RES RES)
double event			- 触发事件
*返回值：	
int						- 返回0表成功
*/
int robot_event(vector<double> rdata, double event);

/*
*函数名:		calib_read_ExternalM  
*功能：     	手眼标定专用，读取UR姿态外参数矩阵
*函数参数： 
void			- 空
*返回值：	
Mat				- 返回机器人外参数矩阵
*/
Mat calib_read_ExternalM(void);

/*
*函数名:		calib_Tsai  
*功能：     	手眼标定专用，求解手眼关系矩阵Tsai算法
*函数参数： 
vector<Mat> robotExternalMList		- 机器人外参数矩阵
vector<Mat> imageExternalMList		- 图片外参数矩阵
int rdata_mod						- 标定模式		-0 眼在手上 -1 眼在手外
*返回值：	
Mat									- 返回Tm关系矩阵
*/
Mat calib_Tsai(vector<Mat> robotExternalMList,vector<Mat> imageExternalMList,int rdata_mod);

/*
*函数名:		calib_Navy  
*功能：     	手眼标定专用，求解手眼关系矩阵Navy算法
*函数参数： 
vector<Mat> robotExternalMList		- 机器人外参数矩阵
vector<Mat> imageExternalMList		- 图片外参数矩阵
int rdata_mod						- 标定模式		-0 眼在手上 -1 眼在手外
*返回值：	
Mat									- 返回Tm关系矩阵
*/
Mat calib_Navy(vector<Mat> robotExternalMList,vector<Mat> imageExternalMList,int rdata_mod);

/*
*函数名:		calib_crossprod  
*功能：     	手眼标定专用，求解叉积函数
*函数参数： 
Mat a		- 输入Mat
*返回值：	
Mat			- 返回Mat

*/
static Mat calib_crossprod(Mat a);

/*
*函数名:		calib_crossprod  
*功能：     	手眼标定专用，矩阵对数函数
*函数参数： 
Mat a		- 输入Mat
*返回值：	
Mat			- 返回Mat

*/
static Mat calib_logMatrix(Mat M);


/*
*函数名：    Pixel2XYZ_3d  
*功能：      像素坐标转XYZ(双目) 
*函数参数：  
vector<Point2d> coord	- 像素坐标 coord[0](左相机) coord[1](右相机)
Mat m					- 左相机关系矩阵
Mat M					- 右相机关系矩阵
*返回值：	
Mat						- 机器人XYZ坐标
*/  
Mat	Pixel2XYZ_3d(vector<Point2d> coord,Mat m,Mat M);


/*
*函数名：    Distor_Correc  
*功能：     像素畸变转换
*函数参数：  
Point2d image_point		- 像素坐标
Mat Min					- 相机内参
Mat kc					- 畸变系数
*返回值：	
Point2d					- 畸变矫正后的坐标
*/ 
Point2d Distor_Correc(Point2d image_point, Mat Min, Mat kc);

/*
*函数名：    Cmd_URrobot_send  
*功能：     向UR端port端口发送data数据
*函数参数：  
long port			- 端口号
const char* data	- 发送数据
*返回值：	
int				- 0表成功
*/
int Cmd_URrobot_send(long port, const char* data);

/*
*函数名：    Cmd_URrobot_move  
*功能：     通过UR端口30001控制UR移动
*函数参数：  
int mod			- 移动模式 
				  0、1、2分别为xyz方向的偏移;
				  3、4、5 分别为rx ry rz偏移;
int distance	- distance 移动变量 
				  移动模式为 0、1、2时单位为mm
				  移动模式为 3、4、5时单位为角度
int speed		- speed 速度 1-20
*返回值：	
int				- 0表成功
*/
int Cmd_URrobot_move(int mod,int distance,int speed);


/*
*函数名：    robot_slave_read  
*功能：     上位机从机等待读取UR信号
*函数参数：  
void - 空
*返回值：	
RobotInfo	- 返回机器人信息结构体
*/
RobotInfo robot_slave_read(void);






#endif


