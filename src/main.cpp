/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>

int main()
{

	HttpDownloader downloader("http://selenium-release.storage.googleapis.com/2.39/selenium-server-2.39.0.zip","D:\\Download\\testDownloader.zip");
	//char *name = NULL;
	//downloader.getFileName(name);
	downloader.startDownloader();
	////cout << name << endl;
	//return 0;
}
