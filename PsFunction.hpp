#ifndef PsFunction_H
#define PsFunction_H

/*
*函数名 ： PS函数(CPU/GPU版)
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2019/02/21
*版本 ： V1.0
	Copyright (C), 2012-2019 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
	2019.02.21： 加入cuda(GPU)图像处理功能函数。
*/

#include <iostream>

// opencv 头文件
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>   
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

 // 相机头文件
#include "Camera.hpp"  
// 图像处理算法头文件        
#include "VisionArithmetic.hpp"

// opencv Cuda头文件
#ifdef TAI_GPU
#include "opencv2/cudaimgproc.hpp"	
#include "opencv2/cudafilters.hpp"
#include "opencv2/cudabgsegm.hpp"
#endif

int PS_init();

#endif



