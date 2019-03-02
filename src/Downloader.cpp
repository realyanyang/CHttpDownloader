/*
 * ----------------------------------------------------
 * Copyright (c) 2019, Yan Yang. All Rights Reserved.
 * ----------------------------------------------------
 */
#include "Downloader.h"
#include <stdio.h>
#include <stdlib.h>

HttpDownloader::HttpDownloader(const char url[], const char file[])
{
	curl_global_init(CURL_GLOBAL_ALL);
	urlAddress = url;
	fileAdress = file;
}
HttpDownloader::~HttpDownloader()
{
	curl_global_cleanup();
}

bool HttpDownloader::supportResumeDownload()
{
	curl = curl_easy_init();
	if (curl)
	{
		//设置curl选项.
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Range: bytes=0-");
		curl_easy_setopt(curl, CURLOPT_URL, urlAddress);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_NOBODY, true);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeOut);
		//调用curl_easy_perform 执行.并进行相关的操作.
		res = curl_easy_perform(curl);
		long response_code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		//cout << response_code << endl;
		//清除curl操作.
		curl_easy_cleanup(curl);
		if (response_code == 206)
		{
			resumable = true;
			return true;
		}
		else
		{
			resumable = false;
			return false;
		}
	}
	else
	{
		resumable = false;
		return false;
	}
}

size_t write_data(char *buffer, size_t size, size_t nmemb, void* stream)
{
	size_t written = fwrite(buffer, size, nmemb, (FILE*)stream);
	if (written != nmemb)
	{
		return written;
	}
	return size * nmemb;
}

void HttpDownloader::startDownloader()
{
	FILE *fp = fopen(fileAdress, "wb");
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, urlAddress);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeOut);
	curl_easy_perform(curl);
	//fclose(fp);
	curl_easy_cleanup(curl);
	fclose(fp);
}