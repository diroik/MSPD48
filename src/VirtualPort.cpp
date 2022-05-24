/*
 * VirtualPort.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: user
 */

#include <VirtualPort.h>


VirtualPort::VirtualPort():TFastTimer(2,&SecCount), isRuning(false), _timeout(30), PortIndex(0)
{
	_recv_timeout = 0;
	//VSPptr = NULL;
	// TODO Auto-generated constructor stub

}

VirtualPort::~VirtualPort()
{
	// TODO Auto-generated destructor stub
}


bool VirtualPort::IsRuning(void)
{
	bool ret;
	pthread_mutex_lock(&sych);
	ret = isRuning;
	pthread_mutex_unlock(&sych);
	return ret;
}

void VirtualPort::SetIsRuning(bool new_val)
{
	pthread_mutex_lock(&sych);
	isRuning = new_val;
	pthread_mutex_unlock(&sych);
}

void VirtualPort::Init(void *cfg1, void *cfg2)
{
	Log::INFO( "[VirtualPort]::init" );

	Word *conf1 = (Word *)cfg1;
	_timeout = *conf1;

	DWord *conf2 = (DWord *)cfg2;
	_recv_timeout = (Word)*conf2;

	//Log::INFO( "[VirtualPort] _timeout=" + toString(_timeout) );
}

void *VirtualPort::TransportData(void *data, void *ptr)
{
	TBuffer *retBuf = NULL;
	try
	{
		//Log::INFO("VirtualPort::TransportData start");
		if(data != NULL && ptr != NULL)
		{
			TBuffer 	*buf =  (TBuffer *)data;
			VirtualPort *VSP = (VirtualPort *)ptr;
			VirtualPortSettings *curr_sett = VSP->GetSett();

			if( curr_sett != NULL)
			{

				if( curr_sett->Port->ConnectOK(VSP->_recv_timeout) )
				{
					curr_sett->InitGPIO();
					retBuf = new TBuffer(2048);
					int len = curr_sett->Port->Send(buf->BUF, buf->DataLen);
					//Log::DEBUG( "[VirtualPort::TransportData] Send len="+toString(len));
					if(len > 0)
					{
						retBuf->DataLen = curr_sett->Port->Recv( retBuf->BUF, retBuf->MaxSize );
						//Log::DEBUG( "[VirtualPort::TransportData] Recv len="+toString(retBuf->DataLen));
					}
					curr_sett->Port->Close();
				}
			}
			else
			{
				Log::DEBUG( "[VirtualPort::TransportData] curr_sett == NULL");
			}
			VSP->SetTimer(0, VSP->_timeout);
		}

	}
	catch(exception &e)
	{
		Log::ERROR( "[VirtualPort::TransportData]" + string( e.what() ) );
	}
	//Log::INFO("VirtualPort::TransportData end");
	return retBuf;
}

void VirtualPort::ReloadConfig()
{
	//pthread_mutex_lock(&sych);
	try
	{
		bool needEnable = false;
		string vsp = uci::Get(CONFIG_FILE, "vsp", "needenable");
		if(vsp.size() > 0)
			needEnable = atoi(vsp.c_str());
		if(needEnable)
		{
			string sportindex  = uci::Get(CONFIG_FILE, "vsp", "sportindex");
			if(sportindex.size() > 0)
				PortIndex = atoi(sportindex.c_str());

			uci::Set(CONFIG_FILE, "vsp", "needenable", "0");
			isRuning = true;
			SetTimer(0, _timeout);//timer 10 min
			Log::DEBUG("[VirtualPort] needEnable=true");
		}
		else
		{
			bool needStop = false;
			string vsp = uci::Get(CONFIG_FILE, "vsp", "needstop");
			if(vsp.size() > 0)
				needStop = atoi(vsp.c_str());
			if(needStop)
			{
				uci::Set(CONFIG_FILE, "vsp", "needstop", "0");
				isRuning = false;
				Log::DEBUG("[VirtualPort] needstop=true");
			}
		}
		string timeout = uci::Get(CONFIG_FILE, "vsp", "sendtimeout");
		if(timeout.size() > 0)
		{
			_recv_timeout = atoi(timeout.c_str());
		}

	}
	catch(exception &e)
	{
		Log::ERROR( "[VirtualPort::ReloadConfig]" + string( e.what() ) );
	}
	//pthread_mutex_unlock(&sych);
}

void VirtualPort::DoWork()
{
	try
	{
		if( this->GetTimValue(0) <= 0 )
		{
			if(IsRuning())
			{
				SetIsRuning( false );
				Log::DEBUG("[VirtualPort] isRuning = false");
			}
		}

		if(this->IsRuning())
		{
			SocketPort.Start();
		}
		else
		{
			SocketPort.Stop();
			//Log::DEBUG("[VirtualPort] timeout stop");
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[VirtualPort::DoWork]" + string( e.what() ) );
	}
}
//=============================================================================

