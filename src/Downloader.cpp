/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#pragma once
#include "Downloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>
#include <iomanip>
#include <regex>
#include <mutex>
#include <Windows.h>
#include <numeric>
bool downloadSuccess[threadNum] = { 0 };
mutex mut;               //线程锁
mutex mut1;
string tmpfileToMerge[threadNum];
string strFileSize;    //下载文件的大小，转换成string型，并带单位
long helpFileSize;    //下载文件大小

HttpDownloader::HttpDownloader(const char url[], const char file[])
{
	//curl_global_init(CURL_GLOBAL_ALL);
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
			//cout << "this res is ::" << res << endl;
			if (res == CURLE_OK)
				break;
			else
				continue;
		}
		if (i == 3)
		{
			cout << "**无法连接到服务器" << endl;
			connectAble = false;
		}
		else
			connectAble = true;
		long response_code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
		//cout << "size::::" << fileSize << endl;
		//cout << response_code << endl;
		//清除curl操作.
		curl_easy_cleanup(curl);
		if (response_code == 206 || fileSize != NULL || fileSize != 0)
		{
			resumable = true;
			cout << "**支持断点续传" << endl;
			strFileSize = mygetFileSize(fileSize);
			helpFileSize = fileSize;
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
	//curl_global_cleanup();
}

string mygetFileSize(double fileSize)
{
	double tmp;
	string strFileSize;
	if (fileSize >= 1024 && fileSize < 1024 * 1024)
	{
		tmp = (double)fileSize / 1024;
		strFileSize = to_string(tmp);
		strFileSize = strFileSize.substr(0, strFileSize.find('.') + 3) + "KB";
	}
	else if (fileSize >= 1024 * 1024 && fileSize < 1024 * 1024 * 1024)
	{
		tmp = (double)fileSize / (1024 * 1024);
		strFileSize = to_string(tmp);
		strFileSize = strFileSize.substr(0, strFileSize.find('.') + 3) + "MB";
	}
	else if (fileSize >= 1024 * 1024 * 1024)
	{
		tmp = (double)fileSize / (1024 * 1024 * 1024);
		strFileSize = to_string(tmp);
		strFileSize = strFileSize.substr(0, strFileSize.find('.') + 3) + "GB";
	}
	else
	{
		strFileSize = to_string(fileSize);
		strFileSize = strFileSize.substr(0, strFileSize.find('.') + 3) + "B";
	}
	return strFileSize;
}

string mygetDownloadSpeed(long double speed)
{
	string strSpeed;
	if (speed >= 1024 && speed < 1024 * 1024)
	{
		speed = speed / 1024;
		strSpeed = to_string(speed);
		strSpeed = strSpeed.substr(0, strSpeed.find('.') + 2) + "KB/s";
	}
	else if (speed >= 1024 * 1024 && speed < 1024 * 1024 * 1024)
	{
		speed = speed / (1024 * 1024);
		strSpeed = to_string(speed);
		strSpeed = strSpeed.substr(0, strSpeed.find('.') + 2) + "MB/s";
	}
	else if (speed >= 1024 * 1024 * 1024)
	{
		speed = speed / (1024 * 1024 * 1024);
		strSpeed = to_string(speed);
		strSpeed = strSpeed.substr(0, strSpeed.find('.') + 2) + "GB/s";
	}
	else
	{
		strSpeed = to_string(speed);
		strSpeed = strSpeed.substr(0, strSpeed.find('.') + 2) + "B/s";
	}
	return strSpeed;
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
const int barlen = 70;
clock_t preTime = 0;
string bar(barlen, '.');        //进度条显示
string block(120, ' ');         //用于清屏一行
string lastSpeed;
double lastPercent;
/*****************/
/*****************/   //全局变量 由多线程使用
string multi_lastSpeed;
double multi_lastPercent; 
double multi_localFileSize[threadNum];     //已下载到本地的文件大小
double multi_totalLocalFileSize = 0;     //已下载到本地的文件大小
double multi_increaseLocalFileSize[threadNum];     //下载到本地的文件大小的增加量 用于计算速度
double multi_preFileSize[threadNum];
//clock_t multi_preTime[threadNum];
clock_t multi_preTime[threadNum];
start_end multi_piece[threadNum];      //记录每一段的起始字节和截止字节
double pieceLen[threadNum];      //每一段的长度
long pieceStart[threadNum];     //每一段起始的位置
//static int labelnum = 0;
//string label = "|/-\\";
/*****************/
void mygetPieceLen()
{
	long tmp;
	for (int i = 0; i < threadNum; i++)
	{
		tmp = multi_piece[i].endPonint - multi_piece[i].startPoint;
		//cout << "tmp::::::::::::::::::::::::::::::::" << multi_piece[i].startPoint<< endl;
		pieceLen[i] = (tmp*barlen) / helpFileSize;
		//cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << pieceLen[i] << endl;
	}
}
void mygetPieceStart()
{
	long tmp;
	for (int i = 0; i < threadNum; i++)
	{
		tmp = (multi_piece[i].startPoint / 10000)*barlen;
		//cout << "cheng" << tmp << endl;
		pieceStart[i] = tmp / (helpFileSize / 10000);
		cout << "star" << pieceStart[i] << endl;
	}
}
int single_progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	long double speed;
	string strSpeed;
	double dByte;
	double percent;
	clock_t nowTime = clock();
	long double dTime = (long double)(nowTime - preTime) / CLOCKS_PER_SEC; //单位为秒s
	int flag = 0;   //标识
	if (dlnow != preFileSize && dltotal != 0)     //刷新进度
	{
		percent = (dlnow / dltotal) * 100;
		lastPercent = percent;
		bar.replace(0, (percent * barlen) / 100, (percent * barlen) / 100, '#');
	}
	else
	{
		percent = lastPercent;
		flag++;
	}
	if (dTime >= 0.5)                //刷新速度
	{
		dByte = dlnow - preFileSize;
		speed = dByte / dTime;
		strSpeed = mygetDownloadSpeed(speed);

		lastSpeed = strSpeed;
		preTime = nowTime;
		preFileSize = dlnow;
	}
	else
	{
		strSpeed = lastSpeed;
		flag++;
	}
		
	if (flag == 2)
		return 0;
	string strDlownNow = mygetFileSize(dlnow);

	printf("%s\r", block.c_str());
	printf("[%s][%.2f%%][%s][%s/%s]\r", bar.c_str(), percent, strSpeed.c_str(), strDlownNow.c_str(), strFileSize.c_str());
	//记录下本次回调参数
	return 0;
}

void HttpDownloader::singleDown()
{
	CURLcode res;
	FILE *fp = fopen(fileAdress, "wb");
	//char *progress_data = NULL;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, urlAddress);
	//curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);  //是否可以访问
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);  //显示进度
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, single_progressCallback);

	//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeOut);  //不要设置超时时间
	res = curl_easy_perform(curl);
	cout << res << endl;
	if (!res)
	{
		long double downloadSzie;
		double speed;
		//cout << "here" << endl;
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &downloadSzie);
		curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
		cout << "downloaded size:" << downloadSzie << endl;
		cout << "average speed: " << speed << "bytes/sec" << " 100.00%" << endl;
	}
	curl_easy_cleanup(curl);
	fclose(fp);

}


int multi_progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	/*for (int i = 0; i < threadNum; i++)
	{
		mut1.lock();
		cout << "len" << i << ":" <<pieceStart[i] << endl;
		mut1.unlock();
	}*/
	//static int num = *(int*)clientp;        //num为对应的进程号
	//long double speed;
	//string strSpeed;
	//double dByte = 0;;
	//double rate;
	//clock_t nowTime = clock();
	//double percent;

	////mut1.lock();

	//long double dTime = (long double)(nowTime - multi_preTime[num]) / CLOCKS_PER_SEC; //单位为秒s
	//int flag = 0;     //标识
	//if (dlnow != multi_preFileSize[num] && dltotal != 0)    //刷新每一个进程段的进度
	//{
	//	mut1.lock();
	//	multi_localFileSize[num] = dlnow;
	//	mut1.unlock();
	//	rate = dlnow / dltotal;
	//	mut1.lock();
	//	bar.replace(pieceStart[num],
	//		rate * pieceLen[num], rate * pieceLen[num], '#');
	//	mut1.unlock();
	//	double sumFileSize = 0;
	//	for (int i = 0; i < threadNum; i++)
	//	{
	//		sumFileSize = sumFileSize + multi_localFileSize[i];
	//	}
	//	mut1.lock();
	//	multi_totalLocalFileSize = sumFileSize;
	//	mut1.unlock();
	//	/*multi_totalLocalFileSize = accumulate(multi_localFileSize,
	//		multi_localFileSize + threadNum, 0);*/
	//	percent = (multi_totalLocalFileSize / helpFileSize) * 100;
	//	//multi_preTotalLocalFileSize = multi_totalLocalFileSize;
	//	mut1.lock();
	//	multi_lastPercent = percent;
	//	mut1.unlock();
	//}
	//else
	//{
	//	percent = multi_lastPercent;
	//	flag++;
	//}
	//if (dTime >= 0.5)                //刷新速度
	//{
	//	mut1.lock();
	//	multi_increaseLocalFileSize[num] = dlnow - multi_preFileSize[num];
	//	mut1.unlock();
	//	double sumdByte = 0;
	//	for (int k = 0; k < threadNum; k++)
	//	{
	//		sumdByte = sumdByte + multi_increaseLocalFileSize[k];
	//	}
	//	dByte = sumdByte;
	//	/*dByte = accumulate(multi_increaseLocalFileSize,
	//		multi_increaseLocalFileSize + threadNum, 0);*/

	//	speed = dByte / dTime;
	//	strSpeed = mygetDownloadSpeed(speed);
	//	mut1.lock();
	//	multi_lastSpeed = strSpeed;
	//	mut1.unlock();
	//	mut1.lock();
	//	multi_preTime[num] = nowTime;
	//	mut1.unlock();
	//	mut1.lock();
	//	multi_preFileSize[num] = dlnow;
	//	mut1.unlock();
	//	
	//}
	//else
	//{
	//	strSpeed = multi_lastSpeed;
	//	flag++;
	//}

	//if (flag == 2)
	//	return 0;
	//mut1.lock();
	//string strDlownNow = mygetFileSize(multi_totalLocalFileSize);
	//mut1.unlock();
	//mut1.lock();
	//printf("%s\r", block.c_str());
	//printf("[%s][%.2f%%][%s][%s/%s]\r", bar.c_str(), percent, strSpeed.c_str(), strDlownNow.c_str(), strFileSize.c_str());
	//mut1.unlock();
	//////记录下本次回调参数
	//mut1.lock();
	//labelnum = (labelnum + 1) % 4;
	//printf("[%c]\r", label[labelnum]);
	//mut1.unlock();
	return 0;
}


start_end* HttpDownloader::getStartEnd()
{
	static start_end st_en[threadNum];
	long eachFileSize = fileSize / threadNum;
	//cout << "eachfilezie:::::::::::" << eachFileSize << endl;
	for (int i = 0; i < threadNum; i++)
	{
		st_en[i].startPoint = i * eachFileSize;
		multi_piece[i].startPoint = i * eachFileSize;
	}
		
	for (int j = 0; j < threadNum - 1; j++)
	{
		st_en[j].endPonint = st_en[j + 1].startPoint - 1;
		multi_piece[j].endPonint = st_en[j + 1].startPoint - 1;
		//cout << multi_piece[j].endPonint << endl;
	}
	st_en[threadNum - 1].endPonint = fileSize - 1;
	multi_piece[threadNum - 1].endPonint = fileSize - 1;
	/*for (int k = 0; k < 8; k++)
	{
		cout << st_en[k].startPoint << endl;
	}*/

	mygetPieceLen();                    //获取每段的长度，为进度条做准备
	mygetPieceStart();                //获取每段起始位置，为进度条做准备
	return st_en;

}

string HttpDownloader::creatTmpFile(int num)
{
	/*创建临时文件*/
	string tmpFileName;                 //生成临时文件的文件名
	string tmpFilePath;                   //生成临时文件的路径
	size_t adressLenth = strlen(fileAdress);
	size_t k = adressLenth;
	while (k > 0 && fileAdress[k] != '\\')
		k--;
	if (k == 0)
		cout << "**输入地址无效" << endl;
	else
	{
		string tmp = fileAdress;
		tmpFileName = &tmp[k + 1];
		tmpFilePath = tmp.substr(0, k + 1);
		//cout << tmpFilePath << endl;
		size_t i = tmpFileName.length();
		while (i > 0 && tmpFileName[i] != '.')
			i--;
		tmpFileName = tmpFileName.substr(0, i);
		tmpFileName = tmpFileName + "." + to_string(num) + "tmp";
	}
	//cout << tmpFileName << endl;
	string tmpfile = tmpFilePath + tmpFileName;     //临时文件路径
	tmpfileToMerge[num] = tmpfile;
	//cout << tmpfile << endl;
	return tmpfile;
}


void HttpDownloader::newDownladThread(long startpoint, long endpoint, string tmpfile, int num)
{
	/*下载该临时文件*/

	CURL *sub_curl = curl_easy_init();
	if (sub_curl)
	{
		curl_slist *headers = NULL;
		CURLcode res;
		int progress_data = num;
		FILE *sub_fp = fopen(tmpfile.c_str(), "wb");
		string requrRange = "Range: bytes=" + to_string(startpoint) + "-" + to_string(endpoint);
		//cout << "requrRange:  " << requrRange << endl;
		//cout << "size::" << endpoint - startpoint << endl;
		
		headers = curl_slist_append(headers, requrRange.c_str());
		curl_easy_setopt(sub_curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(sub_curl, CURLOPT_URL, urlAddress);
		/*curl_easy_setopt(sub_curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(sub_curl, CURLOPT_NOBODY, 1L);*/
		curl_easy_setopt(sub_curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(sub_curl, CURLOPT_WRITEDATA, sub_fp);
		curl_easy_setopt(sub_curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(sub_curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(sub_curl, CURLOPT_PROGRESSDATA, &progress_data);
		curl_easy_setopt(sub_curl, CURLOPT_PROGRESSFUNCTION, multi_progressCallback);
		//curl_easy_setopt(sub_curl, CURLOPT_VERBOSE, 1L);

		//mut.lock();
		res = curl_easy_perform(sub_curl);
		//mut.unlock();

		if (res)
		{
			cout << "**线程" << num << "写文件失败" << endl;
		}
		else
		{
			//mut.lock();
			downloadSuccess[num] = true;
			//mut.unlock();
			cout << "**线程" << num << "写文件成功" << endl;
			
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

bool HttpDownloader::mergeTempFile()
{
	//FILE *mergefile = fopen(fileAdress, "ab+");
	char *buffer[threadNum];
	long fileLengh[threadNum];
	for (int i = 0; i < threadNum; i++)
	{
		FILE *sub_file = fopen(tmpfileToMerge[i].c_str(), "rb");
		if (sub_file == NULL)
		{
			cout << "**打开临时文件" << i << "失败" << endl;
			return false;
		}
		fseek(sub_file, 0, SEEK_END);
		fileLengh[i] = ftell(sub_file);
		//cout << "长度：" << fileLengh[i] << endl;
		rewind(sub_file);                 //回退！！！！！！
		buffer[i] = new char[fileLengh[i]];
		fread(buffer[i], fileLengh[i], 1, sub_file);
		fclose(sub_file);
		//cout << "这是buffer->:" << buffer[i] << endl;
	}
	FILE *mergefile = fopen(fileAdress, "wb");
	for (int k = 0; k < threadNum; k++)
	{
		//cout << fileLengh[k] << endl;
		if (fwrite(buffer[k], fileLengh[k], 1, mergefile) == 1)
			continue;
		else
		{
			cout << "**合并临时文件" << k << "出错";
			return false;
		}
	}
	fclose(mergefile);
	for (int j = 0; j < threadNum; j++)
		delete[]buffer[j];

	return true;
}
