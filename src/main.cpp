/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>


//bool downloadSuccess[threadNum] = { 0 };

int main()
{
	clock_t starttime = clock();

	HttpDownloader downloader("http://selenium-release.storage.googleapis.com/3.0-beta2/selenium-server-standalone-3.0.0-beta2.jar","D:\\Download\\testDownloader.jar");
	if (downloader.getResumable())   //分段下载
	{
		start_end *tmp;
		tmp = downloader.getStartEnd();
		for (int i = 0; i < threadNum; i++)
		{
			/*cout << i << "start::" << (tmp + i)->startPoint << endl;
			cout << i << "end::" << (tmp + i)->endPonint << endl;*/
			string tmpfile = downloader.creatTmpFile((tmp + i)->startPoint, (tmp + i)->endPonint, i);
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
	cout << "TIME:::::" << endtime - starttime << endl;
	//Sleep(1000);
	return 0;
}
