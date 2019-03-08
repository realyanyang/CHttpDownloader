/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>


int main()
{
	clock_t starttime = clock();         //放在类的初始化里出问题？？？
	curl_global_init(CURL_GLOBAL_ALL);
	HttpDownloader downloader("https://files.pythonhosted.org/packages/36/fa/51ca4d57392e2f69397cd6e5af23da2a8d37884a605f9e3f2d3bfdc48397/pip-19.0.3.tar.gz","D:\\Download\\testDownloader.gz");
	if (downloader.getResumable())   //分段下载
	{
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
		downloader.mergeTempFile();
	}
	else
		downloader.singleDown();
	//downloader.singleDown();
	clock_t endtime = clock();
	cout << endl;
	cout << "TIME:::::" << endtime - starttime << endl;
	//Sleep(1000);
	curl_global_cleanup();
	return 0;
}
