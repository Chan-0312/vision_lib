#include "URrobot.hpp"
#include <errno.h>

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


// 外部定义机器人xml标定配置文件
//const char *robotxml = "calib_outdata.xml";
extern char *robotxml;

// 机器人通信端口(PC作服务器)
const int robot_port = 3456;
	
// 外部定义服务器数据结构体
extern ServerInfo Data_ServerInfo;

// 外部定义客户端数据结构体
extern vector<ClientInfo> VData_ClientInfo;

// 外部定义机器人ip地址(UR作服务器)
extern char robot_ip[48];



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
static int ReadXml_mat(const      char *filename ,const char *datanama, Mat& data)
{
	FileStorage fs;
	
	if (!fs.open(filename,FileStorage::READ))
	{	
	    cout<<"Failed to open:"<<filename<<endl;
		return -1;
	}	
    fs[datanama]>>data;
	fs.release();
	if (data.rows != 0)
		return 0;
	else
		return 1;
}

/*
*函数名：	 	RobotData2CoordMatrix  
*功能：		机器人数据转旋转矩阵	 
*函数参数：  
vector<double> rdata		- 机器人输入数据（x,y,z,rx,ry,rz）
Mat& Tr						- 存放返回矩阵
*返回值：	
void						- 无
*/ 
void RobotData2CoordMatrix(vector<double> rdata, Mat& Tr)
{
	double r_vec[3]={rdata[3],rdata[4],rdata[5]};
	double R_matrix[3][3];
	double matrix[4][4];
	
	CvMat pr_vec;
	CvMat pR_matrix;
	
	cvInitMatHeader(&pr_vec,1,3,CV_64FC1,r_vec,CV_AUTOSTEP);
	cvInitMatHeader(&pR_matrix,3,3,CV_64FC1,R_matrix,CV_AUTOSTEP);
	cvRodrigues2(&pr_vec, &pR_matrix,0);
	
	for (int i = 0 ; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if ((i < 3) && (j < 3))
				Tr.at<double>(i,j) = R_matrix[i][j];
			else if ((i < 3) && (j == 3))
				Tr.at<double>(i,j) = rdata[i];
			else if ((i == 3) && (j == 3))
				Tr.at<double>(i,j) = 1;
			else
				Tr.at<double>(i,j) = 0;	
		}
	}

}


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
Point2d 	  Pixel2Robot(int mode, Point2d coord, double z, vector<double> rdata ,int xyz_mode)
{
	double u = coord.x;
	double v = coord.y;
	
	// 转换关系矩阵
	Mat M;
	// 读取Min、Tm、M矩阵
	Mat Min, Tm, kc;
	ReadXml_mat(robotxml ,"Min", Min);
	ReadXml_mat(robotxml ,"Tm", Tm);
	ReadXml_mat(robotxml ,"kc", kc);

	// 自动读取标定模式
	if (mode == -1)
	{
		Mat M_MOD;
		ReadXml_mat(robotxml,"M_MOD",M_MOD);
		if (M_MOD.at<double>(0,0) == 1)
		{
			mode = 2;
		}
		else
		{
			mode = 0;
		}
	}
	
	if (mode != 2)
	{
		/*
   		// 读取Min、Tm、M矩阵
	    Mat Min, Tm;
		ReadXml_mat(robotxml ,"Min", Min);
		ReadXml_mat(robotxml ,"Tm", Tm);
		*/
		Mat MN = Mat::zeros(3,4,CV_64FC1);
		for (int i = 0; i < MN.rows; i++)
		{
			for (int j = 0; j < MN.cols; j++)
			{
				if ( j < MN.cols-1)
					MN.at<double>(i,j) = Min.at<double>(i,j);
			}
		}
		if (mode == 0)
		{
			if (rdata.size() != 6)
				return Point2d(-1,-1);
			// 转换成关系矩阵形式
			Mat TR = Mat::zeros(4,4,CV_64FC1);
			RobotData2CoordMatrix(rdata,TR);
			M = MN*Tm.inv()*TR.inv();
		}
		else 
		{
			M = MN*Tm.inv();
		}
	}
	else
	{
		ReadXml_mat(robotxml ,"M", M);
	}

	Point2d out_point;
	out_point = Distor_Correc(coord,Min,kc);
	u = out_point.x;
	v = out_point.y;

	Mat P;
	if( xyz_mode == 1 )	//  求XZ,Y已知 
	{
		double Ml[2][2] = { {u*M.at<double>(2,0)-M.at<double>(0,0),u*M.at<double>(2,2)-M.at<double>(0,2)} , {v*M.at<double>(2,0)-M.at<double>(1,0),v*M.at<double>(2,2)-M.at<double>(1,2)} };

		double Mr[2][1] = { {M.at<double>(0,3)-u*M.at<double>(2,3)-(u*M.at<double>(2,1)-M.at<double>(0,1))*z} , {M.at<double>(1,3)-v*M.at<double>(2,3)-(v*M.at<double>(2,1)-M.at<double>(1,1))*z} };
		Mat ML(Size(2, 2), CV_64FC1, Ml);
		Mat MR(Size(1, 2), CV_64FC1, Mr);

		P = (ML.inv()*MR).t();
	}
	else if ( xyz_mode == 2 )	//  求YZ,X已知 
	{
		double Ml[2][2] = { {u*M.at<double>(2,1)-M.at<double>(0,1),u*M.at<double>(2,2)-M.at<double>(0,2)} , {v*M.at<double>(2,1)-M.at<double>(1,1),v*M.at<double>(2,2)-M.at<double>(1,2)} };
		double Mr[2][1] = { {M.at<double>(0,3)-u*M.at<double>(2,3)-(u*M.at<double>(2,0)-M.at<double>(0,0))*z} , {M.at<double>(1,3)-v*M.at<double>(2,3)-(v*M.at<double>(2,0)-M.at<double>(1,0))*z} };
		Mat ML(Size(2, 2), CV_64FC1, Ml);
		Mat MR(Size(1, 2), CV_64FC1, Mr);

		P = (ML.inv()*MR).t();
	}
	else					//  求XY,Z已知
	{
		double Ml[2][2] = { {u*M.at<double>(2,0)-M.at<double>(0,0),u*M.at<double>(2,1)-M.at<double>(0,1)} , {v*M.at<double>(2,0)-M.at<double>(1,0),v*M.at<double>(2,1)-M.at<double>(1,1)} };
		double Mr[2][1] = { {M.at<double>(0,3)-u*M.at<double>(2,3)-(u*M.at<double>(2,2)-M.at<double>(0,2))*z} , {M.at<double>(1,3)-v*M.at<double>(2,3)-(v*M.at<double>(2,2)-M.at<double>(1,2))*z} };
		Mat ML(Size(2, 2), CV_64FC1, Ml);
		Mat MR(Size(1, 2), CV_64FC1, Mr);

		P = (ML.inv()*MR).t();
	}
		
    // 坐标矫正	
	Point2d Robot;
	Mat XY_deviate;
	
	if(!ReadXml_mat(robotxml ,"XY_deviate", XY_deviate))
	{
		Robot.x = P.at<double>(0)+XY_deviate.at<double>(0);
		Robot.y = P.at<double>(1)+XY_deviate.at<double>(1);
	}
	else
	{
		Robot.x = P.at<double>(0);
		Robot.y = P.at<double>(1);
	}

	return Robot;
}


/*
*函数名：    strsplit  
*功能：  字符串分割   
*函数参数：  
const string &s, 		- string字符串
const string &seperator - 分割符
*返回值：	
vector<string>			- 分割后的字符串向量
*/  
static vector<string> strsplit(const string &s, const string &seperator)
{
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;
     
    while(i != s.size())
	{
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while(i != s.size() && flag == 0)
		{
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
                if(s[i] == seperator[x])
				{
                	++i;
                    flag = 0;
                    break;
                }
        }     
        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0)
		{
            for(string_size x = 0; x < seperator.size(); ++x)
            	if(s[j] == seperator[x])
				{
					flag = 1;
					break;
                }
            if(flag == 0)
				++j;
        }
        if(i != j)
		{
            result.push_back(s.substr(i, j-i));
            i = j;
        }
    }
    return result;
}

/*
*函数名：    str2rdata  
*功能：  机器人字符串数据转数值   
*函数参数：  
string str		- string字符串
*返回值：	
vector<double>	- 数值
*/  
vector<double> str2rdata(string str)
{
	vector<double> rdata;
	int start = str.find("p["); 
	int end = str.find("]");
	string robootdata = str.substr(start+2, end-start-2);
	vector<string> rstr= strsplit(robootdata,",");
	if (rstr.size() != 6)
		return rdata;
	for (int i = 0; i< rstr.size(); i++)
	{
		double res;
    	stringstream ss;
		ss << rstr[i];
		ss >> res;
		rdata.push_back(res);
	}
	return rdata;
}

/*
*函数名：    robot_open  
*功能：     开启socket通信与机器人通信   
*函数参数：  
void	-空

*返回值：	
vector<double>	- 返回机器人初始位置姿态
*/  
vector<double> robot_open(void)
{
	if (Data_ServerInfo.Server_state != 1)
		Server_open(robot_port);

	while(1)
	{
		if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_amount > 0))
			break;
		usleep(3000);
	}
	//usleep(30000); 
	char TrojanJet[] = "TrojanJet";
	Server_write(0,TrojanJet);
	int read_count = 0;
	while(1)
	{
		if (Data_ServerInfo.Clinet_r[0] == 1)
			break;
		usleep(3000);
		read_count++;
		if (read_count > 300)
		{
			read_count = 0;
			Server_write(0,TrojanJet);
		}
		
	}
	
	char *buf;	
	buf = Server_read(0);
	string buff= buf;
	vector<double> rdata;
	rdata = str2rdata(buff);

	// 输出机器人初始位置姿态信息
	cout<<"\nInitial position information of the robot :\np[ ";
	for (int i = 0; i < rdata.size(); i++)
	{
		cout<<rdata[i]<<" ";
	}
	cout<<"]"<<endl;
	
    return rdata;

}

/*
*函数名：    socket_close  
*功能：     关闭socket通信   
*函数参数：  
void	- 空
*返回值：	
void	- 空
*/ 
void socket_close(void)
{
	vector<double> data;
	char *buff;
	data.push_back(0);
	data.push_back(2);
	send_robot(data);
	robot_read();
	/*
	
	*/
}


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
int send_robot(vector<double> rdata)
{
	char rstr[100];
	strcpy(rstr,"(");
	for (int i = 0; i < rdata.size(); i++)
	{
		char temp[10];
		sprintf(temp,"%f",rdata[i]);
		strcat(rstr,temp);  
		if (i < rdata.size() - 1) 
			strcat(rstr,",");
	}     
	strcat(rstr,")");
	//cout<<rstr<<endl;
	return Server_write(0,rstr);
}


/*
*函数名：	  robot_read  
*功能：      读取机器人ok数据   
*函数参数：  
void			- 空
*返回值：	
int				- 返回0表成功
*/ 
int robot_read(void)
{
	char *buf;
	buf = Server_read(0);
	if(strncmp(buf,"ok",2) == 0)
		return 0;	
	else	
	{
		cout<<"Roboot Error!\nbuf:"<<buf<<endl;
		return 1;
	}
}


/*
*函数名:		read_pose  
*功能：     	读取机器人当前位置   
*函数参数：  
void			- 空
*返回值：	
vector<double>	- 返回机器人当前位置姿态
*/
vector<double> read_pose(void)
{
	vector<double> data;
	char *buff;
	data.push_back(0);
	data.push_back(0);
	send_robot(data);
	while(1)
	{	
		if (Data_ServerInfo.Clinet_r[0] == 1)
			break;
		usleep(1000);
	}
	buff = Server_read(0);
	string buf= buff;
	vector<double> rdata;
	rdata = str2rdata(buf);
	return rdata;
}

/*
*函数名:		set_relative_pose  
*功能：  	   	设置当前位置为机器人相对移动位置   
*函数参数：  
void			- 空
*返回值：	
vector<double>	- 返回机器人当前位置姿态
*/
vector<double> set_relative_pose(void)
{
	vector<double> data;
	char *buff;
	data.push_back(0);
	data.push_back(1);
    
	send_robot(data);
	while(1)
	{
		if (Data_ServerInfo.Clinet_r[0] == 1)
			break;
		usleep(1000);
	}
	buff = Server_read(0);
	
	string buf= buff;
	vector<double> rdata;
	rdata = str2rdata(buf);
	
	return rdata;
}

/*
*函数名:		robot_move  
*功能：     	机器人移动pose   
*函数参数：  
vector<double> rdata	- 移动pose (x,y,z,rx,ry,rz)
int mod					- 移动模式 0 绝对移动 1 相对移动
*返回值：	
int						- 返回0表示移动成功
*/
int robot_move(vector<double> rdata, int mod)
{
	vector<double> data;
	char *buff;
	if (mod == 0)		// 绝对位置移动
		data.push_back(1);
	else if (mod == 1)	// 相对位置移动
		data.push_back(2);
	else
		return 1;
	// UR机器人的X Y Z是以米为单位
	rdata[0] = rdata[0]/1000.0;
	rdata[1] = rdata[1]/1000.0;
	rdata[2] = rdata[2]/1000.0;
	data.insert(data.end(),rdata.begin(),rdata.end());
	
	return send_robot(data);	
	
}

/*
*函数名:		robot_cmd  
*功能：     	机器人io/ad控制  
*函数参数：  
vector<double> rdata	- io/ad控制数据
*返回值：	
int						- 返回0表示控制成功
*/
int robot_cmd(vector<double> rdata, int mod)
{
	vector<double> data;  
	char *buff;
	if (mod == 0)		// IO控制
		data.push_back(3);
	else if (mod == 1)	// AD控制
		data.push_back(4);
	else
		return 1;
	data.insert(data.end(),rdata.begin(),rdata.end());
	
	return send_robot(data);
}


/*
*函数名:		robot_event  
*功能：     	机器人   触发事件
*函数参数： 
vector<double> rdata	- (x y angle RES RES)
double event			- 触发事件
*返回值：	
int						- 返回0表成功
*/
int robot_event(vector<double> rdata, double event)
{
	vector<double> data;
	char *buff;
	data.push_back(5);
	// UR机器人的X Y Z是以米为单位
	rdata[0] = rdata[0]/1000.0;
	rdata[1] = rdata[1]/1000.0;
	data.insert(data.end(),rdata.begin(),rdata.end());
	data.push_back(event);
	
	return send_robot(data);
}

/*
*函数名:		calib_read_ExternalM  
*功能：     	手眼标定专用，读取UR姿态外参数矩阵
*函数参数： 
void			- 空
*返回值：	
Mat				- 返回机器人外参数矩阵
*/
Mat calib_read_ExternalM(void)
{
	if (Data_ServerInfo.Clinet_state[0] == 1)
	{
		vector<double> rdata;
		rdata = robot_open();
		
		Mat P = (Mat_<double>(3, 1) << rdata[0]*1000,rdata[1]*1000,rdata[2]*1000);
		Mat R = (Mat_<double>(3, 1) << rdata[3],rdata[4],rdata[5]);
		Mat buff = (Mat_<double>(1, 4) << 0,0,0,1);
		
		// 旋转矩阵
		Mat RotationM = Mat(3, 3, CV_64FC1, Scalar::all(0));
		Rodrigues(R, RotationM);

		Mat M_temp;
		hconcat(RotationM, P, M_temp);
		Mat ExternalM;
		vconcat(M_temp, buff, ExternalM);
		
		Data_ServerInfo.Clinet_r[0] = 0;

		return ExternalM;
	}
	else
	{
		Mat ExternalM = (Mat_<double>(1, 1) << 0);
		return ExternalM;
	}

}

/*
Mat calib_read_ExternalM(void)
{
	if (Data_ServerInfo.Clinet_state[0] == 1)
    {
        while(1)
        {
            if (Data_ServerInfo.Clinet_r[0] == 1)
            {
				vector<double> rdata;
                rdata = str2rdata(Server_read(0));
				
				Mat P = (Mat_<double>(3, 1) << rdata[0]*1000,rdata[1]*1000,rdata[2]*1000);
				Mat R = (Mat_<double>(3, 1) << rdata[3],rdata[4],rdata[5]);
				Mat buff = (Mat_<double>(1, 4) << 0,0,0,1);
				
				// 旋转矩阵
				Mat RotationM = Mat(3, 3, CV_64FC1, Scalar::all(0));
				Rodrigues(R, RotationM);

				Mat M_temp;
				hconcat(RotationM, P, M_temp);
				Mat ExternalM;
				vconcat(M_temp, buff, ExternalM);
				
                Data_ServerInfo.Clinet_r[0] = 0;

				return ExternalM;
            }
			usleep(3000);
        }
    }
	else
	{
		Mat ExternalM = (Mat_<double>(1, 1) << 0);
		return ExternalM;
	}

}
*/

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
Mat calib_Tsai(vector<Mat> robotExternalMList,vector<Mat> imageExternalMList,int rdata_mod)
{	
	vector<Mat> Hgij;
	vector<Mat> Hcij;
	Mat AA,bb;
	int n = imageExternalMList.size();
	for(int i = 0; i < n-1; i++)
	{
		//cout<<"robotExternalMList["<<i<<"]"<<robotExternalMList[i]<<endl;
		//cout<<"imageExternalMList["<<i<<"]"<<imageExternalMList[i]<<endl;
		if (rdata_mod == 0) 	// 眼在手上
		{
			Hgij.push_back(robotExternalMList[i+1].inv()*robotExternalMList[i]);
			Hcij.push_back(imageExternalMList[i+1]*imageExternalMList[i].inv());
		}
		else					// 眼睛在手外
		{
			//cout<<"######################"<<rdata_mod<<endl;
			Hgij.push_back(robotExternalMList[i+1].inv()*robotExternalMList[i]);
			Hcij.push_back(imageExternalMList[i+1].inv()*imageExternalMList[i]);
		}
		//cout<<"Hgij["<<i<<"]"<<Hgij[i]<<endl;
		//cout<<"Hcij["<<i<<"]"<<Hcij[i]<<endl;
		Mat rgij,rcij;
		Rodrigues(Hgij.back()(Rect(0,0,3,3)),rgij);		// 1行三列
		Rodrigues(Hcij.back()(Rect(0,0,3,3)),rcij);
		//cout<<"rgij"<<rgij<<endl;
		//cout<<"rcij"<<rcij<<endl;
		double theta_gij,theta_cij;
		theta_gij = norm(rgij);
		theta_cij = norm(rcij);	
		//cout<<"theta_gij"<<theta_gij<<endl;
		//cout<<"theta_cij"<<theta_cij<<endl;
		if ((theta_gij == 0) || (theta_cij == 0))
		{
			Hgij.erase(Hgij.end( ));
			Hcij.erase(Hcij.end( ));
			continue;	
		}	
		Mat rngij,rncij;
		rngij = rgij/theta_gij;
        rncij = rcij/theta_cij;
		//cout<<"rngij"<<rngij<<endl;
		//cout<<"rncij"<<rncij<<endl;
		Mat Pgij,Pcij,Pgc;
		Pgij = 2*sin(theta_gij/2)*rngij;
        Pcij = 2*sin(theta_cij/2)*rncij;
		//cout<<"Pgij"<<Pgij<<endl;
		//cout<<"Pcij"<<Pcij<<endl;
		if (AA.empty()) 			
		{
			AA = calib_crossprod(Pgij+Pcij);
			bb = Pcij-Pgij;
		}
		else
		{
			Mat bufAA = AA;
			Mat bufbb = bb;
			vconcat(bufAA,calib_crossprod(Pgij+Pcij),AA);
			vconcat(bufbb,Pcij-Pgij,bb);
		}
		
	}
	if ((int)Hgij.size() < 11)
	{
		cout<<"Overduplication,Unable to calibrate!"<<endl;	//重复数据过多无法标定
		Mat Tm = Mat::zeros(4,4,CV_8U);
		return Tm;
	}
	//cout<<"AA"<<AA<<endl;
	//cout<<"bb"<<bb<<endl;
	
	Mat AA_pinv;
	invert(AA,AA_pinv,DECOMP_SVD);
	Mat Pcg_prime = AA_pinv*bb;

	Mat err = AA*Pcg_prime-bb;
	Mat err_buff = err.t()*err;
	double residus_TSAI_rotation = sqrt(err_buff.at<double>(0,0)/(n-1));
	
	Mat Pcg = 2*Pcg_prime/sqrt(1+pow(norm(Pcg_prime),2));
	Mat Rcg = (1-norm(Pcg)*norm(Pcg)/2)*Mat::eye(3,3,CV_64FC1) + 0.5*(Pcg*Pcg.t()+sqrt(4-norm(Pcg)*norm(Pcg))*calib_crossprod(Pcg));

	//cout<<"Pcg"<<Pcg<<endl;
	//cout<<"Rcg"<<Rcg<<endl;
	for (int i = 0; i < Hgij.size()-1; i++)
	{
		if (i == 0) 			
		{
			AA = Hgij[i](Rect(0,0,3,3))-Mat::eye(3,3,CV_64FC1);
			bb = Rcg*Hcij[i](Rect(3,0,1,3))-Hgij[i](Rect(3,0,1,3));
		}
		else
		{
			Mat bufAA = AA;
			Mat bufbb = bb;
			vconcat(bufAA,Hgij[i](Rect(0,0,3,3))-Mat::eye(3,3,CV_64FC1),AA);
			vconcat(bufbb,Rcg*Hcij[i](Rect(3,0,1,3))-Hgij[i](Rect(3,0,1,3)),bb);
		}
	}
	invert(AA,AA_pinv,DECOMP_SVD);
	//cout<<"AA"<<AA<<endl;
	//cout<<"bb"<<bb<<endl;
	
	Mat Tcg = AA_pinv*bb;	
	//cout<<"Tcg"<<Tcg<<endl;
	err = AA*Tcg-bb;
	err_buff = err.t()*err;
	residus_TSAI_rotation = sqrt(err_buff.at<double>(0,0)/(n-1));

	Mat Tm_buff1,Tm_buff2,Tm; 
	Mat buff = (Mat_<double>(1, 4) << 0,0,0,1);
	hconcat(Rcg,Tcg,Tm_buff1);
	vconcat(Tm_buff1,buff,Tm);	
	//cout<<"Tm"<<Tm<<endl;
	return Tm;	
}


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
Mat calib_Navy(vector<Mat> robotExternalMList,vector<Mat> imageExternalMList,int rdata_mod)
{
	vector<Mat> Ta,Tb,Ra;
	Mat M = Mat::zeros(3,3,CV_64FC1);
	int n = imageExternalMList.size();
	for(int i = 0; i < n-1; i++)
	{
		Mat A,B;
		//cout<<"robotExternalMList["<<i<<"]"<<robotExternalMList[i]<<endl;
		//cout<<"imageExternalMList["<<i<<"]"<<imageExternalMList[i]<<endl;
		if (rdata_mod == 0) 	// 眼在手上
		{
			A = robotExternalMList[i+1].inv()*robotExternalMList[i];
			B = imageExternalMList[i+1]*imageExternalMList[i].inv();
		}
		else					// 眼在手外
		{
			A = robotExternalMList[i+1].inv()*robotExternalMList[i];
			B = imageExternalMList[i+1].inv()*imageExternalMList[i];
		}
		Mat alpha = calib_logMatrix(A);
        Mat beta = calib_logMatrix(B);
		//cout<<"alpha"<<alpha<<endl;
		//cout<<"beta"<<beta<<endl;
		double alpha_sum = alpha.at<double>(0,0)+alpha.at<double>(0,1)+alpha.at<double>(0,2);
		double beta_sum = beta.at<double>(0,0)+beta.at<double>(0,1)+beta.at<double>(0,2);
		//cout<<"alpha_sum"<<alpha_sum<<endl;
		//cout<<"beta_sum"<<beta_sum<<endl;
		if ((alpha_sum == 0) || (beta_sum == 0))
		{
			continue;	
		}
		Ta.push_back(A(Rect(3,0,1,3)));
		Ra.push_back(A(Rect(0,0,3,3)));
		Tb.push_back(B(Rect(3,0,1,3)));
		//cout<<"Ta"<<Ta.back()<<endl;
		//cout<<"Ra"<<Ra.back()<<endl;
		//cout<<"Tb"<<Tb.back()<<endl;
		M +=  beta.t()*alpha;
		//cout<<"M"<<M<<endl;
	}
	//cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	
	if ((int)Ta.size() < 11)
	{
		cout<<"Overduplication,Unable to calibrate!"<<endl;	//重复数据过多无法标定
		Mat Tm = Mat::zeros(4,4,CV_8U);
		return Tm;
	}
	
	Mat MM = M.t()*M;
	Mat d,V;
	eigen(MM,d,V);
	V = V.inv();	// 与pyhton求出来的是逆的关系
	//cout<<"MM"<<MM<<endl;
	//cout<<"d"<<d<<endl;
	//cout<<"V"<<V<<endl;
	
	Mat D = Mat::zeros(3,3,CV_64FC1);
	D.at<double>(0,0) = pow(d.at<double>(0,0),-0.5);
	D.at<double>(1,1) = pow(d.at<double>(0,1),-0.5);
	D.at<double>(2,2) = pow(d.at<double>(0,2),-0.5);

	//cout<<"D"<<D<<endl;
	Mat Rx = V*D*V.inv()*M.t();
	//cout<<"Rx"<<Rx<<endl;
	//cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
	Mat cc,dd; 
	for (int i =0; i < Ta.size(); i++)
	{
		if (i == 0) 			
		{
			cc = Mat::eye(3,3,CV_64FC1)-Ra[i];
			dd = Ta[i]-Rx*Tb[i];
		}
		else
		{
			Mat bufcc = cc;
			Mat bufdd = dd;
			vconcat(bufcc,Mat::eye(3,3,CV_64FC1)-Ra[i],cc);
			vconcat(bufdd,Ta[i]-Rx*Tb[i],dd);
		}
	}
	//cout<<"cc"<<cc<<endl;
	//cout<<"dd"<<dd<<endl;
	Mat Tx = (cc.t()*cc).inv()*(cc.t()*dd);
	//cout<<"Tx"<<Tx<<endl;
	Mat Tm,Tm_buff;
	Mat buff = (Mat_<double>(1, 4) << 0,0,0,1);	
	hconcat(Rx,Tx,Tm_buff);
	vconcat(Tm_buff,buff,Tm);
	//cout<<"Tm_buff"<<Tm_buff<<endl;
	//cout<<"Tm"<<Tm<<endl;
	return Tm;
}


/*
*函数名:		calib_crossprod  
*功能：     	手眼标定专用，求解叉积函数
*函数参数： 
Mat a		- 输入Mat
*返回值：	
Mat			- 返回Mat

*/
static Mat calib_crossprod(Mat a)
{
	Mat ax = (Mat_<double>(3, 3) << 0,-a.at<double>(0,2),a.at<double>(0,1),
									a.at<double>(0,2), 0,-a.at<double>(0,0),
									-a.at<double>(0,1),a.at<double>(0,0),0);

   return ax;
}

/*
*函数名:		calib_crossprod  
*功能：     	手眼标定专用，矩阵对数函数
*函数参数： 
Mat a		- 输入Mat
*返回值：	
Mat			- 返回Mat
*/
static Mat calib_logMatrix(Mat M)
{
    Mat R = M(Rect(0,0,3,3));
    double trace = R.at<double>(0,0)+R.at<double>(1,1)+R.at<double>(2,2);

    double fi = acos((trace-1)/2);
	
    if (sin(fi) == 0)
    {
        return Mat::eye(1,3,CV_64FC1);
    }
	
    Mat w = fi/(2*sin(fi))*(R-R.t());

	Mat W = (Mat_<double>(1, 3) << w.at<double>(2,1),w.at<double>(0,2),w.at<double>(1,0));
	
    return W;
}


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
Mat	Pixel2XYZ_3d(vector<Point2d> coord,Mat m,Mat M)
{
	if (coord.size()<2)
	{
		Mat XYZ = (Mat_<double>(1, 3) << 0,0,0);
		return XYZ;
	}
	double u1 = coord[0].x;
	double v1 = coord[0].y;
	double u2 = coord[1].x;
	double v2 = coord[1].y;
	
	double m11 = m.at<double>(0,0);
	double m12 = m.at<double>(0,1);
	double m13 = m.at<double>(0,2);
	double m14 = m.at<double>(0,3);

	double m21 = m.at<double>(1,0);
	double m22 = m.at<double>(1,1);
	double m23 = m.at<double>(1,2);
	double m24 = m.at<double>(1,3);

	double m31 = m.at<double>(2,0);
	double m32 = m.at<double>(2,1);
	double m33 = m.at<double>(2,2);
	double m34 = m.at<double>(2,3);

	double M11 = M.at<double>(0,0);
	double M12 = M.at<double>(0,1);
	double M13 = M.at<double>(0,2);
	double M14 = M.at<double>(0,3);

	double M21 = M.at<double>(1,0);
	double M22 = M.at<double>(1,1);
	double M23 = M.at<double>(1,2);
	double M24 = M.at<double>(1,3);

	double M31 = M.at<double>(2,0);
	double M32 = M.at<double>(2,1);
	double M33 = M.at<double>(2,2);
	double M34 = M.at<double>(2,3);

	/*		之前的计算XYZ方法 
	double X,Y,Z;
	Z = -(((M14 - M34*u2)/(M11 - M31*u2) - (m14 - m34*u1)/(m11 - m31*u1))/((M12 - M32*u2)/(M11 - M31*u2) - (m12 - m32*u1)/(m11 - m31*u1)) - ((M24 - M34*v2)/(M21 - M31*v2) - (m24 - m34*v1)/(m21 - m31*v1))/((M22 - M32*v2)/(M21 - M31*v2) - (m22 - m32*v1)/(m21 - m31*v1)))/(((M13 - M33*u2)/(M11 - M31*u2) - (m13 - m33*u1)/(m11 - m31*u1))/((M12 - M32*u2)/(M11 - M31*u2) - (m12 - m32*u1)/(m11 - m31*u1)) - ((M23 - M33*v2)/(M21 - M31*v2) - (m23 - m33*v1)/(m21 - m31*v1))/((M22 - M32*v2)/(M21 - M31*v2) - (m22 - m32*v1)/(m21 - m31*v1)));

	Y = ((m14 + Z*m13 - m34*u1 - Z*m33*u1)/(m11 - m31*u1) - (M14 + M13*Z - M34*u2 - M33*Z*u2)/(M11 - M31*u2))/((M12 - M32*u2)/(M11 - M31*u2) - (m12 - m32*u1)/(m11 - m31*u1));

	X = -(m14 + Y*m12 + Z*m13 - m34*u1 - Y*m32*u1 - Z*m33*u1)/(m11 - m31*u1);
	
	Mat XYZ = (Mat_<double>(1, 3) << X,Y,Z);
	*/

	// 最小二乘法求解
	double Ml[4][3] = { {u1*m31-m11 , u1*m32-m12 , u1*m33-m13},
						{v1*m31-m21 , v1*m32-m22 , v1*m33-m23}, 
						{u2*M31-M11 , u2*M32-M12 , u2*M33-M13},
						{v2*M31-M21 , v2*M32-M22 , v2*M33-M23}};
	double Mr[4][1] = { {m14-u1*m34},
						{m24-v1*m34},
						{M14-u2*M34},
						{M24-v2*M34}};
	
	Mat ML(Size(3, 4), CV_64FC1, Ml);
	Mat MR(Size(1, 4), CV_64FC1, Mr);
	Mat XYZ = (ML.t()*ML).inv()*ML.t()*MR;
	return XYZ.t();
}


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
Point2d Distor_Correc(Point2d image_point, Mat Min, Mat kc)
{
	double u = image_point.x;
	double v = image_point.y;

	double ui,vi;
	// 焦距归一化
	ui = (u - Min.at<double>(0,2))/Min.at<double>(0,0);
	vi = (v - Min.at<double>(1,2))/Min.at<double>(1,1);
	
	double r = sqrt(ui*ui+vi*vi);

	double ud,vd;
	ud = ui*(1+kc.at<double>(0,0)*r*r+kc.at<double>(0,1)*r*r*r*r+kc.at<double>(0,4)*r*r*r*r*r*r)+2*kc.at<double>(0,2)*ui*vi+kc.at<double>(0,3)*(r*r+2*ui*ui);
    vd = vi*(1+kc.at<double>(0,0)*r*r+kc.at<double>(0,1)*r*r*r*r+kc.at<double>(0,4)*r*r*r*r*r*r)+2*kc.at<double>(0,3)*ui*vi+kc.at<double>(0,2)*(r*r+2*vi*vi);
	
	u = Min.at<double>(0,0)*ud+Min.at<double>(0,2);
    v = Min.at<double>(1,1)*vd+Min.at<double>(1,2);

	Point2d out_point;
	out_point.x = u;
	out_point.y = v;

	return out_point;
}

/*
*函数名：    Cmd_URrobot_send  
*功能：     向UR端port端口发送data数据
*函数参数：  
long port			- 端口号
const char* data	- 发送数据
*返回值：	
int				- 0表成功
*/
int Cmd_URrobot_send(long port, const char* data)
{
	int server_fd = -1;
	ClientInfo Data_ClientInfo;
	
	for (int i = 0; i < VData_ClientInfo.size(); i++)
	{
		if (VData_ClientInfo[i].Server_fds != -1)
			if (VData_ClientInfo[i].Server_port == port)
			{
				server_fd = VData_ClientInfo[i].Server_fds;
				Data_ClientInfo = VData_ClientInfo[i];
				break;
			}
	}

	if (server_fd == -1)
	{
		if (Client_open(port,robot_ip) == -1)
			return -1;
		else
		{
			for (int i = 0; i < VData_ClientInfo.size(); i++)
			{
				if (VData_ClientInfo[i].Server_fds != -1)
					if (VData_ClientInfo[i].Server_port == port)
					{
						server_fd = VData_ClientInfo[i].Server_fds;
						Data_ClientInfo = VData_ClientInfo[i];
						break;
					}
			}
		}
	}
	
	return Client_write(Data_ClientInfo,data);

}

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
int speed		- speed 速度 1-40
*返回值：	
int				- 0表成功
*/
int Cmd_URrobot_move(int mod,int distance,int speed)
{
	char c_distance[128];

	if (speed < 1)
		speed = 1;
	else if (speed > 40)
		speed = 40;
	
	switch(mod)
	{
		case 0:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[%f,0,0,0,0,0]),%d)\n",(double)distance/1000.0,speed);break;	
		case 1:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[0,%f,0,0,0,0]),%d)\n",(double)distance/1000.0,speed);break;
		case 2:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[0,0,%f,0,0,0]),%d)\n",(double)distance/1000.0,speed);break;
		case 3:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[0,0,0,%f,0,0]),%d)\n",(double)distance*3.14159/180,speed);break;
		case 4:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[0,0,0,0,%f,0]),%d)\n",(double)distance*3.14159/180,speed);break;
		case 5:sprintf(c_distance,"movej(pose_add(get_actual_tcp_pose(),p[0,0,0,0,0,%f]),%d)\n",(double)distance*3.14159/180,speed);break;
	}
	return Cmd_URrobot_send(30001,c_distance);
	
}


/*
*函数名：    robot_slave_read  
*功能：     上位机从机等待读取UR信号
*函数参数：  
void - 空
*返回值：	
RobotInfo	- 返回机器人信息结构体
*/
RobotInfo robot_slave_read(void)
{
	RobotInfo RINFO;
	if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_amount > 0) && (Data_ServerInfo.Clinet_r[0] == 1))
	{	
		char *buf;	
		buf = Server_read(0);
		string buff= buf;
		
		int start = buff.find("["); 
		int end = buff.find("]");
		string robootdata = buff.substr(start+1, end-start-1);

		vector<string> rstr= strsplit(robootdata,",");
		if (rstr.size() != 10)
			return RINFO;

		double res1,res2,res3,res4;
		stringstream ss1,ss2,ss3,ss4;
		
		ss1 << rstr[0];
		ss1 >> res1;
		ss2 << rstr[1];
		ss2 >> res2;
		ss3 << rstr[2];
		ss3 >> res3;
		ss4 << rstr[3];
		ss4 >> res4;
		
		RINFO.Robot_event = (int)res1;
		RINFO.Robot_OHeight = res2; 
		RINFO.Robot_model = (int)res3;
		RINFO.Robot_direction = (int)res4;

		vector<double> rdata;
		for (int i = 4; i< rstr.size(); i++)
		{
			double res;
	    	stringstream ss;
			ss << rstr[i];
			ss >> res;
			rdata.push_back(res);
		}
		rdata[0] = rdata[0]*1000;
		rdata[1] = rdata[1]*1000;
		rdata[2] = rdata[2]*1000;
		RINFO.Robot_pose = rdata;
	}
	return RINFO;
}

/*
RobotInfo robot_slave_read(void)
{
	RobotInfo RINFO;
	if ((Data_ServerInfo.Server_state == 1) && (Data_ServerInfo.Clinet_amount > 0))
	{
		
		vector<double> rdata;
		// 开启socket通讯，并读取机器人初始坐标信息
		rdata = robot_open();
		
		// 机器人传回来的XYZ是以米为单位
		rdata[0] = rdata[0]*1000;
		rdata[1] = rdata[1]*1000;
		rdata[2] = rdata[2]*1000;

		int read_count = 0;
		int read_count_temp = 0;
		static long open_max_count = 0;
		static long open_min_count = 999;
		static long open_mean_count = 0;
		static long open_sum_count = 0;
		static long open_count = 0;
		open_count++;
		while(1)
		{
			
			if (Data_ServerInfo.Clinet_r[0] == 1)
				break;
			usleep(3000);
			//cout<<"xsc3:"<<read_count<<endl;
			read_count++;
			if (read_count > 300)
			{
				read_count_temp++;
				read_count = 0;
				return RINFO;
			}			
			
		}

		read_count = read_count+read_count_temp*300;
		if (read_count > open_max_count)
			open_max_count = read_count;
		if (read_count < open_min_count)
			open_min_count = read_count;
		open_sum_count += read_count;
		open_mean_count = open_sum_count/open_count;
		cout<<"----slave_read----"<<endl;
		cout<<"slave_read_max_count:"<<open_max_count<<endl;
		cout<<"slave_read_min_count:"<<open_min_count<<endl;
		cout<<"slave_read_mean_count:"<<open_mean_count<<endl;
		
		char *buf;	
		buf = Server_read(0);
		string buff= buf;
		
		int start = buff.find("["); 
		int end = buff.find("]");
		string robootdata = buff.substr(start+1, end-start-1);

		vector<string> rstr= strsplit(robootdata,",");
		if (rstr.size() != 3)
			return RINFO;

		double res1,res2,res3;
		stringstream ss1,ss2,ss3;
		
		ss1 << rstr[0];
		ss1 >> res1;
		ss2 << rstr[1];
		ss2 >> res2;
		ss3 << rstr[2];
		ss3 >> res3;
		
		RINFO.Robot_event = (int)res1;
		RINFO.Robot_OHeight = res2;	
		RINFO.Robot_model = (int)res3;
		RINFO.Robot_pose = rdata;
	}

	return RINFO;
}
*/


