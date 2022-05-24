/*
 * VirtualPort.h
 *
 *  Created on: Nov 14, 2014
 *      Author: user
 */

#ifndef VIRTUALPORT_H_
#define VIRTUALPORT_H_

#include <vector>
#include <Log.h>
#include <Classes.h>
#include <SocketPort.h>
#include <CommPort.h>
#include <uci.h>

using namespace std;
using namespace log4net;

class VirtualPortSettings
{
	public:
	IPort      *Port;

	CommSettings  settings;
    string        portselect;
    string        portstate;

    void InitGPIO( )
    {
     if(portselect.size() > 0 && portstate.size() > 0)
     {
    	 string sendStr = "(echo "+portselect+" > /sys/class/gpio/export) >& /dev/null";
    	 sendToConsole(sendStr);
    	 sendStr = "echo out > /sys/class/gpio/gpio"+portselect+"/direction";
    	 sendToConsole(sendStr);
    	 sendStr = "echo "+portstate+" > /sys/class/gpio/gpio"+portselect+"/value";
    	 sendToConsole(sendStr);
     }
    }
};


class VirtualPort:public TFastTimer
{
public:
	VirtualPort();

	virtual ~VirtualPort();

	void Init(void *cfg1, void *cfg2);

	void ReloadConfig(void);

	void DoWork(void);

	bool IsRuning();

	void SetIsRuning(bool new_val);

	static void *TransportData(void *data, void *p2 = NULL);

	VirtualPortSettings* GetSett()
	{
		VirtualPortSettings* ret = NULL;

		int ind = PortIndex - 1;
		if(!PortList.empty() )
		{
			if(ind >= PortList.size())
				ind = PortList.size() - 1;
			ret = PortList[ind];
		}


		return ret;
	}


	sync_deque<VirtualPortSettings*> 	PortList;
	ServerPortUdp 				 		SocketPort;

private:

	Word             PortIndex;
	pthread_mutex_t  sych;
	bool 			 isRuning;
	int  			_timeout;
	int             _recv_timeout;
};

//extern VirtualPort        VSP;

#endif /* VIRTUALPORT_H_ */
