/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#pragma once
#include <iostream>
#include <curl/curl.h>
#include <string>
using namespace std;

const int threadNum = 8;           //线程个数

struct start_end                     //定义结构 标识一段文件的起始和结束位置
{
	long startPoint;
	long endPonint;
};

class HttpDownloader {
public:
	HttpDownloader(const char url[], const char file[]);           //构造函数
	//void startDownloader();       //开始下载，作为线程函数
	void cleanTempFile();            //清理临时文件
	bool mergeTempFile();           //合并临时文件
	void newDownladThread(long startpoint, long endpoint, string tmpfile, int num);    //开启新的下载线程
	string creatTmpFile(int num);      //生成临时文件路径
	void getFileName(char* fileName);          //获取所下载文件的名称/*未实现获取文件名*/
	bool getConnectable();   
	bool getAdressable();
	void singleDown();          //单线程下载
	start_end* getStartEnd();             //多线程下载
	~HttpDownloader();
	bool threadMonitor();            //线程监视器
	bool getResumable() { return resumable; }           //返回是否可以分段下载
private:
	bool resumable;              //是否可以分开传输
	bool connectAble;           //服务器是否可以连接
	bool downloadAdressable;      //输入地址是否合法
	CURL *curl;                      //定义CURL类型的指针
	//CURLcode res;                     //返回的信息
	CURLcode info;                   //响应头信息
	int timeOut = 5000;         //time out 5000 毫秒
	const char* urlAddress;         //访问的URL地址s
	const char* fileAdress;              //保存文件位置
	long int minSize = 1024;             //分线程的最小大小
	long fileSize;      //下载文件的大小

};

string mygetFileSize(double fileSize);   //将文件大小转换成string型，并带单位s

