/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>

int main(int argc, char *argv[])
{
	//clock_t starttime = clock();         //放在类的初始化里出问题？？？
	curl_global_init(CURL_GLOBAL_ALL);
	//HttpDownloader downloader("http://selenium-release.storage.googleapis.com/3.0-beta1/selenium-server-standalone-3.0.0-beta1.jar","D:\\Download\\testDownloader.jar");
	HttpDownloader downloader(argv[1], argv[2]);
	if (downloader.getConnectable() && downloader.getAdressable())
	{
		cout << "**连接服务器成功" << endl;
		if (downloader.getResumable())   //分段下载
		{
			cout << "**支持断点续传" << endl;
			start_end *tmp;
			tmp = downloader.getStartEnd();
			for (int i = 0; i < threadNum; i++)
			{
				//cout << i << "start::" << (tmp + i)->startPoint << endl;
				//cout << i << "end::" << (tmp + i)->endPonint << endl;
				string tmpfile = downloader.creatTmpFile(i);
				thread anThread(&HttpDownloader::newDownladThread, downloader, (tmp + i)->startPoint, (tmp + i)->endPonint, tmpfile, i);
				anThread.detach();
			}
			while (true)
			{
				if (downloader.threadMonitor())
					break;
				else
					continue;
			}
			int k = 0;
			for (k = 0; k < 3; k++)           //尝试合并文件 三次
			{
				if (downloader.mergeTempFile())
					break;
				else
					continue;
			}

			if (k == 3)
			{
				cout << endl;
				cout << "**文件合并失败" << endl;
			}
			else
			{
				cout << endl;
				cout << "**文件下载成功" << endl;
				downloader.cleanTempFile();
			}
		}
		else
		{
			cout << "**不支持断点续传" << endl;
			downloader.singleDown();
		}
	}
	else
		cout << "**无法下载该文件" << endl;
	//clock_t endtime = clock();
	cout << endl;
	//cout << "Total time: " << (endtime - starttime) / CLOCKS_PER_SEC << "sec" << endl;
	curl_global_cleanup();
	return 0;
}
