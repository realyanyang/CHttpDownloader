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

	HttpDownloader downloader("http://selenium-release.storage.googleapis.com/2.50/selenium-java-2.50.0.zip","D:\\Download\\testDownloader.gz");
	//downloader.startDownloader();
	//downloader.singleDown();
	//downloader.newDownladThread(0, 276, 0);
	start_end *tmp;
	tmp = downloader.getStartEnd();
	for (int i = 0; i < threadNum; i++)
	{
		/*cout << i << "start::" << (tmp + i)->startPoint << endl;
		cout << i << "end::" << (tmp + i)->endPonint << endl;*/
		thread anThread(&HttpDownloader::newDownladThread, downloader, (tmp + i)->startPoint, (tmp + i)->endPonint, i);
		anThread.detach();
	}
	while (true)
	{
		if (downloader.threadMonitor())
			break;
		else
			continue;
	}
		
	return 0;
}
