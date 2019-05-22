#include <conn/cms_conn_common.h>
#include <common/cms_url.h>
#include <common/cms_utility.h>

std::string makeHlsUrl(std::string oriUrl)
{
	LinkUrl linkUrl;
	parseUrl(oriUrl, linkUrl);
	std::string hlsUrl;
	hlsUrl = "http://" + linkUrl.host;
	hlsUrl += "/" + linkUrl.app;
	hlsUrl += "/";
	std::size_t pos = linkUrl.instanceName.find("?");
	if (pos != std::string::npos)
	{
		hlsUrl += linkUrl.instanceName.substr(0, pos);
	}
	else
	{
		hlsUrl += linkUrl.instanceName;
	}
	hlsUrl += "/online.m3u8";
	return hlsUrl;
}