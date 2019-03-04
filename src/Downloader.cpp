/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <time.h>
#include <iomanip>
#include <regex>
#include <mutex>

bool downloadSuccess[threadNum] = { 0 };
mutex mut;               //线程锁

HttpDownloader::HttpDownloader(const char url[], const char file[])
{
	curl_global_init(CURL_GLOBAL_ALL);
	urlAddress = url;
	fileAdress = file;
	curl = curl_easy_init();
	if (curl)
	{
		//设置curl选项.
		struct curl_slist *headers = NULL;
		CURLcode res;
		headers = curl_slist_append(headers, "Range: bytes=0-");
		curl_easy_setopt(curl, CURLOPT_URL, urlAddress);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
		int i = 0;
		for (i = 0; i < 3; i++)
		{
			timeOut = timeOut + i * 1500;
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeOut);
			//调用curl_easy_perform 执行.并进行相关的操作.
			res = curl_easy_perform(curl);
			cout << "this res is ::" << res << endl;
			if (res == CURLE_OK)
				break;
			else
				continue;
		}
		cout << "i:::" << i << endl;
		if (i == 3)
		{
			cout << "**无法连接到服务器" << endl;
			connectAble = false;
		}
		else
			connectAble = true;
		long response_code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileSize);
		cout << "size::::" << fileSize << endl;
		//cout << response_code << endl;
		//清除curl操作.
		curl_easy_cleanup(curl);
		if (response_code == 206)
		{
			resumable = true;
			cout << "**支持断点续传" << endl;
		}
		else
		{
			resumable = false;
			cout << "**不支持断点续传" << endl;
		}
	}
	else
	{
		resumable = false;
		cout << "**初始化失败" << endl;

	}
}
HttpDownloader::~HttpDownloader()
{
	curl_global_cleanup();
}


size_t write_data(char *buffer, size_t size, size_t nmemb, void* stream)
{
	size_t written = fwrite(buffer, size, nmemb, (FILE*)stream);
	return written;
}

//void HttpDownloader::getFileName(char* fileName)
//{
//	int urlLenth = strlen(urlAddress);
//	cout << urlAddress << endl;
//	cout << urlLenth << endl;
//	int k = urlLenth;
//	while (k > 0 && urlAddress[k] != '/')
//		k--;
//	cout << k << endl;
//	char *tmp = new char[urlLenth];
//	strcpy(tmp, urlAddress);
//	fileName = &tmp[k + 1];
//	cout << fileName << endl;
//}

/******************///全局变量 用来计算下载速度
double preFileSize = 0;
clock_t preTime = 0;
/*****************/
int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	long double speed;
	double dByte;
	clock_t nowTime = clock();
	long double dTime = (long double)(nowTime - preTime) / CLOCKS_PER_SEC; //单位为秒s
	if (dTime >= 1)               //最少每1s显示一次速度
	{
		dByte = dlnow - preFileSize;
		speed = dByte / dTime;
		if (speed >= 1024 && speed < 1024 * 1024)
			cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / 1024 << "KB/s" << "    " << (dlnow / dltotal) * 100 << " %"<< endl;
		else if (speed >= 1024 * 1024 && speed < 1024 * 1024 * 1024)
			cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / (1024 * 1024) << "MB/s" << "    " << (dlnow / dltotal) * 100 << " %" << endl;
		else if (speed >= 1024 * 1024 * 1024)
			cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / (1024 * 1024 * 1024) << "GB/s" << "    " << (dlnow / dltotal) * 100 << " %" << endl;
		else
			cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed << "B/s" << "    " << (dlnow / dltotal) * 100 << " %"<< endl;
		preTime = nowTime;
		preFileSize = dlnow;
	}


	//if (dlnow == preFileSize)
	//	return 0;
	//else
	//{
	//	clock_t nowTime = clock();
	//	long double dTime = (long double)(nowTime - preTime) / CLOCKS_PER_SEC;  //单位为秒s
	//	dByte = dlnow - preFileSize;
	//	speed = dByte / dTime;
	//	if (speed >= 1024 && speed < 1024 * 1024)
	//		cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / 1024 << "KB/s" << "    " << (dlnow / dltotal) * 100 << " %"<< endl;
	//	else if (speed >= 1024 * 1024 && speed < 1024 * 1024 * 1024)
	//		cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / (1024 * 1024) << "MB/s" << "    " << (dlnow / dltotal) * 100 << " %" << endl;
	//	else if (speed >= 1024 * 1024 * 1024)
	//		cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed / (1024 * 1024 * 1024) << "GB/s" << "    " << (dlnow / dltotal) * 100 << " %" << endl;
	//	else
	//		cout << "speed: " << setiosflags(ios::fixed) << setprecision(2) << speed << "B/s" << "    " << (dlnow / dltotal) * 100 << " %"<< endl;
	//	preTime = nowTime;
	//	preFileSize = dlnow;
	//}
	//记录下本次回调参数
	return 0;
}

void HttpDownloader::singleDown()
{
	CURLcode res;
	FILE *fp = fopen(fileAdress, "wb");
	char *progress_data = NULL;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, urlAddress);
	//curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);  //是否可以访问
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);  //显示进度
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);

	//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeOut);  //不要设置超时时间
	res = curl_easy_perform(curl);
	cout << res << endl;
	if (!res)
	{
		long double downloadSzie;
		double speed;
		cout << "here" << endl;
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &downloadSzie);
		curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
		cout << "size:" << downloadSzie << endl;
		cout << "average speed: " << speed << "bytes/sec" << " 100.00%" << endl;
	}
	curl_easy_cleanup(curl);
	fclose(fp);

}

start_end* HttpDownloader::getStartEnd()
{
	start_end st_en[threadNum];
	//double startPoints[threadNum];           //多线程下载起始位置
	//double endPoints[threadNum];             //多线程下载终止位置
	int eachFileSize = fileSize / threadNum;
	//cout << "eachfilezie:::::::::::" << eachFileSize << endl;
	for (int i = 0; i < threadNum; i++)
		st_en[i].startPoint = i * eachFileSize;
	for (int j = 0; j < threadNum - 1; j++)
		st_en[j].endPonint = st_en[j + 1].startPoint - 1;
	st_en[threadNum - 1].endPonint = fileSize - 1;
	/*for (int k = 0; k < 8; k++)
	{
		cout << st_en[k].startPoint << endl;
	}*/
	return st_en;
	/*创建新的下载线程*/
	/*HttpDownloader *p = this;
	for (int k = 0; k < threadNum; k++)
	{
		thread anThread(&HttpDownloader::newDownladThread, p, startPoints[k], endPoints[k], k);
	}*/



}


void HttpDownloader::newDownladThread(long startpoint, long endpoint, int num)
{
	/*创建临时文件*/
	//regex reg("\\\\\w+\.");
	//string str = fileAdress;
	//smatch result;
	//if (regex_match(str, result, reg))
	//	cout << result[0] << endl;
	//else
	//	cout << "no" << endl;
	/*创建临时文件*/
	string tmpFileName;                 //生成临时文件的文件名
	string tmpFilePath;                   //生成临时文件的路径
	int adressLenth = strlen(fileAdress);
	int k = adressLenth;
	while (k > 0 && fileAdress[k] != '\\')
		k--;
	if (k == 0)
		cout << "**输入地址无效" << endl;
	else
	{
		string tmp = fileAdress;
		tmpFileName = &tmp[k + 1];
		tmpFilePath = tmp.substr(0, k + 1);
		cout << tmpFilePath << endl;
		int i = tmpFileName.length();
		while (i > 0 && tmpFileName[i] != '.')
			i--;
		tmpFileName = tmpFileName.substr(0, i);
		tmpFileName = tmpFileName + "." + to_string(num) + "tmp";
	}
	cout << tmpFileName << endl;
	string tmpfile = tmpFilePath + tmpFileName;
	cout << tmpfile << endl;
	/*下载该临时文件*/
	CURL *sub_curl = curl_easy_init();
	if (sub_curl)
	{
		curl_slist *headers = NULL;
		CURLcode res;
		FILE *sub_fp = fopen(tmpfile.c_str(), "wb");
		string requrRange = "Range: bytes=" + to_string(startpoint) + "-" + to_string(endpoint);
		cout << "requrRange:  " << requrRange << endl;
		headers = curl_slist_append(headers, requrRange.c_str());
		curl_easy_setopt(sub_curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(sub_curl, CURLOPT_URL, urlAddress);
		/*curl_easy_setopt(sub_curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(sub_curl, CURLOPT_NOBODY, 1L);*/
		curl_easy_setopt(sub_curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(sub_curl, CURLOPT_WRITEDATA, sub_fp);

		mut.lock();
		res = curl_easy_perform(sub_curl);
		mut.unlock();

		if (res)
		{
			cout << "**线程写文件失败" << num << endl;
		}
		else
		{
			downloadSuccess[num] = true;
			cout << "**线程写文件成功" << num << endl;
		}
		curl_easy_cleanup(sub_curl);
		fclose(sub_fp);
	}
}


bool HttpDownloader::threadMonitor()
{
	int i;
	for (i = 0; i < threadNum; i++)
	{
		if (downloadSuccess[i])
			continue;
		else
			break;
	}
	if (i == threadNum)            //返回true即为所有线程下载完成
		return true;
	else                  // 返回false即为还有线程正在下载
		return false;
}

void HttpDownloader::startDownloader()
{
	if (!resumable)
		singleDown();
	else;
		//multiDown();
}
