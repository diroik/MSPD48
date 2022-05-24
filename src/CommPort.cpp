/*
 * CommPort.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: user
 */

#include <CommPort.h>


	ComPort::ComPort() {
		port = -1;
		isOpenned = false;
		settings.BaudRate = 38400;
		settings.RecvTimeout = 1000;
	}

	ComPort::~ComPort() {
		// TODO Auto-generated destructor stub
	}

	void ComPort::Init(void *portSettings)
	{
		try
		{
			//Log::INFO( "initCommPort" );

			CommSettings *conf = (CommSettings *)portSettings;
			settings = *conf;
			/*
			Log::DEBUG( "[ComPort::Init] DeviceName="+settings.DeviceName+
					";\r\nBaudRate="+toString(settings.BaudRate)+
					";\r\nSendTimeout="+toString(settings.SendTimeout)+
					";\r\nRecvTimeout="+toString(settings.RecvTimeout)+
					";\r\nDataBits="+toString((int)settings.DataBits)+
					";\r\nStopBit="+toString((int)settings.StopBit)+
					";\r\nParity="+toString((int)settings.Parity));
*/
			//portname = conf->DeviceName;
			//baudrate = conf->BaudRate;
			//recvTimeout = conf->RecvTimeout;
			//sendTimeout.tv_sec = 0;

		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isOpenned = false;
		}
	}

	bool ComPort::ConnectOK(void)
	{
		if(!IsOpenned())
		{
			//Log::DEBUG( "Is not connected Reopen port="+settings.DeviceName );
			Reconnect();
			sleep(1);
		}
		return IsConnected();
	}

	bool ComPort::ConnectOK(int timeout)
	{
		if(!IsOpenned())
		{
			Reconnect(timeout);
			sleep(1);
		}
		return IsConnected();
	}

	bool ComPort::IsOpenned(void)
	{
		bool ret;
		//pthread_mutex_lock(&sych);
		//ret = isOpenned;
		ret = isvirtual == true ? isOpenned :  (isOpenned && port >=0);
		//pthread_mutex_unlock(&sych);
		return ret;
	}

	bool ComPort::IsConnected(void)
	{
		return IsOpenned();
	}

	void ComPort::Reconnect(int timeout)
	{
		//pthread_mutex_lock(&sych);
		try
		{
			//Log::DEBUG( "[CommPort] open CommPort: "+settings.DeviceName + " Speed: " + toString(settings.BaudRate) );
			if(port >= 0 || isOpenned == true)
			{// || isOpenned == true){
				Log::DEBUG( "[CommPort] Reconnect port="+toString(port)+" isOpenned="+toString(isOpenned));
				Close();
			}
			if(!isvirtual)
			{
				port = open(settings.DeviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
				if(port >= 0)
				{
					//Log::DEBUG( "[CommPort] port="+toString(port)+" DeviceName="+settings.DeviceName+" BaudRate="+toString(settings.BaudRate));
					fcntl(port, F_SETFL, 0);
					struct termios options;
					tcgetattr(port, &options);
					cfsetispeed(&options, GetSpeed(settings.BaudRate));
					cfsetospeed(&options, GetSpeed(settings.BaudRate));
					options.c_cflag |= (CLOCAL | CREAD);
					switch(settings.Parity)
					{
					case PARITY_NONE:
						options.c_cflag &= ~PARENB; //disable parity
						break;
					case PARITY_EVEN:
						options.c_cflag |= PARENB; //even parity
						options.c_cflag &= ~PARODD;
						//Log::DEBUG( "[CommPort] PARITY_EVEN");
						break;
					case PARITY_ODD:
						options.c_cflag |= PARENB; //odd parity
						options.c_cflag |= PARODD;
						break;

					};
					options.c_cflag &= ~CSTOPB; //disable 2 stopbit - 1 stopbit enable
					options.c_cflag &= ~CSIZE; //disable bitmask

					switch(settings.DataBits)
					{
					case 5:
						options.c_cflag |= CS5; //5 databit
						break;
					case 6:
						options.c_cflag |= CS6; //6 databit
						break;
					case 7:
						options.c_cflag |= CS7; //7 databit
						//Log::DEBUG( "[CommPort] CS7");
						break;
					case 8:
						options.c_cflag |= CS8; //8 databit
						break;
					};
					options.c_cflag &= ~CRTSCTS;/* flow control disable*/
					options.c_lflag &= ~(ICANON | ECHOE | ECHO | ISIG);
					options.c_iflag = 0;
					//options.c_iflag &= ~(IXON | IXOFF | IXANY);
					options.c_oflag &= ~OPOST;
					//options.c_oflag |= ONLCR;
					options.c_cc[VMIN] = 0;
					options.c_cc[VTIME] = settings.RecvTimeout/100;// timeout - (10 = 1 sek)
					//Log::INFO( "[CommPort] timeout is: " + toString((int)options.c_cc[VTIME]) );
					tcflush(port, TCIOFLUSH);
					tcsetattr(port, TCSANOW, &options);
					//Log::INFO( "[CommPort] open commport OK port="+settings.DeviceName+" BaudRate="+toString(settings.BaudRate) );
					isOpenned = true;
					//Log::DEBUG( "[CommPort] open port="+toString(port)+" isOpenned="+toString(isOpenned));

					//Log::DEBUG( "[CommPort] open commport="+settings.DeviceName + " Speed=" + toString(settings.BaudRate)+ " port="+toString(port) );
				}
				else
				{
					Log::ERROR( "[CommPort] open commport failed! port = "+toString(port) );
					isOpenned = false;
					port = -1;
				}
			}
			else
			{
				isOpenned = true;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "error in ComPort::Reconnect"+string(e.what()) );
		}
		//pthread_mutex_unlock(&sych);
	}

	bool ComPort::Close(void)
	{
		try
		{
			if(!isvirtual)
			{
				//Log::DEBUG( "[CommPort] close commport "+settings.DeviceName+" BaudRate="+toString(settings.BaudRate)+" port="+toString(port) );
				if( port >= 0)
				{
					//tcflush(port, TCIFLUSH);
					tcflush(port, TCIOFLUSH);
					close(port);
				}
				port = -1;
			}
			isOpenned = false;
		}
		catch(exception &e)
		{
			Log::ERROR( "error in ComPort::Close port="+settings.DeviceName+" error="+string(e.what()) );
			return false;
		}
		return true;

	}

	sWord ComPort::Send(Byte *Buf, Word Len)
	{
		if(Len == 0) return 0;
		int len = 0;
		try
		{
			if(isvirtual)return Len;//!!!
			//flush();
			//ioctl(port, TCFLSH);
			len = write(port, Buf, Len);
			if(len > 0 )
			{
				//Log::DEBUG( "[CommPort] send comm len ="+toString(len));
				isOpenned = true;
			}
			else
			{
				//Log::DEBUG( "[CommPort] send comm err");
				isOpenned = false;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isOpenned = false;
			len = 0;
		}
		return len;
	}

	sWord ComPort::RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol)
	{
		int len = 0;
		try
		{
			char rb[1] {0};
            int bytes;
            isOpenned = true;
            do
            {
                bytes = read(port, rb, 1);
    			if(bytes == 1)
    			{//ok
    				Buf[len++] = rb[0];
    				if(len >= MaxLen) break;
    			}
    			else
    			{
    				if(bytes < 0)
    				{// disconnect
    					isOpenned = false;
    				}
    				break;
    			}
            }
            while ( rb[0] != StopSymbol /*0x03*/ );
            if(len > 0)
            {
            	//Log::DEBUG( "[CommPort] recvto ok: ["+ string( (char*)Buf, len ) + "] len="+toString(len));
            }
		}
		catch(exception &e)
		{
			Log::ERROR( "ComPort::RecvTo: " + string(e.what()) );
			isOpenned = false;
			len = 0;
		}
		return len;
	}

	sWord ComPort::RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol, Byte cnt)
	{
		int len = 0;
		int bytes = 0;
		try
		{
			do
			{
				if(len >= MaxLen) break;
				bytes = RecvTo(Buf,MaxLen,StopSymbol);
				if(bytes > 0)
				{
					len+=bytes;
					Buf +=bytes;
				}
				else
				{
					break;
				}
			}
			while(cnt--);
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isOpenned = false;
			len = 0;
		}
		return len;
	}

	sWord ComPort::Recv(Byte *Buf, Word Len)
	{
		if(Len == 0) return 0;
		int len = 0;
		try
		{
			char rb[1] {0};
            int bytes;
            isOpenned = true;
            do
            {
                bytes = read(port, rb, 1);
    			if(bytes == 1)
    			{//ok
    				Buf[len++] = rb[0];
    			}
    			else
    			{
    				if(bytes < 0)
    				{// disconnect
    					isOpenned = false;
    				}
    				break;
    			}
            }
            while ( --Len);
            if(len > 0)
            {
            	//Log::DEBUG( "[CommPort] recvto ok: ["+ string( (char*)Buf, len ) + "] len="+toString(len));
            }
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isOpenned = false;
			len = 0;
		}
		return len;
	}

	sWord ComPort::Recv(Byte *Buf, Word Len, Word MaxLen)
	{
		if(Len == 0) return 0;
		int len = 0;
		try
		{
			if(Len > MaxLen) Len = MaxLen;
			len = read(port, Buf, Len);
			isOpenned = true;
			if(len > 0)
			{
				//Log::DEBUG( "recv: "+ string( (char*)Buf, len ) );
			}
			else
			{
				len = 0;
				isOpenned = false;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isOpenned = false;
			len = 0;
		}
		return len;
	}

	unsigned int ComPort::GetSpeed(unsigned int speed)
	{
		switch(speed)
		{
		case 200: return B200;
		case 300: return B300;
		case 600: return  B600;
		case 1200:return  B1200;
		case 1800:return  B1800;
		case 2400:return  B2400;
		case 4800:return  B4800;
		case 9600:return  B9600;
		case 19200:return  B19200;
		case 38400:return  B38400;
		case 57600:return  B57600;
		case 115200:return  B115200;
		case 230400:return  B230400;
		case 460800:return  B460800;
		default: return B38400;
		}
	}


	int ComPort::GetBytesInBuffer(void)
	{
		int fd;
		int bytes;

		ioctl(port, FIONREAD, &bytes);

		return bytes;
	}


