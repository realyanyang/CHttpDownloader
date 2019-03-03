/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <thread>

int main()
{

	HttpDownloader downloader("https://files.pythonhosted.org/packages/36/fa/51ca4d57392e2f69397cd6e5af23da2a8d37884a605f9e3f2d3bfdc48397/pip-19.0.3.tar.gz","D:\\Download\\testDownloader.gz");
	//downloader.startDownloader();
	downloader.singleDown();
	//downloader.newDownladThread(0, 276, 0);
	return 0;
}
