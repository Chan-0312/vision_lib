#ifndef _CAMERA_H_
#define _CAMERA_H_

/*
*函数名 ： 相机相关函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/6/19
*版本 ： V2.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
	2018.7.3：修改高速分辨率参数的控制参数。
	2018.8.7: 修改BUG无法重复打开同一标号相机，加入留位机制。
	2018.8.16:修改BUG无法重复打开高速相机的，原因是分辨率设置的问题。
	2018.10.19:加入CameraInfo相机信息结构体，可以同时打开多个高速相机
	2018.11.01:修复加入CameraInfo相机信息结构体无法关闭相机的bug
*/



#include <iostream>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "CameraApi.h"


using namespace cv;
using namespace std;

// 高速相机上限个数
#define Maxnum_Camera_High 8


/*
* 相机信息结构体
*/
typedef struct  CameraInformation{
	// 相机型号 0普通相机 1 高速相机 -1 表示相机经释放
	int Camera_model;
 	// 普通相机类
	VideoCapture Camera_cap;
	// 高速相机类
	int	Camera_hCamera;
	// 高速相机图像通道数
	int Camera_channel;		
	// 高速相机数据缓存区
	unsigned char *Camera_pRgbBuffer; 
}CameraInfo;


/*
*函数名：	Camera_init
*功能：	  相机初始化
*函数参数：	
int model		- 相机型号
				- 0 普通免驱相机
				- 1 迈德高速相机
int cap_nume	- 相机标号

*返回值：				
int			- 0表成功
*/
int Camera_init(int model,int cap_nume = 0);

/*
*函数名：	Camera_close
*功能：	  释放相机
*函数参数：	
int cap_nume	- 相机号	
*返回值：				
int				- 0表成功
*/
int Camera_close(int cap_nume = -1);

/*
*函数名：	Camera_readimage
*功能：	  读取相机图像
*函数参数：	
int cap_num	- 相机编号
*返回值：				
Mat			- Mat图像
*/
Mat Camera_readimage(int cap_nume = 0);

/*
*函数名：	Camera_resolution
*功能：	  设置相机分辨率
*函数参数：	
int width		- 分辨率宽
int height		- 分辨率高
int iHOffsetFOV - X方向偏移值
int iVOffsetFOV - Y方向偏移值
int cap_nume	- 相机编号

*返回值：				
int			- 返回0表成功
*/
int Camera_resolution(int width,int height,int iHOffsetFOV=0, int iVOffsetFOV=0,int cap_nume = 0);

/*
*函数名：	Camera_exposure
*功能：	  设置相机曝光时间
*函数参数：	
cap_exposure	- 曝光时间
				- 0 为自动曝光
int cap_nume	- 相机编号
*返回值：				
int				- 返回0表成功
*/
int Camera_exposure(int exposure,int cap_nume = 0);



#endif


