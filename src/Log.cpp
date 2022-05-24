/*
 * Log.cpp
 *
 *  Created on: Sep 18, 2014
 *      Author: user
 */

#include <Log.h>
//#include <syslog.h>

namespace log4net
{

	void Log::Configure(string name)
	{
		debug_flag = false;
		processName = name;
		openlog(processName.c_str(), LOG_PID | LOG_CONS, LOG_USER);
	}
	void Log::INFO(string mess)
	{
		syslog(LOG_INFO, mess.c_str());
		printf("INFO: %s\n", mess.c_str());
	}
	void Log::DEBUG(string mess)
	{
		if(debug_flag)
		{
			syslog(LOG_DEBUG, mess.c_str());
			printf("DEBUG: %s\n", mess.c_str());
		}
	}
	void Log::ERROR(string mess)
	{
		syslog(LOG_ERR, mess.c_str());
		printf("ERROR: %s\n", mess.c_str());
	}

	string Log::processName;

	bool Log::debug_flag;



} /* namespace log4net */
