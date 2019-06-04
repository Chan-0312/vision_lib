#include "VisionArithmetic.hpp"

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
2018.10.10: 	加入IsPointInMatrix判断一个点是否在矩形内部函数。
*/


// 过滤黑色HVS
Hsv HSV_delete_black = {0,360,0,255,200,255};
// 过滤白色HVS
Hsv HSV_delete_white = {0,360,145,255,0,255};

// 混合高斯背景建模变量
Ptr<BackgroundSubtractorMOG2> mog = createBackgroundSubtractorMOG2(); 


/*
*函数名：	getDotAtLine
*功能：		判断点C在直线矢量AB方向那一侧
*函数参数：	Point A,Point B,Point C
*返回值：		S		
*			S>0 则C在矢量AB的左侧
*			S<0 则C在矢量AB的右侧
*			S=0 则C在直线AB上
*/
double getDotAtLine(Point2f A,Point2f B,Point2f C)
{
	double S=0;
	S = (A.x-C.x)*(B.y-C.y) - (A.y-C.y)*(B.x-C.x);
	return S;
}

/*
*函数名：    getDistance  
*功能：      获取两点之间的距离   
*函数参数：  
*Point2f pointO  - 起点  
*Point2f pointA  - 终点  
*函数返回值：  
*double        两点之间的距离  
*/    
double getDistance (Point2f pointO,Point2f pointA )    
{    
    double distance; 
    distance = powf((pointO.x - pointA.x),2) + powf((pointO.y - pointA.y),2); 
	return sqrtf(distance);    
}


/*
*函数名：    getTriangleIncentre  
*功能：     获取三角形内切圆心   
*函数参数：  
Point2f A,B,C	- 三角形三点坐标  
*函数返回值：  
Point2f        	- 中心坐标  
*/ 
Point2f getTriangleIncentre(Point2f A,Point2f B,Point2f C)
{
	double AB,BC,CA;
	Point2f X;

	AB = getDistance(A,B);
	BC = getDistance(B,C);
	CA = getDistance(C,A);
	X.x = (BC*A.x+CA*B.x+AB*C.x)/(AB+BC+CA);
	X.y = (BC*A.y+CA*B.y+AB*C.y)/(AB+BC+CA);
	return X;
}


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
Point2d getCoordinateRotation(Point2d A,Point2d O,double angle)
{
	Point2d X;

	X.x = ( A.x - O.x)*cos(angle) - (A.y - O.y)*sin(angle) + O.x;
	X.y = (A.x - O.x)*sin(angle) + (A.y - O.y)*cos(angle) + O.y;

	return X;
}


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
int graylevel(Mat src, Point cen, int r)
{
	int graysum = 0, n = 0;
	for(int i = (cen.y - r); i <= (cen.y + r); ++i)//访问矩形框内的像素值
	{
		uchar* data = src.ptr<uchar>(i);

		for(int j = (cen.x - r); j <= (cen.x + r); ++j)
		{
			double d = (i-cen.y)*(i-cen.y) + (j-cen.x)*(j-cen.x);
			if(d < r*r)
			{
				++n;
				graysum += (int)data[j];
			}
		}
	}
	
	return (graysum/n);
}



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
vector<float> TemplateMatch(const Mat& src, const Mat& obj, vector<Rect>& rect, int type, int num)
{
    Mat dst;
    matchTemplate(src, obj, dst, type);
    normalize(dst, dst, 0, 255, NORM_MINMAX);

    vector<float> pro;  rect.clear();
    double mx = 0;  Point loc, pre(-1, -1);
    for (int i = 0; i < num;)
    {
        minMaxLoc(dst, NULL, &mx, NULL, &loc);
        if (abs(loc.x - pre.x) > 10 && abs(loc.y - pre.y) > 10)
        {
            rect.push_back(Rect(loc.x, loc.y, obj.cols, obj.rows));
            pro.push_back(mx/255);  pre = loc;  ++i;
        }
        dst.at<float>(loc.y, loc.x) = 0;    mx = 0;
    }
    return pro;
}

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
Point2d RotateTemplateMatch(const Mat& src,const Mat& obj,double angle)
{
	//指定旋转中心 
	Size src_sz = obj.size();  
	Point2f center(obj.cols/ 2., obj.rows / 2.);

	//获取旋转矩阵（2x3矩阵）  
	Mat rot_mat = getRotationMatrix2D(center, angle, 1.0); 
	
	// 改变图形尺寸
	Rect bbox = RotatedRect(center, src_sz, angle).boundingRect();
	rot_mat.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rot_mat.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	//根据旋转矩阵进行仿射变换     
	Mat templ;       
	// 指定颜色填充
	//warpAffine(obj, templ, rot_mat, bbox.size(),INTER_LINEAR,BORDER_CONSTANT,Scalar(255,255,255));
	// 按边界颜色填充
	warpAffine(obj, templ, rot_mat, bbox.size(),INTER_LINEAR, BORDER_REPLICATE);

	imshow("Template", templ);  
	
	Mat dst=src.clone();
	vector<Rect> rect;  
	vector<float> pro;
	pro = TemplateMatch(src, templ, rect, match_method, 1);	
	Point2d boxxy;
	for (int i = 0; i < 1; ++i)
    {
		boxxy.x = rect[i].x + rect[i].width/2.0; 
		boxxy.y = rect[i].y + rect[i].height/2.0;
       	rectangle(dst, rect[i], Scalar(0,255,0), 2);       	
       	circle(dst, boxxy, 4, Scalar(0, 255, 0), -1);
		//cout<<"Template-->"<<"x: "<<boxxy.x<<" y: "<<boxxy.y<<" pro: "<<pro[i]<<endl;
    }
	imshow("MatchFigure", dst);
	return boxxy;
	

}




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
void ContrastControl(const Mat& src, Mat& obj,double alpha, double beta)
{
	Mat img = Mat::zeros( src.size(), src.type() );
    for( int y = 0; y < src.rows; y++ )
    {
        for( int x = 0; x < src.cols; x++ )
        {
            for( int c = 0; c < 3; c++ )
            {
            	// 执行运算 img(i,j) = alpha*src(i,j) + beta
                img.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( src.at<Vec3b>(y,x)[c] + beta ) );
            }
        }
    }	
	obj = img;
}


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
void ColorFilter(const Mat& src, Mat& obj, Hsv* colour_hsv, int mod)
{
	// 灰度值归一化
	Mat bgrimg;
	// HSV图像
	Mat hsvimg;
	// 掩码图像
	Mat mask;

	// 彩色图像的灰度值归一化
	src.convertTo(bgrimg, CV_32FC3, 1.0 / 255, 0);
	// 颜色空间转换
	cvtColor(bgrimg, hsvimg, COLOR_BGR2HSV);
	// 颜色分割
	inRange(hsvimg, Scalar(colour_hsv->hmin, colour_hsv->smin / 255.0, colour_hsv->vmin / 255.0), Scalar(colour_hsv->hmax, colour_hsv->smax / 255.0, colour_hsv->vmax / 255.0), mask);
	if (mod > 0)	// 原色彩
	{
		Mat dst;
		dst = Mat::zeros(src.size(),CV_32FC3);
		for (int r = 0; r < bgrimg.rows; r++)
		{
			for (int c = 0; c < bgrimg.cols; c++)
			{
				if (mask.at<uchar>(r, c) == 255)
				{
					dst.at<Vec3f>(r, c) = bgrimg.at<Vec3f>(r, c);
				}
			}
		}
		obj = dst;
	}
	else		// 黑白色彩
		obj = mask;
}

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
vector< vector<Point> > ImageProcessing(const Mat& src, Mat& obj, int debug)
{
	Mat midImg1,midImg2,midImg3,midImg4,midImg5;

	//开运算(MORPH_OPEN)
	// 用于消除小物体，在纤细点处分离物体，并且在平滑较大物体的边界的同时不明显改变其面积，同时抑制比结构元小的亮细节。 
	midImg1=src.clone();
	Mat element=getStructuringElement(MORPH_RECT,Size(5,5));
	morphologyEx(src,midImg1,MORPH_OPEN,element);

	//闭运算（MORPH_CLOSE）
	//用来填充物体内细小空洞、连接邻近物体、平滑其边界的同时并不明显改变其面积，同时抑制比结构元小的暗细节。 
    morphologyEx(midImg1, midImg2,MORPH_CLOSE,element);
	
	// 均值滤波
	blur(midImg2,midImg3,Size(40,40),Point(-1,-1));
	
	// 二值化
	midImg4=midImg3.clone();
	int rowNumber=midImg4.rows;
	int colNumber=midImg4.cols;
	for(int i=0;i<rowNumber;i++)
	{
		uchar* data=midImg4.ptr<uchar>(i);  
		for(int j=0;j<colNumber;j++)
		{		
			if(data[j]>120)
				data[j]=255;
			else
				data[j]=0;
		}
	}
	
	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;	
	midImg5=Mat::zeros(midImg4.rows,midImg4.cols,CV_8UC3);	
	findContours(midImg4,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);	
	for(int index=0;index< contours.size();index++)
	{		
		Scalar color(255,255,255);
		drawContours(midImg5,contours,index,color,CV_FILLED,8,hierarchy);
	}
	
	if (debug > 0)
	{
		imshow("OpenOperation",midImg1);
		imshow("CloseOperation",midImg2);
		imshow("MeanFiltering",midImg3);
		imshow("Binaryzation",midImg4);
		imshow("SkeletonMap",midImg5);
	}

	return contours;
}





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
vector<GraphicInfo> TakeContours(const Mat& src, Mat& obj, vector< vector<Point> > contours, char Colour_str)
{
	vector<GraphicInfo> GG_Info;
	GraphicInfo G_Info;
	//G_Info = (GraphicInfo*)malloc(sizeof(GraphicInfo));
	
	double angle;
	Mat xianshi=src.clone();
	// 对每个轮廓的点集 找逼近多边形
	vector< vector<Point> > approxPoint(contours.size());  
    for (int i = 0; i < (int)contours.size(); i++)  
    {  
        approxPolyDP(contours[i], approxPoint[i], 3, true);  
    } 
	//

	for(int i=0;i<approxPoint.size();i++)
	{
		
		//每个轮廓
		vector<Point> points=approxPoint[i];
		//对给定的2D点集，寻找最小面积的包围矩形
		RotatedRect box=minAreaRect(Mat(points));
		Point2f vertex[4];
		box.points(vertex);
	
		/*
        //  找最小包围圆
        Point2f center;
        float radius = 0;
        minEnclosingCircle(points, center, radius);
		*/
		
		// 过滤比较小的边框	
		// 矩阵长宽
		if ( box.size.width<min_box )
			continue ;
		if ( box.size.height<min_box )
			continue ;

		//cout<<box.angle<<endl;
		
		// 矩形高宽比
		double ratioWL = box.size.height/box.size.width;

		// 中心坐标
		// box.center.x box.center.y
	
		// 求解最长的边
		// 找到轮廓点最远的两个点
		double max_length = 0;
		double length = 0;
		Point max_length_num;
		for (int j=0;j<points.size();j++)
		{
			for(int k=j+1;k<points.size();k++)
			{
				if (k>=points.size())
					break;
				length = getDistance(points[j],points[k]);
				if (length > max_length)
				{
					max_length = length;
					max_length_num.x = j;
					max_length_num.y = k;
				}
			}
		}
		CvScalar colour_show = Scalar(0,0,0);
		// 判断点在直线的左右那一侧
		// 采用几何法	 
		double S = getDotAtLine(points[max_length_num.x],points[max_length_num.y],box.center);

		// 图形角度
		angle=atan2(points[max_length_num.y].y-points[max_length_num.x].y,points[max_length_num.y].x-points[max_length_num.x].x)*180/3.1415926;
		if (abs(S) > MinDotDistance)
		{
			if (S >= 0)
			if 	(angle > 0)
				angle = -180 + angle;
			else
				angle = 180 + angle;
			// 三角形
			putText(xianshi,"Triangle",box.center,FONT_HERSHEY_COMPLEX,1,Scalar(0,0,255),1,8);

			G_Info.Graphic_name = 'T';
		}
		else
		{
			// 寻找矩阵中的起始点
			double min_length = 5000;
			int min_nou = 0;		
			for (int j = 0; j < 4; j++)
			{
				length =getDistance(vertex[j],points[max_length_num.y]);
				if (length < min_length)
				{
					min_length = length;
					min_nou = j;
				}
			}
			// 寻找起始点长的点
			int a,b;
			a = (min_nou==3) ? 0 : (min_nou+1);
			b = (min_nou==0) ? 3 : (min_nou-1);
			if (getDistance(vertex[a],vertex[min_nou]) < getDistance(vertex[b],vertex[min_nou]))
				a = b;
			
			angle = atan2(vertex[min_nou].y-vertex[a].y,vertex[min_nou].x-vertex[a].x)*180/3.1415926;
			if ((ratioWL>=0.85) && (ratioWL<1.18))	// 正方形
			{
				//putText(xianshi,"Square",box.center,FONT_HERSHEY_COMPLEX,1,Scalar(0,0,255),1,8);
				G_Info.Graphic_name = 'S';
				
				// 因为图形是正方形，采用算法减少机器手臂的旋转角度
				if 	(angle > 90)
					angle = angle-90;
				else if (angle < -90)
					angle = angle+90;

				if (abs(angle) > 45)
					if (angle > 0)
						angle = angle-90;
					else
						angle = angle+90;
				
				colour_show = Scalar(0,0,255);
				
			}
			else if (((ratioWL>=0.37) && (ratioWL<0.47)) || ((ratioWL>=2.1) && (ratioWL<2.7)))	// 平行四边形
			{
				//putText(xianshi,"Rhomboid",box.center,CV_FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,255),1,8);
				G_Info.Graphic_name = 'R';
				colour_show = Scalar(0,255,0);
			}
			else 
			{
				G_Info.Graphic_name = '#';		//其他图形
			}

		}		

		// 输出信息
		//cout<<Colour_str<<"-->"<<"x: "<<box.center.x<<" y: "<<box.center.y<<" angle: "<<angle<<" ratioWL: "<<ratioWL<<endl;

		G_Info.Graphic_colour = Colour_str;	// 颜色
		G_Info.Graphic_Point = box.center;		// 中心坐标
		G_Info.Graphic_angle = angle;			// 旋转角度
		GG_Info.push_back(G_Info);	

		
		
		//绘制出最小面积的包围矩形
		line(xianshi,vertex[0],vertex[1],colour_show,2,LINE_AA);
		line(xianshi,vertex[1],vertex[2],colour_show,2,LINE_AA);
		line(xianshi,vertex[2],vertex[3],colour_show,2,LINE_AA);
		line(xianshi,vertex[3],vertex[0],colour_show,2,LINE_AA);

		//绘制中心的光标
		Point l,r,u,d;
		l.x=box.center.x-10;
		l.y=box.center.y;
		r.x=box.center.x+10;
		r.y=box.center.y;
		u.x=box.center.x;
		u.y=box.center.y-10;
		d.x=box.center.x;
		d.y=box.center.y+10;	
		line(xianshi,l,r,colour_show,2,LINE_AA);
		line(xianshi,u,d,colour_show,2,LINE_AA);

	}
	obj = xianshi;
	return GG_Info;
}


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
vector<GraphicInfo> TakeTriangleContours(const Mat& src, Mat& obj, vector< vector<Point> > contours, char Colour_str)
{
	vector<GraphicInfo> GG_Info;
	GraphicInfo G_Info;
	
	double angle;
	Mat xianshi=src.clone();

	// 对每个轮廓的点集 找逼近多边形
	vector< vector<Point> > approxPoint(contours.size());  
    for (int i = 0; i < (int)contours.size(); i++)  
    {  
        approxPolyDP(contours[i], approxPoint[i], 3, true);  
    }
	for(int i=0;i<approxPoint.size();i++)
	{
		// 每个轮廓
		vector<Point> points=approxPoint[i];

        //  找最小封闭三角形
        vector<Point2f> triangle;
        minEnclosingTriangle(points, triangle);
		if (triangle.size() != 3)
			continue ;
		// 求解最长最短的边
		double length1,length2,length3,length_max;
		int max_num[2];
		length1 = getDistance(triangle[0],triangle[1]);
		length2 = getDistance(triangle[0],triangle[2]);
		length3 = getDistance(triangle[1],triangle[2]);
		if (length1 > length2)
		{
			if (length1 > length3)
			{
				length_max = length1;
				max_num[0]=0;max_num[1]=1;
			}
			else
			{
				length_max = length3;
				max_num[0]=1;max_num[1]=2;
			}
		}
		else
		{
			if (length2 > length3)
			{
				length_max = length2;
				max_num[0]=0;max_num[1]=2;
			}
			else
			{
				length_max = length3;
				max_num[0]=1;max_num[1]=2;
			}
		}
		// 过滤比较小的边框	
		if ( length_max < min_box )
			continue ;

		// 求解内切圆心
		Point2f incentre;
		incentre = getTriangleIncentre(triangle[0],triangle[1],triangle[2]);


		// 图形角度
		double S ;
		if (triangle[max_num[1]].y > triangle[max_num[0]].y)
		{
			angle=atan2(triangle[max_num[1]].y-triangle[max_num[0]].y,triangle[max_num[1]].x-triangle[max_num[0]].x)*180/3.1415926;
			S = getDotAtLine(triangle[max_num[1]],triangle[max_num[0]],incentre);
		}
		else 
		{
			angle=atan2(triangle[max_num[0]].y-triangle[max_num[1]].y,triangle[max_num[0]].x-triangle[max_num[1]].x)*180/3.1415926;
			S = getDotAtLine(triangle[max_num[0]],triangle[max_num[1]],incentre);
		}
		
		if (S <  0)
			angle = -180 + angle;
		
		//cout<<Colour_str<<"-->"<<"x: "<<incentre.x<<" y: "<<incentre.y<<" angle: "<<angle<<endl;
		
		G_Info.Graphic_name = 'T';				// 图型
		G_Info.Graphic_colour = Colour_str;		// 颜色
		G_Info.Graphic_Point = incentre;		// 中心坐标
		G_Info.Graphic_angle = angle;			// 旋转角度

		GG_Info.push_back(G_Info);	
		
		line(xianshi,triangle[0],triangle[1],Scalar(255,0,0),2,LINE_AA);
		line(xianshi,triangle[0],triangle[2],Scalar(255,0,0),2,LINE_AA);
		line(xianshi,triangle[2],triangle[1],Scalar(255,0,0),2,LINE_AA);
		
		//绘制中心的光标
		Point l,r,u,d;
		l.x=incentre.x-10;
		l.y=incentre.y;
		r.x=incentre.x+10;
		r.y=incentre.y;
		u.x=incentre.x;
		u.y=incentre.y-10;
		d.x=incentre.x;
		d.y=incentre.y+10;	
		line(xianshi,l,r,Scalar(255,0,0),2,LINE_AA);
		line(xianshi,u,d,Scalar(255,0,0),2,LINE_AA);
	}
	obj = xianshi;
	return GG_Info;
}


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
vector<GraphicInfo> TakeTriangleRectangle(const Mat& src, Mat& obj, vector< vector<Point> > contours,RectLimitInfo R_Info)
{
	vector<GraphicInfo> GG_Info;
	GraphicInfo G_Info;
	// 对每个轮廓的点集 找逼近多边形
	vector< vector<Point> > approxPoint(contours.size());  
	for (int i = 0; i < (int)contours.size(); i++)	
	{  
		approxPolyDP(contours[i], approxPoint[i], 3, true);  
	} 
	
	Mat drawing = src.clone();
	for(int i=0;i<approxPoint.size();i++)
	{
	
		vector<Point> points=contours[i];
		//对给定的2D点集，寻找最小面积的包围矩形
		RotatedRect box=minAreaRect(Mat(points));
		Point2f vertex[4];
		box.points(vertex);

		/*** 默认高比宽大 ***/
		// 重新定义宽高、高度
		double buf_w,buf_h,angle;
		if (box.size.height > box.size.width)
		{
			buf_w = box.size.width;
			buf_h = box.size.height;
			angle = 90+box.angle;
		}
		else
		{
			buf_w = box.size.height;
			buf_h = box.size.width;
			angle = box.angle;
		}

		
		// 过滤比较小的边框 
		if ( (buf_w<R_Info.min_w) || (buf_w>R_Info.max_w))
			continue ;
		
		// 矩形宽高比
		double ratioWL = buf_w/buf_h;

		if ((R_Info.min_WL > 0) && (R_Info.max_WL > 0))
			if (!((ratioWL>=R_Info.min_WL) && (ratioWL<=R_Info.max_WL)))
				continue ;
		
		CvScalar colour_show = Scalar(156,177,242);
		//绘制出最小面积的包围矩形
		line(drawing,vertex[0],vertex[1],colour_show,2,LINE_AA);
		line(drawing,vertex[1],vertex[2],colour_show,2,LINE_AA);
		line(drawing,vertex[2],vertex[3],colour_show,2,LINE_AA);
		line(drawing,vertex[3],vertex[0],colour_show,2,LINE_AA);
	
		//绘制中心的光标
		Point l,r,u,d;
		l.x=box.center.x-10;
		l.y=box.center.y;
		r.x=box.center.x+10;
		r.y=box.center.y;
		u.x=box.center.x;
		u.y=box.center.y-10;
		d.x=box.center.x;
		d.y=box.center.y+10;	
		line(drawing,l,r,colour_show,2,LINE_AA);
		line(drawing,u,d,colour_show,2,LINE_AA);
	
	
		G_Info.Graphic_colour = '#';			// 颜色
		G_Info.Graphic_Point = box.center;		// 中心坐标
		G_Info.Graphic_angle = angle;			// 旋转角度
		G_Info.Graphic_size.width = buf_w;
		G_Info.Graphic_size.height = buf_h;
		G_Info.Graphic_rRect = box;
		GG_Info.push_back(G_Info);	
	
		//cout<<"-->"<<"x:"<<box.center.x<<" y:"<<box.center.y<<" angle:"<<angle<<" box:"<<box.size<<" ratioWL:"<<ratioWL<<endl;
		//imshow( "【识别】", drawing );
	}
	obj = drawing;
	return GG_Info;
}




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
void GrabCutFiltration(const Mat& src, Mat& obj ,Rect selection, int mod)
{
	Mat result; // 4种可能结果
	Mat bgModel, fgModel; // 背景/前景 

	grabCut(src, result, selection, bgModel, fgModel, 1, GC_INIT_WITH_RECT);
	compare(result,GC_PR_FGD,result,CMP_EQ);//得到前景mask
	imshow("GrabCut",result);
	if (mod != 0)
	{	
		Mat foreground(src.size(),CV_8UC3,Scalar::all(0));
		src.copyTo(foreground,result);
		obj = foreground;
		//imshow("foreground",foreground);
	}
	else
	{	
		Mat midImg;
		Mat element=getStructuringElement(MORPH_RECT,Size(20,20));
		// 开运算
		morphologyEx(result,midImg,MORPH_OPEN,element);

		// 闭运算
		//morphologyEx(result, midImg,MORPH_CLOSE,element);
		
		obj = midImg;
		imshow("foreground",midImg);	
	}
}


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
GraphicInfo GetLabelInfo(const Mat& src, Mat& obj, Rect selection, RectLimitInfo R_Info)
{
	// GrabCut过滤
	Mat GrabCut;
	GrabCutFiltration(src, GrabCut, selection,0); 

	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;	
	findContours(GrabCut,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);

	// 读取标签信息
	vector<GraphicInfo> GG_Info;
	GraphicInfo G_Info;
	Mat out_img;
	GG_Info = TakeTriangleRectangle(src, out_img, contours,R_Info);
	if (GG_Info.size() > 0)
		G_Info = GG_Info[0];
	
	obj = out_img;
	return G_Info;
}


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
vector<GraphicInfo> UPC_Mark(const Mat& src, Mat& obj,int thresh,RectLimitInfo R_Info,int flag_angle)
{
	//转化为灰度图
	Mat image1;
	cvtColor(src,image1,CV_RGB2GRAY);

	//高斯平滑滤波
	//Mat image2;
	//GaussianBlur(image1,image2,Size(3,3),0);
	
	Mat image2;
	image2 = image1;

	//求得水平和垂直方向灰度图像的梯度差,使用Sobel算子
	Mat image3;
	Mat imageX16S,imageY16S,imageSobelX,imageSobelY;
	Sobel(image2,imageX16S,CV_16S,1,0,3,1,0,4);
	Sobel(image2,imageY16S,CV_16S,0,1,3,1,0,4);
	convertScaleAbs(imageX16S,imageSobelX,1,0);
	convertScaleAbs(imageY16S,imageSobelY,1,0);
	//image3=imageSobelX-imageSobelY;
	addWeighted(imageSobelX, 0.5, imageSobelY, 0.5, 0, image3);
	//imshow("Sobel",image3);
	
	// 均值滤波，消除高频噪声
	//blur(image3,image3,Size(5,5));


	//二值化
	Mat image4;
	threshold(image3,image4,thresh,255,CV_THRESH_BINARY);	
	//imshow("Binaryzation",image4);
	
	// 开运算
	Mat  element;
	element=getStructuringElement(MORPH_ELLIPSE,Size(5,5));
	morphologyEx(image4,image4,MORPH_CLOSE,element);
	imshow("OpenOperation",image4);
	
	/*
	//闭运算，填充条形码间隙
	element=getStructuringElement(0,Size(5,5));
	morphologyEx(image4,image4,MORPH_CLOSE,element);	
	imshow("CloseOperation",image4);

	//腐蚀，去除孤立的点
	element=getStructuringElement(0,Size(5,5));
	erode(image4,image4,element);
	imshow("Etch",image4);
	
	//膨胀，填充条形码间空隙，根据核的大小，有可能需要2~3次膨胀操作
	element=getStructuringElement(0,Size(3,3));
	dilate(image4,image4,element);
	imshow("Swell",image4);
	*/
	
	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;	
	findContours(image4,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);	

	Mat out;
	out = src.clone();
	
	vector<GraphicInfo> GG_Info;
	GraphicInfo G_Info;
	
	//寻找最小面积的包围矩形
	for(int i=0;i<contours.size();i++)
	{
		vector<Point> points=contours[i];
		//对给定的2D点集，寻找最小面积的包围矩形
		RotatedRect box=minAreaRect(Mat(points));
		Point2f vertex[4];
		box.points(vertex);
		
		/*** 默认高比宽大 ***/
		// 重新定义宽高、高度
		double buf_w,buf_h,angle;
		if (box.size.height > box.size.width)
		{
			buf_w = box.size.width;
			buf_h = box.size.height;
			angle = 90+box.angle;
		}
		else
		{
			buf_w = box.size.height;
			buf_h = box.size.width;
			angle = box.angle;
		}

		// 过滤比较小的边框 
		if ( (buf_w<R_Info.min_w) || (buf_w>R_Info.max_w))
			continue ;
		
		// 矩形宽高比
		double ratioWL = buf_w/buf_h;

		if ((R_Info.min_WL > 0) && (R_Info.max_WL > 0))
			if (!((ratioWL>=R_Info.min_WL) && (ratioWL<=R_Info.max_WL)))
				continue ;
			
		//通过findContours找到条形码区域的矩形边界
		CvScalar colour_show = Scalar(156,177,242);
		//绘制出最小面积的包围矩形
		line(out,vertex[0],vertex[1],colour_show,2,LINE_AA);
		line(out,vertex[1],vertex[2],colour_show,2,LINE_AA);
		line(out,vertex[2],vertex[3],colour_show,2,LINE_AA);
		line(out,vertex[3],vertex[0],colour_show,2,LINE_AA);

		// 设置几等分点
		int fen = 30;
		// 短边中点
		Point2f Short_edge_point1,Short_edge_point2;
		if (getDistance(vertex[0],vertex[1]) > getDistance(vertex[1],vertex[2]))
		{
			//line(out,vertex[1],vertex[2],Scalar(0, 0, 255),2,LINE_AA);
			//line(out,vertex[3],vertex[0],Scalar(0, 0, 255),2,LINE_AA);
			Short_edge_point1 = (vertex[1]+vertex[2])/2;
			Short_edge_point2 = (vertex[3]+vertex[0])/2;
		}
		else
		{
			//line(out,vertex[0],vertex[1],Scalar(0, 0, 255),2,LINE_AA);
			//line(out,vertex[2],vertex[3],Scalar(0, 0, 255),2,LINE_AA);
			Short_edge_point1 = (vertex[0]+vertex[1])/2;
			Short_edge_point2 = (vertex[2]+vertex[3])/2;	
		}
					
		// 计算fen等分点半径为5的平均灰度值
		int graylevel1,graylevel2;
		Point2f equal_diversion_point1,equal_diversion_point2;
		equal_diversion_point1 = (Short_edge_point1+(fen-1)*Short_edge_point2)/fen;
		equal_diversion_point2 = (Short_edge_point2+(fen-1)*Short_edge_point1)/fen;
		graylevel1 = graylevel(image4,equal_diversion_point1,5);
		graylevel2 = graylevel(image4,equal_diversion_point2,5);

		if ( flag_angle != 0)
		{
			if (graylevel1 < graylevel2)
			{
				circle(out,equal_diversion_point1, 5, Scalar(0, 255, 0),-1);
				if (equal_diversion_point1.x > equal_diversion_point2.x)
					if (ratioWL > 1)
						angle = -180 + angle;
					else
						angle = 180 + angle;
			}
			else
			{
				circle(out,equal_diversion_point2, 5, Scalar(0, 255, 0),-1);
				if (equal_diversion_point2.x > equal_diversion_point1.x)
					if (ratioWL > 1)
						angle = -180 + angle;
					else
						angle = 180 + angle;
			}
		}
		
			
		G_Info.Graphic_colour = '#';			// 颜色
		G_Info.Graphic_Point = box.center;		// 中心坐标
		G_Info.Graphic_angle = angle;			// 旋转角度
		G_Info.Graphic_size.width = buf_w;
		G_Info.Graphic_size.height = buf_h;
		G_Info.Graphic_rRect = box;
		GG_Info.push_back(G_Info);	
		//cout<<"-->"<<"x:"<<box.center.x<<" y:"<<box.center.y<<" angle:"<<angle<<" box:"<<box.size<<" ratioWL:"<<ratioWL<<endl;

	}
	obj = out;
	return GG_Info;
}


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
vector<GraphicInfo> MobileIdentification(const Mat& src, Mat& obj, Mat& foreground, double thresh, RectLimitInfo R_Info)
{
	vector<GraphicInfo> GG_Info;

	// 背景建模
	Mat image1;
	mog->apply(src, image1, thresh);
	//imshow("Picture",image1);
	
	Mat  element,image2;
	element=getStructuringElement(MORPH_ELLIPSE,Size(src.cols*0.02,src.rows*0.02));
	morphologyEx(image1,image2,MORPH_CLOSE,element);
	//imshow("OpenOperation",image2);

	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;	
	findContours(image1,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);	

	Mat out;
	GG_Info = TakeTriangleRectangle(src, out, contours,R_Info);

	obj = out;	
	foreground = image2;
	return GG_Info;
}


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
vector< vector<Point> >  EdgeDetection (const Mat& src, Mat& obj ,double thresh, int mode)
{
	Mat src_gray;
	// 灰度化
	cvtColor( src, src_gray, CV_BGR2GRAY );  
	// 高斯滤波
    GaussianBlur(src_gray, src_gray, Size(3,3), 0.1, 0, BORDER_DEFAULT );    

	// Canny检测图像       
    Mat canny_output;     
    //canny检测边缘       
    Canny( src_gray, canny_output, thresh, thresh*3, 3 );     
    
	// 轮廓化
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;	
	findContours(canny_output,contours,hierarchy,mode,CHAIN_APPROX_SIMPLE);	
	
	obj = canny_output;
	return contours;
}


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
vector<GraphicInfo> HoffCircleDetection (const Mat& src, Mat& obj , double thresh, RectLimitInfo R_Info)
{
	Mat src_gray;
	
	medianBlur(src, src, 3);		//中值滤波，去除噪声影响，不去噪的话，霍夫圆检测的结果会不准确
	
	cvtColor( src, src_gray, CV_BGR2GRAY );

	GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );

	vector<Vec3f> circles;

	HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, thresh, R_Info.min_WL, R_Info.min_w, R_Info.max_w );

	vector<GraphicInfo> GG_Info;
	for( size_t i = 0; i < circles.size(); i++ )
	{
		GraphicInfo G_Info;
	  	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));	  
	  	int radius = cvRound(circles[i][2]);
	  	// circle center
	  	circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
	  	// circle outline
	  	circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
		G_Info.Graphic_Point = center;
		G_Info.Graphic_radius = radius;
		GG_Info.push_back(G_Info);
	}
	obj =src;
	return GG_Info;
}

/*
*函数名：    Rid_Data_Exception  
*功能：     拉依达准则剔除数据异常  
*函数参数：  
vector<double> data						- 原数据
double thresh							- 阈值
*返回值：	
vector<double>							- 剔除后数据
*/
vector<double> Rid_Data_Exception(vector<double> data, double thresh)
{
	vector<double> out_data;
	// 平均值
	double mean_data = 0;
	for(int i = 0; i < data.size(); i++)
	{
		mean_data +=  data[i];
	}
	mean_data = mean_data/data.size();

	// 标准差
	double std_data = 0;
	for(int i = 0; i < data.size(); i++)
	{
		std_data +=  pow(data[i]-mean_data,2);
	}
	std_data = sqrt(std_data/(data.size()-1));

	for(int i = 0; i < data.size(); i++)
	{
		if ( fabs(data[i] - mean_data) <= thresh* std_data)
		{
			out_data.push_back(data[i]);
		}
	}

	return out_data;

}

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
bool IsPointInMatrix(Point2d centre, Size  Graphic_size, Point2d p)
{	
	Point2d p1,p2,p3,p4;
	p1.x = centre.x - Graphic_size.width/2;
	p1.y = centre.y - Graphic_size.height/2;
	
	p2.x = centre.x - Graphic_size.width/2;
	p2.y = centre.y + Graphic_size.height/2;

	p3.x = centre.x + Graphic_size.width/2;
	p3.y = centre.y + Graphic_size.height/2;
	
	p4.x = centre.x + Graphic_size.width/2;
	p4.y = centre.y - Graphic_size.height/2;

    int a = (p2.x - p1.x)*(p.y - p1.y) - (p2.y - p1.y)*(p.x - p1.x);
   	int b = (p3.x - p2.x)*(p.y - p2.y) - (p3.y - p2.y)*(p.x - p2.x);
    int c = (p4.x - p3.x)*(p.y - p3.y) - (p4.y - p3.y)*(p.x - p3.x);
    int d = (p1.x - p4.x)*(p.y - p4.y) - (p1.y - p4.y)*(p.x - p4.x);
	if((a > 0 && b > 0 && c > 0 && d > 0) || (a < 0 && b < 0 && c < 0 && d < 0))
	{
		return true;
    }
	return false;

}



