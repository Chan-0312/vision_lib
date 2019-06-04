#include "Camera.hpp"

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


// 相机信息结构体
vector<CameraInfo> CCInfo;		


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
int Camera_init(int model,int cap_nume)
{
	CameraInfo CInfo;
	
	if (model == 0)
	{
		VideoCapture Capture;
		Capture.open(cap_nume);
    	if (!Capture.isOpened())
    	{
       		cerr << "\nFailed to open a video device or video file!" << endl;
        	return -1;
    	}
		else
		{
			int flag = -1;
			for(int i = 0; i<CCInfo.size(); i++)
			{
				if (CCInfo[i].Camera_model == -1)
				{
					flag = i;
					break;
				}
			}
			if (flag >= 0)
			{
				CCInfo[flag].Camera_model = 0;
				CCInfo[flag].Camera_cap = Capture;				
			}
			else
			{
				CInfo.Camera_model = 0;
				CInfo.Camera_cap = Capture;
				CCInfo.push_back(CInfo);
			}
			return 0;
		}
	}
	else if (model == 1)
	{
		CInfo.Camera_model = 1;
		
		int                     iCameraCounts = Maxnum_Camera_High;
		int                     iStatus=-1;
		tSdkCameraDevInfo       tCameraEnumList[Maxnum_Camera_High];
		tSdkCameraCapbility     tCapability;      //设备描述信息
		CameraSdkInit(0);
		//枚举设备，并建立设备列表
		CameraEnumerateDevice(tCameraEnumList,&iCameraCounts);
		//没有连接设备
		if(iCameraCounts==0)
		{
			cerr << "Failed to open a video device or video file!\n" << endl;
			return -1;
		}

		// 相机标号
		if ((cap_nume > (iCameraCounts-1)) || (cap_nume < 0))
			cap_nume = 0;
		

		//相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
		
		iStatus = CameraInit(&tCameraEnumList[cap_nume],0,0,&CInfo.Camera_hCamera);
		
		//初始化失败
		if(iStatus!=CAMERA_STATUS_SUCCESS)
		{
			cerr << "Camera initialization failed!\n" << endl;
			return -1;
		}
		//获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
		CameraGetCapability(CInfo.Camera_hCamera,&tCapability);
		// 图像缓存
		CInfo.Camera_pRgbBuffer = (unsigned char*)malloc(tCapability.sResolutionRange.iHeightMax*tCapability.sResolutionRange.iWidthMax*3);

		/*让SDK进入工作模式，开始接收来自相机发送的图像
		数据。如果当前相机是触发模式，则需要接收到
		触发帧以后才会更新图像。    */
		CameraPlay(CInfo.Camera_hCamera);
		if(tCapability.sIspCapacity.bMonoSensor)
		{
			CInfo.Camera_channel = 1;
			CameraSetIspOutFormat(CInfo.Camera_hCamera,CAMERA_MEDIA_TYPE_MONO8);
		}else
		{
			CInfo.Camera_channel = 3;
			CameraSetIspOutFormat(CInfo.Camera_hCamera,CAMERA_MEDIA_TYPE_BGR8);
		}
		
		int flag = -1;
		for(int i = 0; i<CCInfo.size(); i++)
		{
			if (CCInfo[i].Camera_model == -1)
			{
				flag = i;
				break;
			}
		}
		if (flag >= 0)
		{
			CCInfo[flag].Camera_model = 1;
			CCInfo[flag].Camera_hCamera = CInfo.Camera_hCamera;	
			CCInfo[flag].Camera_channel = CInfo.Camera_channel;
			CCInfo[flag].Camera_pRgbBuffer = CInfo.Camera_pRgbBuffer;
		}
		else
		{
			CCInfo.push_back(CInfo);
		}
			
		return 0;
	}
	else
	{
		cerr << "The model was not found!\n" << endl;
		return -1;
	}
}


/*
*函数名：	Camera_close
*功能：	  释放相机
*函数参数：	
int cap_nume	- 相机号	
*返回值：				
int				- 0表成功
*/
int Camera_close(int cap_nume)
{
	if (cap_nume != -1)
	{
		if (CCInfo.size() < cap_nume)
		{
			cerr << "The camera was not found\n" << endl;
			return -1;
		}
		
		if ( CCInfo[cap_nume].Camera_model == 0)
		{
			CCInfo[cap_nume].Camera_cap.release();
			
			CCInfo[cap_nume].Camera_model = -1;	//留位

			return 0;
		}
		else if( CCInfo[cap_nume].Camera_model == 1)
		{
			CameraUnInit(CCInfo[cap_nume].Camera_hCamera);
			free(CCInfo[cap_nume].Camera_pRgbBuffer);
			CCInfo[cap_nume].Camera_model = -1;	//留位
			return 0;
		}
		else
			return -1;
	}
	else
	{
		for(int i = 0; i < CCInfo.size();i++)
		{
			if ( CCInfo[i].Camera_model == 0)
			{
				CCInfo[i].Camera_cap.release();
			}
			else if( CCInfo[i].Camera_model == 1)
			{
				CameraUnInit(CCInfo[i].Camera_hCamera);
				free(CCInfo[i].Camera_pRgbBuffer);
			}
			CCInfo[i].Camera_model = -1;	//留位
		}
		return 0;
	}
	
}


/*
*函数名：	Camera_readimage
*功能：	  读取相机图像
*函数参数：	
int cap_num	- 相机编号
*返回值：				
Mat			- Mat图像
*/
Mat Camera_readimage(int cap_nume)
{
	Mat src;
	if (CCInfo.size() < cap_nume)
	{
		cerr << "The camera was not found\n" << endl;
		return src;
	}
	
	if ( CCInfo[cap_nume].Camera_model == 0)
	{
		CCInfo[cap_nume].Camera_cap >> src;
	}
	else if( CCInfo[cap_nume].Camera_model == 1)
	{
			
		IplImage *iplImage = NULL;
		tSdkFrameHead	sFrameInfo;
		BYTE*	pbyBuffer;
		if(CameraGetImageBuffer(CCInfo[cap_nume].Camera_hCamera,&sFrameInfo,&pbyBuffer,1000) == CAMERA_STATUS_SUCCESS)
		{
		
			CameraImageProcess(CCInfo[cap_nume].Camera_hCamera, pbyBuffer, CCInfo[cap_nume].Camera_pRgbBuffer,&sFrameInfo);
			if (iplImage)
			{
				cvReleaseImageHeader(&iplImage);
			}
			iplImage = cvCreateImageHeader(cvSize(sFrameInfo.iWidth,sFrameInfo.iHeight),IPL_DEPTH_8U,CCInfo[cap_nume].Camera_channel);
			cvSetData(iplImage,CCInfo[cap_nume].Camera_pRgbBuffer,sFrameInfo.iWidth*CCInfo[cap_nume].Camera_channel);//此处只是设置指针，无图像块数据拷贝，不需担心转换效率
			src = cvarrToMat(iplImage);

			//释放buffer
			CameraReleaseImageBuffer(CCInfo[cap_nume].Camera_hCamera,pbyBuffer);	
		}
		
	}
	
	return 	src;
}

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
int Camera_resolution(int width,int height,int iHOffsetFOV, int iVOffsetFOV,int cap_nume)
{	
	if (CCInfo.size() < cap_nume)
	{
		cerr << "The camera was not found\n" << endl;
		return -1;
	}
	
	if ( CCInfo[cap_nume].Camera_model == 0)
	{
		CCInfo[cap_nume].Camera_cap.set(CV_CAP_PROP_FRAME_WIDTH, width);  
		CCInfo[cap_nume].Camera_cap.set(CV_CAP_PROP_FRAME_HEIGHT,height);
		return 0;
	}
	else if ( CCInfo[cap_nume].Camera_model == 1)
	{
		tSdkImageResolution 	sResolution_read;
		CameraGetImageResolution(CCInfo[cap_nume].Camera_hCamera,&sResolution_read);
		//sResolution_set.iIndex = 255;
		sResolution_read.iHOffsetFOV = iHOffsetFOV;
		sResolution_read.iVOffsetFOV = iVOffsetFOV;
		sResolution_read.iWidthFOV = width;
		sResolution_read.iHeightFOV= height;
		sResolution_read.iWidth = width;
		sResolution_read.iHeight= height;
		CameraSetImageResolution(CCInfo[cap_nume].Camera_hCamera,&sResolution_read);

		return 0;
	}
	else
	{
		return -1;
	}
}

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
int Camera_exposure(int exposure,int cap_nume)
{
	if (CCInfo.size() < cap_nume)
	{
		cerr << "The camera was not found\n" << endl;
		return -1;
	}
	
	if ( CCInfo[cap_nume].Camera_model == 0)
	{
		if (exposure != 0)
		{
			Mat src;
			CCInfo[cap_nume].Camera_cap >> src;
			//将曝光时间调为手动设置
			system("v4l2-ctl --set-ctrl=exposure_auto=1");
			// 设置曝光时间
			char c_exposure[50];
			//memset(c_exposure,0,50);
			sprintf(c_exposure,"v4l2-ctl --set-ctrl=exposure_absolute=%d",exposure);	
			system(c_exposure);
		}
		else
			system("v4l2-ctl --set-ctrl=exposure_auto=0");
		return 0;
	}
	else if ( CCInfo[cap_nume].Camera_model == 1)
	{
		if (exposure != 0)
		{
			exposure = exposure*100;
			CameraSetAeState(CCInfo[cap_nume].Camera_hCamera,FALSE);//TRUE:自动曝光,FALSE:手动曝光
			CameraSetExposureTime(CCInfo[cap_nume].Camera_hCamera,exposure);
		}
		else
		{
			CameraSetAeState(CCInfo[cap_nume].Camera_hCamera,TRUE);//TRUE:自动曝光,FALSE:手动曝光
		}
		return 0;
	}
	else
	{
		return -1;
	}
}


