/*
 * ModbusRTU.cpp
 *
 *  Created on: May 7, 2015
 *      Author: user
 */

#include <ModbusRTU.h>

TModbusManager::TModbusManager():TFastTimer(1,&SecCount) {
	// TODO Auto-generated constructor stub
	_isMaster 			= false;
	_myaddress 			= 254;
	OnCmdComplited 		= NULL;
	//NeedReloadConfig.SetData(false);
	IsStopped.SetData(false);
	//outPtr = NULL;
	_need_pop_back.SetData(false);
}

TModbusManager::~TModbusManager() {
	// TODO Auto-generated destructor stub
}

