/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#pragma once
#include <iostream>
#include <curl/curl.h>

using namespace std;

class HttpDownloader {
public:
	HttpDownloader(const char url[], const char file[]);           //构造函数
	void startDownloader();       //开始下载，作为线程函数

	void cleanTempFile();            //清理临时文件
	void mergeTempFile();           //合并临时文件
	void getFileName(char* fileName);          //获取所下载文件的名称
	/*未实现获取文件名*/
	~HttpDownloader();
	//void threadMonitor();            //主线程监视器
private:
	bool resumable;              //是否可以分开传输
	bool connectAble;           //服务器是否可以连接
	CURL *curl;                      //定义CURL类型的指针
	//CURLcode res;                     //返回的信息
	CURLcode info;                   //响应头信息
	int threadNum = 8;           //线程个数
	int timeOut = 5000;         //time out 5000 毫秒
	const char* urlAddress;         //访问的URL地址s
	const char* fileAdress;              //保存文件位置
	long int minSize;             //分线程的最小大小
	double fileSize;      //下载文件的大小
	double localFileSize;     //已下载到本地的文件大小
	int startPoints[2];            //断点起始位置
	//FILE *fp;            //文件指针

};
