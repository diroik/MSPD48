/*
 * CommPort.h
 *
 *  Created on: Sep 25, 2014
 *      Author: user
 */

#ifndef COMMPORT_H_
#define COMMPORT_H_

#include <sys/param.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <termios.h>

#include <Log.h>
#include <Classes.h>

using namespace std;
using namespace log4net;

	class ComPort:public IPort
	{
	public:
	ComPort();
		virtual ~ComPort();
		void Init(void *settings);

		bool ConnectOK(void);

		bool ConnectOK(int timeout);

		void Reconnect(int timeout = 0);

		bool Close(void);

		bool IsOpenned(void);

		bool IsConnected(void);

		sWord Send(Byte *Buf, Word Len);

		sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol);

		sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol, Byte cnt);

		sWord Recv(Byte *Buf, Word Len);

		sWord Recv(Byte *Buf, Word Len, Word MaxLen);

		int GetDescriptor()
		{
			return port;
		}

		string GetSettingsString(void)
		{
			string ret = "";
			ret+="[DeviceName="+settings.DeviceName+";";
			ret+="BaudRate="+toString(settings.BaudRate)+";";
			ret+="DataBits="+toString((int)settings.DataBits)+";";
			ret+="StopBit="+toString((int)settings.StopBit)+";";
			ret+="Parity="+toString((int)settings.Parity)+";";
			ret+="RecvTimeout="+toString(settings.RecvTimeout)+";";
			ret+="SendTimeout="+toString(settings.SendTimeout)+"]";
			return ret;
		}

		int GetBytesInBuffer(void);

		CommSettings settings;

		bool PortIsLAN()
		{
			return false;
		}

	private:
		int port;
		//pthread_mutex_t sych;
		//string portname;
		//Word baudrate;
		//Word recvTimeout;
		//struct timeval sendTimeout;
		bool isOpenned;

		unsigned int GetSpeed(unsigned int speed);
	};

#endif /* COMMPORT_H_ */
