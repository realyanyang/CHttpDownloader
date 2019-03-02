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
	bool supportResumeDownload();     //检测服务器是否支持断点续传
	void startDownloader();       //开始下载，作为线程函数
	void cleanTempFile();            //清理临时文件
	void mergeTempFile();           //合并临时文件
	~HttpDownloader();
	//void threadMonitor();            //主线程监视器
private:
	bool resumable;              //是否可以分开传输
	CURL *curl;                      //定义CURL类型的指针
	CURLcode res;                     //返回的信息
	CURLcode info;                   //响应头信息
	int fileSize;                    //文件大小
	int threadNum = 8;           //线程个数
	int timeOut = 5000;         //time out 5000 毫秒
	const char* urlAddress;         //访问的URL地址s
	const char* fileAdress;              //保存文件位置
	long int minSize;             //分线程的最小大小
	//FILE *fp;            //文件指针

};