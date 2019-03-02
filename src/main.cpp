/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>

int main()
{

	HttpDownloader downloader("http://dl.cnponer.com/PanDownload_v2.0.6.zip","D:\\Download\\testDownloader.zip");
	if (downloader.supportResumeDownload())
	{
		cout << "**支持断点续传" << endl;
		downloader.startDownloader();
	}
	else
	{
		cout << "**不支持断点续传" << endl;
		downloader.startDownloader();
	}
	return 0;
}