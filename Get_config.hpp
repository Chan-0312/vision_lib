#ifndef _GET_CONFIG_H_
#define _GET_CONFIG_H_

/*
*函数名 ： 读取配置文件的信息
*作者 ：Barrett
*联系方式：Barrett.xiong@trojanjet.com
*函数创建日期 ： 2018/5/21
*版本 ： V1.0
	Copyright (C), 2012-2018 , TrojanJet Intelligent Technologies LTD , All rights reserved
* 修订：
*/


#include <string>
#include <map>
#include <fstream>
#include <iostream>

#define COMMENT_CHAR '#'

using namespace std;

/*
*函数名：	IsSpace
*功能：	  判断空格
*函数参数：	
char c		- 字符
*返回值：				
bool		- true表存在
*/
static bool IsSpace(char c);

/*
*函数名：	Trim
*功能：	  处理等号两侧的字符串
*函数参数：	
string & str	- 字符串
*返回值：				
void			- 空
*/
static void Trim(string & str);

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
static bool AnalyseLine(const string & line, string & key, string & value);

/*
*函数名：	ReadConfig
*功能：	  读取出来的MAP数据
*函数参数：	
const string & filename				- 配置文件名
const map<string, string> & m		- 存放map
*返回值：				
bool								- true表成功
*/
bool ReadConfig(const string & filename, map<string, string> & m);

/*
*函数名：	PrintConfig
*功能：	  打印读取出来的MAP数据
*函数参数：	
const map<string, string> & m		- 读出map
*返回值：				
void								- 空
*/
void PrintConfig(const map<string, string> & m);

#endif


