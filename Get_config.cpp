/*
*函数名 ： 读取配置文件的信息
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/5/21
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
*/

#include "Get_config.hpp"


/*
*函数名：	IsSpace
*功能：	  判断空格
*函数参数：	
char c		- 字符
*返回值：				
bool		- true表存在
*/
static bool IsSpace(char c)
{
	if (' ' == c || '\t' == c)
		return true;
	return false;
}

/*
*函数名：	Trim
*功能：	  处理等号两侧的字符串
*函数参数：	
string & str	- 字符串
*返回值：				
void			- 空
*/
static void Trim(string & str)
{
	if (str.empty()) 
	{
		return;
	}
	int i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i) 
	{
		if (!IsSpace(str[i])) 
		{
			break;
		}
	}
	if (i == str.size())
	{ // 全部是空白字符串
		str = "";
		return;
	}
	start_pos = i;
	for (i = str.size() - 1; i >= 0; --i)
	{
		if (!IsSpace(str[i]))
		{
			break;
		}
	}
	end_pos = i;
	str = str.substr(start_pos, end_pos - start_pos + 1);
}


/*
*函数名：	AnalyseLine
*功能：	  处理1行数据
*函数参数：	
const string & line		- 1行字符串
string & key			- 存放key
string & value			- 存放value
*返回值：				
bool								- true表成功
*/
static bool AnalyseLine(const string & line, string & key, string & value)
{
	if (line.empty())
		return false;
	int start_pos = 0, end_pos = line.size() - 1, pos;
	if ((pos = line.find(COMMENT_CHAR)) != -1) 
	{
		if (0 == pos)
		{  // 行的第一个字符就是注释字符
			return false;
		}
		end_pos = pos - 1;
	}
	string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分

	if ((pos = new_line.find('=')) == -1)
		return false;  // 没有=号

	key = new_line.substr(0, pos);
	value = new_line.substr(pos + 1, end_pos + 1- (pos + 1));

	Trim(key);
	if (key.empty()) 
	{
		return false;
	}
	Trim(value);
	return true;
}

/*
*函数名：	ReadConfig
*功能：	  读取出来的MAP数据
*函数参数：	
const string & filename				- 配置文件名
const map<string, string> & m		- 存放map
*返回值：				
bool								- true表成功
*/
bool ReadConfig(const string & filename, map<string, string> & m)
{
	m.clear();
	ifstream infile(filename.c_str());
	if (!infile) 
	{
		cout << "file open error" << endl;
		return false;
	}
	string line, key, value;
	while (getline(infile, line)) 
	{
		if (AnalyseLine(line, key, value))
		{
			m[key] = value;
		}
	}
	infile.close();
	return true;
}


/*
*函数名：	PrintConfig
*功能：	  打印读取出来的MAP数据
*函数参数：	
const map<string, string> & m		- 读出map
*返回值：				
void								- 空
*/
void PrintConfig(const map<string, string> & m)
{
	map<string, string>::const_iterator mite = m.begin();
	for (; mite != m.end(); ++mite)
	{
		cout << mite->first << "=" << mite->second << endl;

	}
}






