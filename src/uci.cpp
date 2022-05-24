/*
 * uci.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: user
 */

#include <uci.h>

 string uci::Get(string file,string config, string option)
 {
	 	string ret;
		FILE *fp;
		char buffer[500];
		bzero(buffer, sizeof(buffer));
		string url = "uci get "+file+"."+config+"."+option;
		fp=popen( url.c_str(), "r");
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		ret = string(buffer);
		if(ret.size()>0) ret = ret.erase(ret.size()-1, 1);

		//printf("DEBUG: %s\n", url.c_str());
		//printf("DEBUG: %s\n", ret.c_str());
	 return ret;
 }


 string uci::Get(string file,string config)
 {
	 	string ret;
		FILE *fp;
		char buffer[5000];
		bzero(buffer, sizeof(buffer));
		string url = "uci get "+file+"."+config;
		fp=popen( url.c_str(), "r");
		fgets(buffer, sizeof(buffer), fp);
		pclose(fp);
		ret = string(buffer);
		if(ret.size()>0) ret = ret.erase(ret.size()-1, 1);

		//printf("DEBUG: %s\n", url.c_str());
		//printf("DEBUG: %s\n", ret.c_str());
	 return ret;
 }

 bool uci::Set(string file,string config, string option, string value)
 {
	 bool ret = true;
	 FILE *fp;

		string url = "uci set "+file+"."+config+"."+option+"="+value;
		fp=popen( url.c_str(), "w");
		//printf("DEBUG: %s\n", url.c_str());
		//printf("DEBUG: fp=%d\n", fp);
		if(fp == NULL) return false;
		pclose(fp);

		//bzero(buffer, sizeof(buffer));
		url = "uci commit";
		fp=popen( url.c_str(), "w");
		//printf("DEBUG: %s\n", url.c_str());
		//printf("DEBUG: fp=%d\n", fp);
		if(fp == NULL) return false;
		pclose(fp);
		return ret;
 }

