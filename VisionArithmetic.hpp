#ifndef VisionArithmetic_H
#define VisionArithmetic_H

/*
*函数名 ： 机器视觉处理函数
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/4/26
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
2018.7.10： 	加入URC_Mark条形码识别函数
2018.7.16：	加入MobileIdentification移动物体识别
2018.7.17：	加入RectLimitInformation矩形大小限制结构体，并修改了部分函数传递的参数。
2018.8.24:	加入HoffCircleDetection霍夫园型检测函数。
2018.9.13:	加入Rid_Data_Exception拉依达准则剔除数据异常函数
2018.10.10: 加入IsPointInMatrix判断一个点是否在矩形内部函数。
*/


#include <iostream>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <opencv2/video/background_segm.hpp>

using namespace cv;
using namespace std;


// 中心点最小距离偏差 辨认除三角形其他图形
#define MinDotDistance	1000
// 边缘检测最小边框大小
#define min_box  		30
// 模板匹配的算法
#define match_method 	5
/*
0. cv::TM_SQDIFF
1. cv::TM_SQDIFF_NORMED
2. cv::TM_CCORR
3. cv::TM_CCORR_NORMED
4. cv::TM_CCOEFF
5. cv::TM_CCOEFF_NORMED
*/

/*
* 图形信息结构体
*/
typedef struct  GraphicInformation{
	// 图形名称
	char Graphic_name;
	// 图形颜色	
	char Graphic_colour;
 	// 中心位置坐标
	Point2d Graphic_Point;
	// 旋转角度
	double Graphic_angle;
	// 图形尺寸
	Size  Graphic_size;
	// 旋转矩形
	RotatedRect Graphic_rRect;
	// 园型半径
	double Graphic_radius;
}GraphicInfo;

/*
* HSV结构体
*/
typedef struct HSV{
 	//色相
	int hmin;
	int hmax;
	//饱和度
	int smin;
	int smax;
	//亮度
	int vmin;
	int vmax;
}Hsv;

/*
* 矩形大小限制结构体
*/
typedef struct RectLimitInformation{
	// 最小宽高比
	double min_WL;
	// 最大宽高比
	double max_WL;
	/*** 默认高比宽大 ***/
	// 最小宽度
	double min_w;
	// 最大宽度
	double max_w;
	// 最小高度
	double min_h;
	// 最大高度
	double max_h;
}RectLimitInfo;

/*
*函数名：	getDotAtLine
*功能：		判断点C在直线矢量AB方向那一侧
*函数参数：	Point A,Point B,Point C
*返回值：		S		
*			S>0 则C在矢量AB的左侧
*			S<0 则C在矢量AB的右侧
*			S=0 则C在直线AB上
*/
double getDotAtLine(Point2f A,Point2f B,Point2f C);

/*
*函数名：    getDistance  
*功能：      获取两点之间的距离   
*函数参数：  
*Point2f pointO  - 起点  
*Point2f pointA  - 终点  
*函数返回值：  
*double        两点之间的距离  
*/ 
double getDistance (Point2f pointO,Point2f pointA );

/*
*函数名：    getTriangleIncentre  
*功能：     获取三角形内切圆心   
*函数参数：  
Point2f A,B,C	- 三角形三点坐标  
*函数返回值：  
Point2f        	- 中心坐标  
*/ 
Point2f getTriangleIncentre(Point2f A,Point2f B,Point2f C);

/*
*函数名：    getTriangleIncentre  
*功能：     获取坐标A绕O点旋转angle角度后坐标  
*函数参数：  
Point2d A		- 待转换坐标
Point2d O		- 中心坐标
double angle	- 旋转角度
*函数返回值：  
Point2d        	- 转换后坐标  
*/ 
Point2d getCoordinateRotation(Point2d A,Point2d O,double angle);

/*
*函数名：    graylevel  
*功能：     获取坐标圆的平均灰度值  
*函数参数：  
const Mat& src		- 待转换坐标
Point cen			- 中心坐标
int r				- 半径
*函数返回值：  
Point2d        	- 转换后坐标  
*/
int graylevel(Mat src, Point cen, int r);

/*
*函数名：    TemplateMatch  
*功能：      模板匹配   
*函数参数：  
const Mat& src  	- 原图像  
const Mat& obj  	- 模板图  
vector<Rect>& rect	- 返回位置
int type			- 检测方法
int num				- 检测数量
*函数返回值： 
vector<float> 		- 匹配度
*/  
vector<float> TemplateMatch(const Mat& src, const Mat& obj, vector<Rect>& rect, int type, int num);

/*
*函数名：    RotateTemplateMatch  
*功能：      带旋转模板的模板匹配   
*函数参数：  
const Mat& src  - 原图像  
const Mat& obj  - 模板图  
double angle	- 旋转角度
*函数返回值：  
Point2d			- 中心坐标
*/ 
Point2d RotateTemplateMatch(const Mat& src,const Mat& obj,double angle);

/*
*函数名：    ContrastControl  
*功能：      对比度曝光度调节   
*函数参数：  
const Mat& src  - 原图像  
Mat& obj  		- 处理过图像  
double alpha	- 对比度调节参数
double beta		- 曝光度调节参数
*函数返回值：  
void			- 无
*/
void ContrastControl(const Mat& src, Mat& obj,double alpha, double beta);

/*
*函数名：    ColorFilter  
*功能：      颜色过滤   
*函数参数：  
const Mat& src  - 原图像  
Mat& obj  		- 过滤后图像  
Hsv* colour_hsv - HVS范围结构体
				  	typedef struct HSV{
 				  		//色相
				  		int hmin;
				  		int hmax;
						//饱和度
						int smin;
						int smax;
						//亮度
						int vmin;
						int vmax;
					}Hsv;
int mod			- 1 为返回彩色图像
				- 0 为返回黑白图像
*函数返回值：  无
*/  
void ColorFilter(const Mat& src, Mat& obj, Hsv* colour_hsv, int mod);

/*
*函数名：    ImageProcessing  
*功能：      图像处理轮廓化
*函数参数：  
const Mat& src  - 原图像  
Mat& obj  		- 过滤后图像
int debug		- 1 输出中间图像 
				- 0 不输出中间图像
*返回值：		
vector< vector<Point> >		返回所有轮廓坐标
*/
vector< vector<Point> > ImageProcessing(const Mat& src, Mat& obj, int debug);

/*
*函数名：    TakeContours  
*功能：      边框绘制与图形识别定位  
*函数参数：  
const Mat& src							- 原图像
Mat& obj 								- 识别后图像  
vector< vector<Point> > contours  		- 轮廓坐标
char Colour_num							- 颜色编号 
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/ 
vector<GraphicInfo> TakeContours(const Mat& src, Mat& obj, vector< vector<Point> > contours, char Colour_str);

/*
*函数名：    TakeTriangleContours  
*功能：      三角形边框绘制与识别定位  
*函数参数：  
const Mat& src							- 原图像
Mat& obj 								- 识别后图像  
vector< vector<Point> > contours  		- 轮廓坐标
char Colour_num							- 颜色编号 
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/ 
vector<GraphicInfo> TakeTriangleContours(const Mat& src, Mat& obj, vector< vector<Point> > contours, char Colour_str);

/*
*函数名：    TakeTriangleRectangle  
*功能：      矩形边框绘制与识别定位  
*函数参数：  
const Mat& src							- 原图像
Mat& obj 								- 识别后图像  
vector< vector<Point> > contours  		- 轮廓坐标
RectLimitInfo R_Info					- 矩形大小限制结构体
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/
vector<GraphicInfo> TakeTriangleRectangle(const Mat& src, Mat& obj, vector< vector<Point> > contours, RectLimitInfo R_Info);

/*
*函数名：    GrabCutFiltration  
*功能：GrabCut过滤  
*函数参数：  
const Mat& src	- 原图像
Mat& obj 		- 过滤后图像  
Rect selection	- 过滤区域边框
int mod			- 0 返回前景黑白图像
				- 1 返回前景彩色图像
*返回值：	
void			- 空
*/
void GrabCutFiltration(const Mat& src, Mat& obj ,Rect selection, int mod);

/*
*函数名：    GetLabelInfo  
*功能：通过GrabCut算法获取标签信息
*函数参数：  
const Mat& src			- 原图像
Mat& obj 				- 识别后图像   
Rect selection			- 过滤区域边框
RectLimitInfo R_Info	- 矩形大小限制结构体
*返回值：	
GraphicInfo			- GraphicInfo结构体
*/
GraphicInfo GetLabelInfo(const Mat& src, Mat& obj, Rect selection, RectLimitInfo R_Info);

/*
*函数名：    UPC_Mark  
*功能：      矩形边框绘制与识别定位  
*函数参数：  
const Mat& src							- 原图像
Mat& obj 								- 识别后图像  
int thresh								- 图像二值化阈值
RectLimitInfo R_Info					- 矩形大小限制结构体
int flag_angle							- 识别范围		默认为0
										- 0 180度 	1 360度
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/ 
vector<GraphicInfo> UPC_Mark(const Mat& src, Mat& obj,int thresh,RectLimitInfo R_Info,int flag_angle = 0);

/*
*函数名：    MobileIdentification  
*功能：      移动物体识别  
*函数参数：  
const Mat& src							- 原图像
Mat& obj 								- 识别后图像  
double thresh							- 学习率
RectLimitInfo R_Info					- 矩形大小限制结构体
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/
vector<GraphicInfo> MobileIdentification(const Mat& src, Mat& obj, Mat& foreground, double thresh,RectLimitInfo R_Info);

/*
*函数名：    EdgeDetection  
*功能：      边缘检测  
*函数参数：  
const Mat& src,							- 原图像
Mat& obj  								- 识别后图像  
double thresh							- 阈值
int mode								- 轮廓检索模式
										RETR_EXTERNAL:表示只检测最外层轮廓，对所有轮廓设置hierarchy[i][2]=hierarchy[i][3]=-1 
										RETR_LIST:提取所有轮廓，并放置在list中，检测的轮廓不建立等级关系 
										RETR_CCOMP:提取所有轮廓，并将轮廓组织成双层结构(two-level hierarchy),顶层为连通域的外围边界，次层位内层边界 
										RETR_TREE:提取所有轮廓并重新建立网状轮廓结构 
										RETR_FLOODFILL：官网没有介绍，应该是洪水填充法

*返回值：	
vector< vector<Point> >					- 返回轮廓坐标
*/
vector< vector<Point> >  EdgeDetection (const Mat& src, Mat& obj ,double thresh, int mode = RETR_CCOMP);



/*
*函数名：    HoffCircleDetection  
*功能：     霍夫圆检测  
*函数参数：  
const Mat& src,							- 原图像
Mat& obj  								- 识别后图像  
double thresh							- 阈值
RectLimitInfo R_Info					- 矩形大小限制结构体
*返回值：	
vector<GraphicInfo>						- GraphicInfo结构体
*/
vector<GraphicInfo> HoffCircleDetection (const Mat& src, Mat& obj , double thresh, RectLimitInfo R_Info);

/*
*函数名：    Rid_Data_Exception  
*功能：     拉依达准则剔除数据异常  
*函数参数：  
vector<double> data						- 原数据
double thresh							- 阈值
*返回值：	
vector<double>							- 剔除后数据
*/
vector<double> Rid_Data_Exception(vector<double> data, double thresh);


/*
*函数名：	  IsPointInMatrix  
*功能： 	判断一个点是否在四边形内部  
*函数参数：  
Point2d centre					 - 四边形中心
Size  Graphic_size				 - 四边形尺寸
Point2d p						 - 点坐标
*返回值：	 
bool 							 - 真为点在四边形内
*/
bool IsPointInMatrix(Point2d centre, Size  Graphic_size, Point2d p);




#endif












