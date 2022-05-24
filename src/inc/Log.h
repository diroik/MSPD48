/*
 * Log.h
 *
 *  Created on: Sep 18, 2014
 *      Author: user
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <syslog.h>
#include <iostream>
#include <stdio.h>

using namespace std;
namespace log4net {

	class Log
	{
	public:
		static void Configure(string name);

		//static void INFO(char *  mess);
		//static void DEBUG(char * mess);
		//static void ERROR(char * mess);

		static void INFO(string mess);
		static void DEBUG(string mess);
		static void ERROR(string mess);

		//static bool info_flag;
		static bool debug_flag;


	private:
		static string processName;
	};

} /* namespace log4net */

#endif /* LOG_H_ */
