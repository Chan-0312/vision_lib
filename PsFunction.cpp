#include "PsFunction.hpp"

/*
*函数名 ： PS函数(GPU版)
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2019/02/21
*版本 ： V1.0
	Copyright (C), 2012-2019 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
*/

// 外部定义相机源图
extern Mat cap_src;

int show_w = 1280;                  // 显示界面分辨率宽
int show_h = 1024;                  // 显示界面分辨率高

// 识别物体信息数组，便于外部调用
vector<GraphicInfo> PS_Ginfo;

static int PS_Fun1(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun2(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun3(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun4(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun5(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun6(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun7(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun8(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun9(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun10(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );

static int PS_Fun11(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun12(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun13(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun14(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun15(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun16(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun17(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun18(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun19(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun20(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );

static int PS_Fun21(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun22(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun23(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun24(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun25(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun26(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun27(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun28(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun29(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun30(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );

static int PS_Fun31(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun32(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun33(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun34(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun35(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun36(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun37(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun38(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun39(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun40(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );

static int PS_Fun41(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun42(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun43(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun44(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun45(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun46(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun47(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun48(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun49(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );
static int PS_Fun50(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values );


// PS_FUN函数数组
int (*PS_FUN[])(InputArray, OutputArray, vector<int>) = {
	PS_Fun1,PS_Fun2,PS_Fun3,PS_Fun4,PS_Fun5,
	PS_Fun6,PS_Fun7,PS_Fun8,PS_Fun9,PS_Fun10,
	PS_Fun11,PS_Fun12,PS_Fun13,PS_Fun14,PS_Fun15,
	PS_Fun16,PS_Fun17,PS_Fun18,PS_Fun19,PS_Fun20,
	PS_Fun21,PS_Fun22,PS_Fun23,PS_Fun24,PS_Fun25,
	PS_Fun26,PS_Fun27,PS_Fun28,PS_Fun29,PS_Fun30,
	PS_Fun31,PS_Fun32,PS_Fun33,PS_Fun34,PS_Fun35,
	PS_Fun36,PS_Fun37,PS_Fun38,PS_Fun39,PS_Fun40,
	PS_Fun41,PS_Fun42,PS_Fun43,PS_Fun44,PS_Fun45,
	PS_Fun46,PS_Fun47,PS_Fun48,PS_Fun49,PS_Fun50
	};


/*
 * PS调用初始化
 */
int PS_init()
{
#ifdef TAI_GPU
	static int flag = 0;
	if (flag == 0)
	{
		int num_devices = cuda::getCudaEnabledDeviceCount(); 
		if (num_devices <= 0){  
		    cerr << "There is no device." << endl;  
		    return -1;  
		}  
	  
		int enable_device_id = -1;  
		for (int i = 0; i < num_devices; i++){  
		    cuda::DeviceInfo dev_info(i);  
		    if (dev_info.isCompatible()){  
		        enable_device_id = i;  
		    }  
		}

	 	if (enable_device_id < 0){  
		    cerr << "GPU module isn't built for GPU" << endl;  
		}
		cuda::setDevice(enable_device_id);  
		flag = -1;
	}
	
	return 1;
#endif

#ifdef TAI_CPU
	
#endif
	return 0;
}

/*
 * 相机曝光
 */
static int PS_Fun1(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	static int exposure_buff = 300;
    if (exposure_buff != parameter_values[0])
    {
        Camera_exposure(parameter_values[0]);
        exposure_buff = parameter_values[0];
    }
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif

#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * 灰度化
 */
static int PS_Fun2(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	if (in_mat.channels() == 3)
    {
	#ifdef TAI_GPU
        cuda::cvtColor(in_mat,out_mat,CV_RGB2GRAY);
	#endif
	#ifdef TAI_CPU
	 	cv::cvtColor(in_mat,out_mat,CV_RGB2GRAY);
	#endif
    }
	else
	{
	#ifdef TAI_GPU
        in_mat.getGpuMat().copyTo(out_mat);
	#endif
	#ifdef TAI_CPU
	 	in_mat.getMat().copyTo(out_mat);
	#endif
		
	}

	return 0;
}
/*
 * 滤波
 */
static int PS_Fun3(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	Size size(parameter_values[0],parameter_values[0]);
	int mode = parameter_values[1];
#ifdef TAI_GPU
	Ptr<cuda::Filter> GPU_Filter;
	cuda::GpuMat g_src = in_mat.getGpuMat();
	if (mode == 0)		// 方框滤波
	{
		if (in_mat.channels() != 1)
		{
			cuda::cvtColor(in_mat,g_src,CV_RGB2GRAY);
		}
		GPU_Filter =  cuda::createBoxFilter(g_src.type(), g_src.type(), size);	//创建方框滤波器
	}
	else if (mode == 1)	// 高斯滤波
	{	
		if (size.width % 2 == 0)
		{
			size.width++;
		}
		if (size.height % 2 == 0) 
		{
			size.height++;
		}
		if  (size.width > 31)
		{
			size.width = 31;
		}
		if  (size.height > 31)
		{
			size.height = 31;
		}
		double sigma_x = 0.3*((size.width-1)*0.5 - 1) + 0.8;
		double sigma_y = 0.3*((size.height-1)*0.5 - 1) + 0.8;
		GPU_Filter = cuda::createGaussianFilter(g_src.type(), g_src.type(), size, sigma_x, sigma_y); //创建高斯滤波器 	
		
	}
	else if (mode == 2)	// 中值滤波
	{
		if (size.width < 3)
		{
			size.width = 3;
		}
		if (in_mat.channels() != 1)
		{
			cuda::cvtColor(in_mat,g_src,CV_RGB2GRAY);
		}
		GPU_Filter = cuda::createMedianFilter(g_src.type(), size.width); //创建中值滤波器 	
	}
	else if (mode == 3) 	// 双边滤波 
	{
		cuda::bilateralFilter(in_mat,out_mat,size.width,size.width/2,size.width*2);
	}
	else if (mode == 4)		// 拉普拉斯滤波器
	{
		if (in_mat.channels() != 1)
		{
			cuda::cvtColor(in_mat,g_src,CV_RGB2GRAY);
		}
		if (size.width >= 3)
		{
			size.width = 3;
		}
		else
		{
			size.width = 1;
		}
		GPU_Filter = cuda::createLaplacianFilter(g_src.type(),g_src.type(),size.width);	//创建拉普拉斯滤波器 	
	}
	else
	{
		cout<<"Did not find the change mode."<<endl;
		return -1;
	}

	if ( mode != 3 )
	{
		GPU_Filter->apply(g_src, out_mat);	//滤波
	}

#endif

#ifdef TAI_CPU
	switch(mode)
	{
		case 0:	//方框滤波
		{
			cv::boxFilter(in_mat,out_mat,-1,size);
			break;
		}
		case 1:	//高斯滤波
		{
			//高斯滤波核必须为奇数
			if (size.width % 2 == 0)
			{
				size.width++;
			}
			if (size.height % 2 == 0) 
			{
				size.height++;
			}
			cv::GaussianBlur(in_mat,out_mat,size,0,0);
			break;
		}
		case 2:	//中值滤波
		{
			//中值滤波核必须为奇数
			if (size.width % 2 == 0)
			{
				size.width++;
			}
			cv::medianBlur(in_mat,out_mat,size.width);
			break;
		}
		case 3:	//双边滤波
		{
			//双边滤波核必须为奇数
			if (size.width % 2 == 0)
			{
				size.width++;
			}
			Mat buff_image;
			cv::bilateralFilter(in_mat,buff_image,size.width,size.width/2,size.width*2);
			buff_image.copyTo(out_mat);
			break;
		}
		case 4:	//拉普拉斯滤波器
		{
			if (size.width >= 3)
			{
				size.width = 3;
			}
			else
			{
				size.width = 1;
			}
			cv::Laplacian(in_mat,out_mat,in_mat.type(),size.width);
			break;
		}
	}
#endif

	return 0;
}
/*
 * 对比度/曝光	
 */
static int PS_Fun4(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	if (in_mat.channels() == 3)
	{
		cuda::GpuMat temp(in_mat.size(), CV_8UC3, Scalar(parameter_values[1],parameter_values[1],parameter_values[1]));
		cuda::scaleAdd(in_mat,(double)parameter_values[0]/100.0,temp,out_mat);	
	}
	else if (in_mat.channels() == 1)
	{
		cuda::GpuMat temp(in_mat.size(), CV_8UC1, Scalar(parameter_values[1]));
		cuda::scaleAdd(in_mat,(double)parameter_values[0]/100.0,temp,out_mat);	
	}
	else
	{
		in_mat.getGpuMat().copyTo(out_mat);
		return -1;
	}

#endif
#ifdef TAI_CPU
	if (in_mat.channels() == 3)
	{
		Mat img = in_mat.getMat();
		Mat out;
		ContrastControl(img,out, (double)parameter_values[0]/100.0,parameter_values[1]);
		out.copyTo(out_mat);
	}
#endif
	return 0;
}
/*
 * 阈值化
 */
static int PS_Fun5(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{ 
	int type = parameter_values[2];
#ifdef TAI_GPU
	if (!((type >= 0) && (type <= 4)))
	{
		type = 0;
	}
	cuda::threshold(in_mat, out_mat, parameter_values[0], parameter_values[1], type);
#endif

#ifdef TAI_CPU
	Mat temp = in_mat.getMat();
	if (in_mat.channels() == 3)
	{
		cv::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	if (type <= 4)
		cv::threshold(temp, out_mat, parameter_values[0], parameter_values[1] , type);
	else if (type <= 6)
		cv::threshold(temp, out_mat, parameter_values[0], parameter_values[1] , type+2);
	else
		cv::threshold(temp, out_mat, parameter_values[0], parameter_values[1] , CV_THRESH_TRIANGLE);
#endif
	return 0;
}
/*
 * 直方图均化
 */
static int PS_Fun6(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{

	if (in_mat.channels() == 1)
	{
	#ifdef TAI_GPU
		cuda::equalizeHist(in_mat,out_mat);
	#endif

	#ifdef TAI_CPU
		cv::equalizeHist(in_mat,out_mat);
	#endif
		return 0;
	}
	else if (in_mat.channels() == 3)
	{
	#ifdef TAI_GPU
		cuda::GpuMat imageRGB[3];
		cuda::split(in_mat, imageRGB);
		for (int i = 0; i < 3; i++)
		{
			cuda::equalizeHist(imageRGB[i],imageRGB[i]);
		}
		cuda::merge(imageRGB, 3, out_mat);
	#endif

	#ifdef TAI_CPU
		Mat imageRGB[3];
		Mat temp = in_mat.getMat();
		cv::split(temp, imageRGB);
		for (int i = 0; i < 3; i++)
		{
			cv::equalizeHist(imageRGB[i], imageRGB[i]);
		}
		cv::merge(imageRGB, 3, out_mat);
	#endif
		return 0;
	}
	return -1;


	
}
/*
 * 自适应阈值化		GPU未完成
 */
static int PS_Fun7(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif

#ifdef TAI_CPU
	int pos = parameter_values[1];
	Mat temp = in_mat.getMat();
	if (pos % 2 == 0)
	{
		pos++;
	}
	if (in_mat.channels() != 1)
	{
		cv::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	cv::adaptiveThreshold(temp, out_mat, 255, parameter_values[0], THRESH_BINARY, pos, 5);
#endif
	return 0;
}
/*
 * 形态学处理
 */
static int PS_Fun8(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	int mode = parameter_values[2];
	int shape = parameter_values[1];
	
	if ((shape < 0) || (shape > 2))
	{
		shape = 0;
	}
	// 创建核
	Mat elem = getStructuringElement( shape, Size(parameter_values[0], parameter_values[0]));

#ifdef TAI_GPU
	if ((mode < 0) || (mode > 6))
	{
		mode = 0;
	}
	cuda::GpuMat temp = in_mat.getGpuMat();
	if (in_mat.channels() != 1)
	{
		cuda::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	
	// 形态学处理
	Ptr<cuda::Filter> GPU_MorphologyEx = cuda::createMorphologyFilter(mode,temp.type(), elem);
	
	GPU_MorphologyEx->apply(temp, out_mat);
#endif

#ifdef TAI_CPU
	// 形态学处理
	Mat temp = in_mat.getMat();
	if ((mode == 7) && (in_mat.channels() != 1))
	{
		cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	morphologyEx(temp, out_mat, mode, elem);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun9(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun10(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun11(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun12(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun13(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun14(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun15(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun16(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun17(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun18(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun19(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun20(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * 颜色过滤
 */
static int PS_Fun21(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	if (in_mat.channels() == 3)
	{
		if (parameter_values[6] == 0)
		{
		#ifdef TAI_GPU
			int mode = 0;
			cuda::GpuMat hsv;
			cuda::GpuMat shsv[3];
			cuda::GpuMat thresc[3];
			cuda::GpuMat temp;
			cuda::GpuMat result;

			// 转换成HSV
			cuda::cvtColor(in_mat, hsv, COLOR_BGR2HSV);

			// 分割成H,S,V三通道
			cuda::split(hsv, shsv);

			// HSV过滤
			// 范围是0-255
			cuda::threshold(shsv[0], thresc[0], 255*parameter_values[0]/360, 255, THRESH_TOZERO);
			cuda::threshold(shsv[1], thresc[1], parameter_values[2], 255, THRESH_TOZERO);
			cuda::threshold(shsv[2], thresc[2], parameter_values[4], 255, THRESH_TOZERO);
			cuda::threshold(thresc[0], thresc[0], 255*parameter_values[1]/360, 255, THRESH_BINARY_INV);
			cuda::threshold(thresc[1], thresc[1], parameter_values[3], 255, THRESH_BINARY_INV);
			cuda::threshold(thresc[2], thresc[2], parameter_values[5], 255, THRESH_BINARY_INV);

			// 按位合并通道
			cuda::bitwise_and(thresc[0], thresc[1],temp);
			cuda::bitwise_and(temp, thresc[2], result);

			if (mode != 0)	// 彩色
			{
				in_mat.getGpuMat().copyTo(out_mat,result);
			}
			else			// 灰度
			{
				result.copyTo(out_mat);
			}
		#endif

		#ifdef TAI_CPU
			Hsv hsv;
			hsv.hmin = parameter_values[0];
			hsv.hmax = parameter_values[1];
			hsv.smin = parameter_values[2];
			hsv.smax = parameter_values[3];
			hsv.vmin = parameter_values[4];
			hsv.vmax = parameter_values[5];
			Mat src,dst;
			src = in_mat.getMat();
			ColorFilter(src,dst,&hsv,0);
			dst.copyTo(out_mat);
		#endif
			return 0;
		}
	}
	else
	{
	#ifdef TAI_GPU
		in_mat.getGpuMat().copyTo(out_mat);
	#endif
	#ifdef TAI_CPU
		in_mat.getMat().copyTo(out_mat);
	#endif
	}
	
	return 0;
}
/*
 * 边缘检测
 */
static int PS_Fun22(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	#ifdef TAI_GPU
	cuda::GpuMat temp = in_mat.getGpuMat();
	if (in_mat.channels() != 1)
	{
		cuda::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	Ptr<cuda::CannyEdgeDetector> canny_edg = cuda::createCannyEdgeDetector(parameter_values[0], parameter_values[1]);  

	canny_edg->detect(temp,out_mat);
	#endif

	#ifdef TAI_CPU
	Mat temp = in_mat.getMat();
	if (in_mat.channels() != 1)
	{
		cv::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}
	Canny(temp,out_mat,parameter_values[0],parameter_values[1],3);
	#endif
	return 0;
}
/*
 * 移动物识别
 */
static int PS_Fun23(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	static Ptr<cuda::BackgroundSubtractorMOG2> mog = cuda::createBackgroundSubtractorMOG2();
	mog->apply(in_mat,out_mat,(double)parameter_values[0]/1000);
#endif

#ifdef TAI_CPU
	//混合高斯背景建模变量
	static Ptr<cv::BackgroundSubtractorMOG2> mog = cv::createBackgroundSubtractorMOG2();
	//背景建模
	mog->apply(in_mat, out_mat, (double)parameter_values[0]/1000);
#endif

	return 0;
}
/*
 * GrabCut过滤		GPU未完成
 */
static int PS_Fun24(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
	return 0;
#endif
#ifdef TAI_CPU
	if (in_mat.channels() != 3)
	{
		in_mat.getMat().copyTo(out_mat);
		return -1;
	}
	Mat src = in_mat.getMat();
	if (parameter_values[0] == 0)
	{
		if  (parameter_values.size() > 2 ) 
		{ 
			int ax,ay,bx,by;
			// 坐标转换
			ax = parameter_values[2]*src.cols/show_w;
			ay = parameter_values[3]*src.rows/show_h;
			bx = parameter_values[4]*src.cols/show_w;
			by = parameter_values[5]*src.rows/show_h;
			Rect selection = Rect(ax,ay, abs(bx - ax), abs(by -ay));
			
			Mat result; // 4种可能结果
			Mat bgModel, fgModel; // 背景/前景 

			grabCut(src, result, selection, bgModel, fgModel, 1, GC_INIT_WITH_RECT);
			compare(result,GC_PR_FGD,result,CMP_EQ);//得到前景mask

			if (parameter_values[1] == 0)	// 黑白
			{
				result.copyTo(out_mat);
				return 0;
			}
			else	// 彩色
			{
				Mat foreground(src.size(),CV_8UC3,Scalar::all(0));
				src.copyTo(foreground,result);
				foreground.copyTo(out_mat);
				return 0;
			}
		}
		src.copyTo(out_mat);
		return -1;
	}
	else if (parameter_values[0] == 1)
	{
		Mat out_img = src.clone();
		if (parameter_values[2] != -1)
		{
			int ax,ay,bx,by;
			// 坐标转换
			ax = parameter_values[2]*out_img.cols/show_w;
			ay = parameter_values[3]*out_img.rows/show_h;
			bx = parameter_values[4]*out_img.cols/show_w;
			by = parameter_values[5]*out_img.rows/show_h;
			rectangle(out_img, Rect(ax,ay, abs(bx - ax), abs(by - ay)), Scalar(156,177,242),1);
		}
		out_img.copyTo(out_mat);
		return 0;
	}
	src.copyTo(out_mat);
	return 0;
#endif
}
/*
 * res
 */
static int PS_Fun25(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun26(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun27(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun28(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun29(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun30(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun31(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun32(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun33(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun34(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun35(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun36(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun37(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun38(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun39(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun40(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * 矩形识别
 */
static int PS_Fun41(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	Mat dst_image;
#ifdef TAI_GPU
	in_mat.getGpuMat().download(dst_image); 
#endif
#ifdef TAI_CPU
	dst_image = in_mat.getMat();
#endif

	if (dst_image.channels() != 1)
		cv::cvtColor(dst_image,dst_image,CV_RGB2GRAY);

	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(dst_image,contours,hierarchy,parameter_values[0],CHAIN_APPROX_SIMPLE);

	RectLimitInfo R_Info;
	R_Info.min_WL = (double)parameter_values[1]/100.0;
	R_Info.max_WL = (double)parameter_values[2]/100.0;
	R_Info.min_w = parameter_values[3];
	R_Info.max_w = parameter_values[4];

	// 矩形识别
	PS_Ginfo.clear();
	Mat temp;
	PS_Ginfo = TakeTriangleRectangle(cap_src,temp,contours,R_Info);

#ifdef TAI_GPU
	cuda::GpuMat out(temp);
	out.copyTo(out_mat);
#endif
#ifdef TAI_CPU
	temp.copyTo(out_mat);
#endif
	return 0;
}
/*
 * 霍夫圆检测
 */
static int PS_Fun42(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{

	Mat dst_image;
#ifdef TAI_GPU
	cuda::GpuMat temp = in_mat.getGpuMat();
	if (in_mat.channels() != 1)
	{
		cuda::cvtColor(in_mat,temp,CV_RGB2GRAY);
	}

	if (parameter_values[2] >= parameter_values[3])
	{
		int temp = parameter_values[3];
		parameter_values[3] = parameter_values[2]+1;
		parameter_values[2] = temp;
	}
	Ptr<cuda::HoughCirclesDetector> GPU_HoughCircles = cuda::createHoughCirclesDetector(2, temp.rows/4, parameter_values[0], parameter_values[1], parameter_values[2], parameter_values[3]);
	GPU_HoughCircles->detect(temp,out_mat);

	// 下载到CPU;
	out_mat.getGpuMat().download(dst_image);
#endif	

#ifdef TAI_CPU
	Mat temp = in_mat.getMat();
	if (in_mat.channels() != 1)
		cv::cvtColor(in_mat,temp,CV_RGB2GRAY);

	cv::HoughCircles(temp, dst_image, CV_HOUGH_GRADIENT, 2, temp.rows/4, \
				 parameter_values[0], parameter_values[1], \
				parameter_values[2], parameter_values[3]);
	
#endif	
	PS_Ginfo.clear();
	
	Mat src_image = cap_src.clone();

	// 查找圆	
	for(int i=0;i<dst_image.cols;i++)
	{
		Point center(dst_image.at<Vec3f>(0, i)[0], dst_image.at<Vec3f>(0, i)[1]);	  
  		int radius = dst_image.at<Vec3f>(0, i)[2];
		cv::circle(src_image, center, 3, Scalar(0,255,0), -1, 8, 0 );
		cv::circle(src_image, center, radius, Scalar(0,0,255), 3, 8, 0 );
		
		GraphicInfo G_Info;
		G_Info.Graphic_name = 'C';
		G_Info.Graphic_Point = center;
		G_Info.Graphic_radius = radius;
		PS_Ginfo.push_back(G_Info);
	}

#ifdef TAI_GPU
	// 上传到GPU
	cuda::GpuMat d_src_img(src_image);
	d_src_img.copyTo(out_mat);
#endif
#ifdef TAI_CPU
	src_image.copyTo(out_mat);
#endif
	
	return 0;
}
/*
 * 模板匹配
 */
static int PS_Fun43(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	static int flag = 0;
	static Mat img;
	if (parameter_values[0] == 0)
	{
		flag = 0;
		if ( (!in_mat.empty()) && (parameter_values.size() > 3)) 
		{ 
			if (in_mat.channels() != img.channels())
			{
			#ifdef TAI_GPU
				in_mat.getGpuMat().copyTo(out_mat);
			#endif
			#ifdef TAI_CPU
				in_mat.getMat().copyTo(out_mat);
			#endif
				return -1;
			}
			int ax,ay,bx,by;
			// 坐标转换
			ax = parameter_values[3]*img.cols/show_w;
			ay = parameter_values[4]*img.rows/show_h;
			bx = parameter_values[5]*img.cols/show_w;
			by = parameter_values[6]*img.rows/show_h;
			Mat obj = img(Rect(ax,ay, abs(bx - ax), abs(by -ay)));
		#ifdef TAI_GPU	
			cuda::GpuMat d_obj(obj);
			cuda::GpuMat result;
			Ptr<cuda::TemplateMatching> GPU_TemplateMatch = cuda::createTemplateMatching(in_mat.type(), parameter_values[1]);
			GPU_TemplateMatch->match(in_mat, d_obj, result);
			// 归一化
			cuda::normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

			double minVal,maxVal; 
			Point minLoc; 
			Point maxLoc;
			Point matchLoc;

			// 查找最大最小值
			cuda::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc);

			if( parameter_values[1]  == CV_TM_SQDIFF || parameter_values[1] == CV_TM_SQDIFF_NORMED )
			{ 
				matchLoc = minLoc; 
			}
		  	else
			{	
				matchLoc = maxLoc; 
			}

			Mat dst_image;
			in_mat.getGpuMat().download(dst_image); 
			rectangle(dst_image, matchLoc,Point( matchLoc.x + obj.cols , matchLoc.y + obj.rows ),Scalar(156,177,242));
			Point center(matchLoc.x + obj.cols/2, matchLoc.y + obj.rows/2);
			PS_Ginfo.clear();
			GraphicInfo G_Info;
			G_Info.Graphic_name = 'R';
			G_Info.Graphic_Point = center;
			G_Info.Graphic_angle = 0;
			G_Info.Graphic_size = Size(obj.cols,obj.rows);
			PS_Ginfo.push_back(G_Info);

			cuda::GpuMat out(dst_image);
			out.copyTo(out_mat);
			return 0;
		#endif	
			
		#ifdef TAI_CPU
			vector<Rect> rect;
			Mat temp = in_mat.getMat();
			TemplateMatch(temp, obj, rect, parameter_values[1], parameter_values[2]);
			PS_Ginfo.clear();
			for (int i = 0; i < parameter_values[2]; ++i)
	        {
	            rectangle(temp, rect[i], Scalar(156,177,242), 2);
	            Point center(rect[i].x + rect[i].width/2, rect[i].y + rect[i].height/2);
	            circle(temp, center, 4, Scalar(0, 255, 0), -1);
				GraphicInfo G_Info;
				G_Info.Graphic_name = 'R';
				G_Info.Graphic_Point = center;
				G_Info.Graphic_angle = 0;
				G_Info.Graphic_size = Size(rect[i].width,rect[i].height);
				PS_Ginfo.push_back(G_Info);
	        }
			temp.copyTo(out_mat);
			return 0;
		#endif	
		}
	#ifdef TAI_GPU
		in_mat.getGpuMat().copyTo(out_mat);
	#endif
	#ifdef TAI_CPU
		in_mat.getMat().copyTo(out_mat);
	#endif	
		return -1;	
	}
	else if ((parameter_values[0] == 1) && (flag != 1))
	{
		flag = 1;	
	#ifdef TAI_GPU
		in_mat.getGpuMat().download(img); 
	#endif
	#ifdef TAI_CPU
		img = in_mat.getMat();
	#endif	
	}
	else if ((parameter_values[0] == 1) && (flag == 1))
	{
		Mat out_img = img.clone();
		if (parameter_values[3] != -1)
		{
			int ax,ay,bx,by;
			// 坐标转换
			ax = parameter_values[3]*out_img.cols/show_w;
			ay = parameter_values[4]*out_img.rows/show_h;
			bx = parameter_values[5]*out_img.cols/show_w;
			by = parameter_values[6]*out_img.rows/show_h;

			rectangle(out_img, Rect(ax,ay, abs(bx - ax), abs(by -ay)), Scalar(156,177,242),1);
		}
	#ifdef TAI_GPU
		cuda::GpuMat d_out_img(out_img);
		d_out_img.copyTo(out_mat);
	#endif
	#ifdef TAI_CPU
		out_img.copyTo(out_mat);
	#endif	
		return 0;
	}
#ifdef TAI_GPU
	cuda::GpuMat d_img(img);
	d_img.copyTo(out_mat);
#endif
#ifdef TAI_CPU
	img.copyTo(out_mat);
#endif	
	return 0;
}
/*
 * 识别范围限制
 */
static int PS_Fun44(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
	Mat img;
#ifdef TAI_GPU
	in_mat.getGpuMat().download(img);
#endif
#ifdef TAI_CPU
	img = in_mat.getMat();
#endif	

	int ax,ay,bx,by;
	// 坐标转换
	ax = parameter_values[0]*img.cols/show_w;
	ay = parameter_values[2]*img.rows/show_h;
	bx = parameter_values[1]*img.cols/show_w;
	by = parameter_values[3]*img.rows/show_h;

	Rect rect = Rect(ax,ay, abs(bx - ax), abs(by -ay));
	rectangle(img, rect, Scalar(0,255,0), 2);

	if (PS_Ginfo.size()>0)
	{
		vector<GraphicInfo> PS_Ginfo_buff;;
		for(int i = 0;i < PS_Ginfo.size();i++)
		{
			Point2d centre = Point2d((ax+bx)/2,(ay+by)/2);
			Size  Graphic_size = Size(abs(bx - ax),abs(by -ay));
			if (IsPointInMatrix(centre,Graphic_size,PS_Ginfo[i].Graphic_Point))
			{
				PS_Ginfo_buff.push_back(PS_Ginfo[i]);
			}
		}
		PS_Ginfo.clear();
		PS_Ginfo.assign(PS_Ginfo_buff.begin(), PS_Ginfo_buff.end());
	}
#ifdef TAI_GPU
	cuda::GpuMat d_img(img);
	d_img.copyTo(out_mat);
#endif
#ifdef TAI_CPU
	img.copyTo(out_mat);
#endif	
	return 0;
}
/*
 * res
 */
static int PS_Fun45(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun46(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun47(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun48(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun49(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}
/*
 * res
 */
static int PS_Fun50(InputArray in_mat, OutputArray out_mat, vector<int> parameter_values )
{
#ifdef TAI_GPU
	in_mat.getGpuMat().copyTo(out_mat);
#endif
#ifdef TAI_CPU
	in_mat.getMat().copyTo(out_mat);
#endif
	return 0;
}


