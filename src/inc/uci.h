/*
 * uci.h
 *
 *  Created on: Sep 25, 2014
 *      Author: user
 */

#ifndef UCI_H_
#define UCI_H_

#include <string.h>
#include <syslog.h>
#include <iostream>
#include <stdio.h>


using namespace std;

class uci
{
public:
		static string 	Get(string file, string config, string option);
		static string   Get(string file,string config);
		static bool 	Set(string file, string config, string option, string value);

};

#endif /* UCI_H_ */
