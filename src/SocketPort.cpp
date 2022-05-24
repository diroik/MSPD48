/*
 * SocketPort.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: user
 */

#include <SocketPort.h>

	//=======================================
	//*** SocketPortUdp class ******************
	//=======================================
	SocketPortUdp::SocketPortUdp()
	{
		sock = 0;
		isConnected = false;
		nodelayFlag  = 1;
	}

	SocketPortUdp::SocketPortUdp(int s, struct sockaddr_in recv_addr, struct sockaddr_in snd_addr, struct timeval s_tim)
	{
		sock 			= s;
		recvTimeout 	= s_tim;
		bind_addr		= recv_addr;
		send_addr		= snd_addr;
		nodelayFlag 	= 1;

		int flags = fcntl(sock, F_GETFL, 0);
		fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);

		int stat = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval*)&recvTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
		if(stat < 0)
			throw runtime_error("SO_RCVTIMEO error!");

		//Log::DEBUG( "[SocketPortUdp::SocketPort]  tv_sec="+toString(recvTimeout.tv_sec) + " tv_usec="+toString(recvTimeout.tv_usec));

		isConnected = true;
		//Log::INFO( "[SocketPortUdp::SocketPort]  is created" );


	}

	SocketPortUdp::~SocketPortUdp()
	{

	}

	void SocketPortUdp::Init(void *sockSettings)
	{
		try
		{
			Log::INFO( "[SocketPortUdp::Init start]" );

			SocketSettings *conf = (SocketSettings *)sockSettings;

			bzero(&send_addr, sizeof(send_addr));
			send_addr.sin_family 		= AF_INET;
			send_addr.sin_port 			= htons(conf->IpPort);//htons(9002);
			send_addr.sin_addr.s_addr 	= inet_addr(conf->IpAddress.c_str());//inet_addr("192.168.1.10");//gethostbyname

			bzero(&bind_addr, sizeof(bind_addr));
			bind_addr.sin_family 		= AF_INET;
			bind_addr.sin_port 			= conf->BindPort > 0 ?
					htons(conf->BindPort): htons(conf->IpPort);
			bind_addr.sin_addr.s_addr 	= htonl(INADDR_ANY);

			Log::INFO( "[SocketPortUdp] send_addr=" +
				toString(inet_ntoa(send_addr.sin_addr))+":"+toString(ntohs(send_addr.sin_port)) );

			Log::INFO( "[SocketPortUdp] bind_addr=" +
				toString(inet_ntoa(bind_addr.sin_addr))+":"+toString(ntohs(bind_addr.sin_port)) );

			nodelayFlag = 1;
			if(conf->SocketRecvTimeout < 1000)
			{
				recvTimeout.tv_usec = conf->SocketRecvTimeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec =0;
				recvTimeout.tv_sec = conf->SocketRecvTimeout/1000;
			}
			sendTimeout.tv_sec = 1;
			sendTimeout.tv_usec = 0;
			//timeout = conf->SocketRecvTimeout;

			config = *conf;
			Log::INFO( "[SocketPortUdp::Init OK]" );
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			isConnected = false;
		}
	}

	bool SocketPortUdp::ConnectOK(void)
	{
		//Log::DEBUG( "[SocketPortUdp] ConnectOK()");
		if(!IsConnected())
		{
			Reconnect();
		}
		return IsConnected();
	}

	bool SocketPortUdp::ConnectOK(int timeout)
	{
		//Log::DEBUG( "[SocketPortUdp] ConnectOK(int timeout) timeout="+toString(timeout));
		if(!IsConnected())
		{
			Reconnect(timeout);
		}
		return IsConnected();
	}

	void SocketPortUdp::Reconnect(int timeout)
	{
		//pthread_mutex_lock(&sych);
		try
		{
			Log::INFO( "[SocketPortUdp] start reconnectSocket" );
			if(sock > 0 || isConnected == true)
			{
				Close();
			}
			if(!isvirtual)
			{
				sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if(sock < 0)
					throw runtime_error( "create socket error code=" + string( strerror(errno) ) );

				struct timeval recv_tim = recvTimeout;
				if(timeout != 0)
				{
					if(timeout < 1000)
					{
						recv_tim.tv_usec = timeout*1000;
						recv_tim.tv_sec  = 0;
					}
					else
					{
						recv_tim.tv_usec =(timeout%1000)*1000;
						recv_tim.tv_sec = timeout/1000;
					}
					//Log::DEBUG( "[SocketPortUdp] reconnectSocket new timeout tv_sec="+toString(recv_tim.tv_sec) + " tv_usec="+toString(recv_tim.tv_usec) );
				}

				int stat = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval*)&recv_tim, sizeof(struct timeval)); //SOL_SOCKET, );
				if(stat < 0)
					throw runtime_error("SO_RCVTIMEO error!");

				stat = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(struct timeval*)&sendTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
				if(stat < 0)
					throw runtime_error("SO_SNDTIMEO error!");

				Log::INFO( "[SocketPortUdp] try bind_addr=" + toString(inet_ntoa(bind_addr.sin_addr))+":"+toString(ntohs(bind_addr.sin_port)) );

				if( bind(sock , (struct sockaddr*)&bind_addr, sizeof(bind_addr) ) < 0)
					throw runtime_error( "SocketPortUdp bind error code=" + string( strerror(errno) ) );

				isConnected = true;
				Log::INFO( "[SocketPortUdp] end reconnectSocket sock="+toString(sock) +" isConnected="+toString(isConnected) );
			}
			else
			{
				isConnected = true;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			Close();
			//sleep(1);
		}
		//pthread_mutex_unlock(&sych);
	}

	bool SocketPortUdp::Close(void)
	{
		bool ret = false;
		try
		{
			//Log::DEBUG( "[SocketPortUdp] Close() sock="+toString(sock) );
			if(!isvirtual)
			{
				if(sock > 0)
				{
					shutdown(sock, SHUT_RDWR);
					close(sock);
					sock = 0;
				}
			}
			isConnected = false;
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
		}
		ret = true;
		return ret;
	}

	bool SocketPortUdp::IsConnected(void)
	{
		bool ret;
		//pthread_mutex_lock(&sych);
		ret = isvirtual == true ? isConnected :  isConnected;
		//pthread_mutex_unlock(&sych);
		//Log::DEBUG( "[SocketPortUdp] IsConnected()="+toString(isConnected) );
		return ret;
	}

	sWord SocketPortUdp::Send(Byte *Buf, Word Len)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{

			Log::INFO( "[SocketPortUdp] Send sendto addr=" +
					toString(inet_ntoa(send_addr.sin_addr))+
						":"+toString(ntohs(send_addr.sin_port))+" Len="+toString(Len) );

			len = sendto(sock, Buf, Len, 0,(struct sockaddr*)&send_addr, sizeof(send_addr));
			Log::DEBUG( "SocketPortUdp::Send len=" + toString(len)+" Len="+toString(Len));
			if(len >= 0 )
			{
				//Log::DEBUG( "[SocketPortUDP] send ok: [" + string((char*)Buf, len)+"]");
				//Log::DEBUG( "[UdpListener] accept client OK, connection from addr " + toString(inet_ntoa(s_addr.sin_addr))+":"+ntohs(s_addr.sin_port) );
				//Log::DEBUG( "[SocketPortUdp] send socket len ="+toString(len)+ " to "+
				//		 toString(inet_ntoa(send_addr.sin_addr))+":"+toString(ntohs(send_addr.sin_port)) );
				isConnected = true;
			}
			else
			{
				//Log::DEBUG( "[SocketPortUdp] send err");
				isConnected = false;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPortUdp::Recv(Byte *Buf, Word Len)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			Log::DEBUG( "SocketPortUdp::Recv recvfrom with memset 0 before");
			memset(Buf, 0, Len);
			unsigned int slen = sizeof(recv_addr);
			len = recvfrom(sock, Buf, Len, 0,(struct sockaddr*)&recv_addr, &slen );
			//Log::DEBUG( "SocketPortUdp::Recv len=" + toString(len));
			Log::DEBUG( "SocketPortUdp::recvfrom len=" + toString(len)+ " ip="+toString(inet_ntoa(recv_addr.sin_addr))+":"+toString(ntohs(recv_addr.sin_port)) );
			isConnected = true;
			if(len == 0)
			{
				isConnected = false;
			}
			else if(len > 0)
			{
				//Log::DEBUG( "[SocketPortUDP] recvfrom ok: [" + string((char*)Buf, len)+"]");
				/*
				Log::DEBUG( "SocketPortUdp::recvfrom len=" + toString(len)+ " ip="+
						toString(inet_ntoa(addr.sin_addr))+
							":"+toString(ntohs(addr.sin_port)) );
				 	 	 */
				//Log::DEBUG( "SocketPortUdp::Recv len="+ toString(len) );
			}
			else
			{
				len = 0;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPortUdp::Recv(Byte *Buf, Word Len, Word MaxLen)
	{
		if(Len > MaxLen)
			Len = MaxLen;
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			unsigned int slen = sizeof(recv_addr);
			len = recvfrom(sock, Buf, Len, 0,(struct sockaddr*)&recv_addr, &slen );
			//len = recv(sock, Buf, Len, 0);
			//Log::DEBUG( "len =" + toString(len));
			Log::DEBUG( "SocketPortUdp::recvfrom len=" + toString(len)+ " ip="+
					toString(inet_ntoa(recv_addr.sin_addr))+
						":"+toString(ntohs(recv_addr.sin_port)) );
			isConnected = true;
			if(len == 0)
			{
				isConnected = false;
			}
			else if(len > 0)
			{
				//Log::DEBUG( "recv: "+ string( (char*)Buf, len ) );
			}
			else
			{
				len = 0;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPortUdp::RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			char rb[1] {0};
            int bytes;
            isConnected = true;
            do
            {
    			unsigned int slen = sizeof(recv_addr);
    			bytes = recvfrom(sock, rb, 1, 0,(struct sockaddr*)&recv_addr, &slen );
                //bytes = recv(sock, rb, 1, 0);
    			if(bytes == 1)
    			{//ok
    				Buf[len++] = rb[0];
    				if(len >= MaxLen) break;
    			}
    			else
    			{
    				if(bytes == 0)
    				{// disconnect
    					isConnected = false;
    				}
    				break;
    			}
            }
            while ( rb[0] != StopSymbol );
            if(len > 0)
            {
            	//Log::DEBUG( "[SockeSocketPortUdptPort] recvto ok: ["+ string( (char*)Buf, len )+ "] len="+toString(len));
            }
		}
		catch(exception &e)
		{
			Log::ERROR( "SocketPortUdp::RecvTo: " + toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	string SocketPortUdp::ToString(){
		return config.IpAddress+":"+toString(config.IpPort);
	}

	int SocketPortUdp::GetBytesInBuffer(void)
	{
		int fd;
		int bytes;

		ioctl(sock, FIONREAD, &bytes);

		return bytes;
	}

	//=======================================
	//*** SocketPort class ******************
	//=======================================
	SocketPort::SocketPort()
	{
		sock = 0;
		isConnected 	= false;
		nodelayFlag  	= 1;
	}

	SocketPort::SocketPort(void *sockSettings)
	{
		sock = 0;
		isConnected 	= false;
		nodelayFlag  	= 1;
		Init(sockSettings);
	}

	SocketPort::SocketPort(int s, struct sockaddr_in s_addr, struct timeval s_tim )
	{
		sock 			= s;
		recvTimeout 	= s_tim;
		addr			= s_addr;
		nodelayFlag 	= 1;

		Log::INFO( "[SocketPort::SocketPort]  creating sock=" + toString(sock) );
		Log::INFO( "[SocketPort::SocketPort]  recvTimeout.tv_sec=" + toString(recvTimeout.tv_sec) );
		Log::INFO( "[SocketPort::SocketPort]  recvTimeout.tv_usec=" + toString(recvTimeout.tv_usec) );
		int stat1 = 0;
		int stat2 = 0;

		//stat1 = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelayFlag, sizeof(int));
		stat2 = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&recvTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
		if(stat1 < 0 || stat2 < 0)
			throw runtime_error( "server setsockopt error code=" + string( strerror(errno) ) );

		socklen_t optlen = sizeof(struct timeval);
		struct timeval 	getrecvTimeout;
		stat2 = getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&getrecvTimeout, &optlen);

		Log::INFO( "[SocketPort::SocketPort]  getrecvTimeout.tv_sec=" + toString(getrecvTimeout.tv_sec) );
		Log::INFO( "[SocketPort::SocketPort]  getrecvTimeout.tv_usec=" + toString(getrecvTimeout.tv_usec) );
		isConnected = true;

		Log::INFO( "[SocketPort::SocketPort]  is created" );
	}

	SocketPort::~SocketPort()
	{

	}

	void SocketPort::Init(void *sockSettings)
	{
		try
		{
			Log::INFO( "[SocketPort::Init]" );

			SocketSettings *conf = (SocketSettings *)sockSettings;
			remoute_host = conf->IpAddress;
			bzero(&addr, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(conf->IpPort);//htons(9002);

			//addr.sin_addr.s_addr = inet_addr(remoute_host.c_str());//inet_addr("192.168.1.10");//gethostbyname
			///////////////////////////init addr//////////////
			if (inet_addr(remoute_host.c_str()) != INADDR_NONE){
				addr.sin_addr.s_addr = inet_addr(remoute_host.c_str());
				//Log::DEBUG( "[SocketPort::Init] inet_addr != INADDR_NONE" );
			}
			else{
				//Log::DEBUG( "[SocketPort::Init] inet_addr == INADDR_NONE start gethostbyname" );

				struct hostent *hst = gethostbyname(remoute_host.c_str());
			    if ( hst ){
			    	addr.sin_addr = *(struct in_addr *) hst->h_addr_list[0];
			    	Log::DEBUG( "[SocketPort::Init] gethostbyname finish add="+toString(inet_ntoa(addr.sin_addr)) );
			    }

			    //const struct sockaddr sa;
			    //sockaddr sa;
			    //int res = getnameinfo(&sa, sizeof(sa), (char*)remoute_host.c_str(), remoute_host.size(), NULL, NULL, 0);
			}
			////////////////////////////////////////////////////
			nodelayFlag = 1;
			if(conf->SocketRecvTimeout < 1000)
			{
				recvTimeout.tv_usec = conf->SocketRecvTimeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec = 0;
				recvTimeout.tv_sec = conf->SocketRecvTimeout/1000;
			}
			sendTimeout.tv_sec = 0;
			//timeout = conf->SocketRecvTimeout;

			config = *conf;
			Log::INFO( "[SocketPort::Init OK]" );



		}
		catch(exception &e)
		{
			Log::ERROR("[SocketPort::Init] "+ toString(e.what()) );
			isConnected = false;
		}
	}

	bool SocketPort::ConnectOK(void)
	{
		if(!IsConnected())
		{
			Reconnect();
		}
		return IsConnected();
	}

	bool SocketPort::ConnectOK(int timeout)
	{
		if(!IsConnected())
		{
			Reconnect(timeout);
		}
		return IsConnected();
	}

	void SocketPort::Reconnect(int timeout)
	{
		//pthread_mutex_lock(&sych);
		try
		{
			Log::INFO( "[SocketPort] reconnectSocket" );
			if(sock > 0 || isConnected == true)
			{
				Close();
			}

			if(!isvirtual)
			{
				/////////////////////init addr//////////////
				if (inet_addr(remoute_host.c_str())!=INADDR_NONE)
				{
					addr.sin_addr.s_addr = inet_addr(remoute_host.c_str());
					Log::DEBUG( "[SocketPort::Init] inet_addr != INADDR_NONE" );
				}
				else
				{
					Log::DEBUG( "[SocketPort::Init] inet_addr == INADDR_NONE start gethostbyname" );
					struct hostent *hst = gethostbyname(remoute_host.c_str());
					if ( hst ){
						addr.sin_addr = *(struct in_addr *) hst->h_addr_list[0];
						Log::DEBUG( "[SocketPort::Init] gethostbyname finish add="+toString(inet_ntoa(addr.sin_addr)) );
					}
				}
				///////////////////////////////////////////////
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if(sock < 0)
				{
					Log::ERROR( "[SocketPort] socket error" );
					//pthread_mutex_unlock(&sych);
					return;
				}
				int stat = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelayFlag, sizeof(int));
				if(stat < 0)
				{
					Log::ERROR( "[SocketPort] Reconnect TCP_NODELAY" );
				}
				stat = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&recvTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
				if(stat < 0)
				{
					Log::ERROR( "[SocketPort] Reconnect SO_RCVTIMEO" );
				}
				Log::INFO( "[SocketPort] connecting to " + toString(inet_ntoa(addr.sin_addr))+":"+toString(ntohs(addr.sin_port)) );
				if(  connect(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0)
				{
					Log::INFO( "[SocketPort] connect OK socket="+toString(sock) );
					isConnected = true;

					/////////////////////////////
					socklen_t optlen = sizeof(struct timeval);
					struct timeval 	getrecvTimeout;
					int stat2 = getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&getrecvTimeout, &optlen);
					Log::INFO( "[SocketPort::SocketPort]  getrecvTimeout=" + toString(getrecvTimeout.tv_sec)+"." + toString(getrecvTimeout.tv_usec) );
				}
				else
				{
					//Log::ERROR( "[SocketPort] connect failed!" );
					isConnected = false;
				}
			}
			else
			{
				isConnected = true;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
		}
		//pthread_mutex_unlock(&sych);
	}

	bool SocketPort::Close(void)
	{
		bool ret = false;
		try
		{
			if(!isvirtual)
			{
				if(sock > 0)
				{
					Log::INFO( "[SocketPort] Closing socket="+toString(sock) );
					shutdown(sock, SHUT_RDWR);
					close(sock);
					sock 		= 0;
				}
			}
			isConnected = false;

		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );

		}
		ret = true;
		return ret;
	}

	bool SocketPort::IsConnected(void)
	{
		bool ret;
		//pthread_mutex_lock(&sych);
		//ret = isConnected;

		ret = isvirtual == true ? isConnected :  isConnected;
		//pthread_mutex_unlock(&sych);
		return ret;
	}

	sWord SocketPort::Send(Byte *Buf, Word Len)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			if(isvirtual)return Len;

			len = send(sock, Buf, Len, MSG_NOSIGNAL);
			if(len >= 0 )
			{
				//Log::DEBUG( "[SocketPort] send ok: [" + string((char*)Buf, len)+"]");
				//Log::DEBUG( "[SocketPort] send socket len ="+toString(len));
				isConnected = true;
			}
			else
				throw runtime_error( "socket Send error code=" + string( strerror(errno) ) );

		}
		catch(exception &e)
		{
			Log::ERROR( "[SocketPort::Send] "+toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPort::Recv(Byte *Buf, Word Len)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			//Log::DEBUG( "recv sock =" + toString(sock));
			len = recv(sock, Buf, Len, 0);
			//Log::DEBUG( "recv sock len =" + toString(len));
			isConnected = true;
			if(len == 0)
			{
				isConnected = false;
			}
			else if(len > 0)
			{
				//Log::DEBUG( "recv: "+ string( (char*)Buf, len ) );
			}
			else
			{
				len = 0;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPort::Recv(Byte *Buf, Word Len, Word MaxLen)
	{
		if(Len > MaxLen)
			Len = MaxLen;
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			len = recv(sock, Buf, Len, 0);
			//Log::DEBUG( "len =" + toString(len));
			isConnected = true;
			if(len == 0)
			{
				isConnected = false;
			}
			else if(len > 0)
			{
				//Log::DEBUG( "recv: "+ string( (char*)Buf, len ) );
			}
			else
			{
				len = 0;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	sWord SocketPort::RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;
		try
		{
			char rb[1] {0};
            int bytes;

            int available = GetBytesInBuffer();

            if(available > 0)
            {
				isConnected = true;
				do
				{
					bytes = recv(sock, rb, 1, 0);
					//Log::DEBUG( "[SocketPort] recvto bytes="+toString(bytes));
					if(bytes == 1)
					{//ok
						Buf[len++] = rb[0];
						if(len >= MaxLen) break;
					}
					else
					{
						if(bytes == 0)
						{// disconnect
							isConnected = false;
						}
						break;
					}
				}
				while ( rb[0] != StopSymbol );
            }

            if(len > 0)
            {
            	//Log::DEBUG( "[SocketPort] recvto ok: ["+ string( (char*)Buf, len )+ "] len="+toString(len));
            }
		}
		catch(exception &e)
		{
			Log::ERROR( "SocketPort::RecvTo: " + toString(e.what()) );
			isConnected = false;
			len = 0;
		}
		//pthread_mutex_unlock(&sych);
		return len;
	}

	int SocketPort::sendall(int s, char *buf, int len, int flags)
	{
		//pthread_mutex_lock(&sych);
	    int total = 0;
	    int n;
	    //unsigned long arg;
	    while(total < len)
	    {
	        n = send(s, buf+total, 1, flags);
	        if(n == -1) { break; }
	        total += n;
	    }
	    //pthread_mutex_unlock(&sych);
	    return (n==-1 ? -1 : total);
	}

	int SocketPort::GetBytesInBuffer(void)
	{
		int fd;
		int bytes;

		ioctl(sock, FIONREAD, &bytes);

		return bytes;
	}

	string SocketPort::ToString(){
		return config.IpAddress+":"+toString(config.IpPort);
	}
	//=======================================
	//*** ServerPort class ******************
	//=======================================
	ServerPort::ServerPort()
	{
		// TODO Auto-generated constructor stub
		sock 		= 0;
		newsock 	= 0;
		isConnected = false;
		isRuning = false;
		wait_clients_thread = 0;
		OnDataRecive = NULL;
	}

	ServerPort::~ServerPort()
	{
		// TODO Auto-generated destructor stub
	}

	void ServerPort::Start(void)
	{
		try
		{

			if(IsRuning())
			{
				return;
			}
			Log::INFO( "[ServerPort] Starting" );
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock < 0) return;
			if(bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
			{
				Stop();
				throw runtime_error("server bind error!");
			}
			if(listen(sock, 1) < 0)
			{
				Stop();
				throw runtime_error("server listen error!");
			}
			Log::INFO( "[ServerPort] listen");
			pthread_create(&wait_clients_thread, 	NULL, wait_clients_proc, this);
			isRuning = true;
			Log::INFO( "[ServerPort] Starting successfull!" );
		}
		catch(exception &e)
		{
			Log::ERROR( "[ServerPort::Start] "+toString(e.what()) );
			Stop();
		}
	}

	void ServerPort::Stop(void)
	{
		try
		{
			if(newsock > 0)
			{
				shutdown(newsock, SHUT_RDWR);
				close(newsock);
				newsock = 0;
			}
			isConnected = false;

			if(sock > 0)
			{
				shutdown(sock, SHUT_RDWR);
				close(sock);
				sock = 0;
			}
			isRuning = false;

		}
		catch(exception &e)
		{
			Log::ERROR( "[ServerPort::Stop] "+ toString(e.what()) );
			isRuning = false;
		}
	}

	void ServerPort::Close(void)
	{

	}

	void ServerPort::Init(void *sockSettings)
	{
			Log::INFO( "ServerPort::init" );

			SocketSettings *conf = (SocketSettings *)sockSettings;
			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(conf->IpPort);
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			if(conf->SocketRecvTimeout < 1000)
			{
				recvTimeout.tv_usec = conf->SocketRecvTimeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec =0;
				recvTimeout.tv_sec = conf->SocketRecvTimeout/1000;
			}
			sendTimeout.tv_sec = 0;
	}

	bool ServerPort::IsRuning(void)
	{
		try
		{
		}
		catch(exception &e)
		{
			Log::ERROR( "[ServerPort] "+toString(e.what()) );
		}
		return isRuning;
	}

	bool ServerPort::IsConnected(void)
	{
		return isConnected;
	}

	void *ServerPort::wait_clients_proc(void *arg)
	{
		Log::INFO( "[ServerPort] wait for client..." );
		ServerPort *cl = (ServerPort*)arg;
		socklen_t clien = sizeof(cl->cliaddr);

		while(cl->sock > 0)
		{
			fd_set s_set;
			FD_ZERO(&s_set);
			FD_SET(cl->sock, &s_set);
			timeval timeout = {0, 0};
			int select_res = select(cl->sock+1, &s_set, 0, 0, &timeout);
			//Log::DEBUG("[ServerPort] select_res="+toString(select_res));
			if(select_res > 0) break;
		}
		if(cl->sock > 0)
		{
			cl->newsock = accept(cl->sock, (struct sockaddr *) &cl->cliaddr, &clien);
			if(cl->newsock < 0)
			{
				Log::INFO( "[ServerPort] newsock < 0 exit" );
				cl->Stop();
				return 0;
			}
			else
			{//set options
				Log::INFO( "[ServerPort] accept client OK" );
				Log::INFO( "[ServerPort] connection from " + toString(inet_ntoa(cl->cliaddr.sin_addr)) );
				cl->isConnected = true;
			}
			while(cl->sock > 0 && cl->newsock > 0 )
			{
				try
				{
					TBuffer	Buffer(2048);
					Buffer.DataLen = recv(cl->newsock, Buffer.BUF, 2048, 0);
					if(Buffer.DataLen > 0)
					{//ok
						Log::INFO( "[ServerPort] recv " + toString(Buffer.DataLen) + " bytes" );
						if(cl->OnDataRecive != NULL)
						{//obrabotka dannih
							TBuffer* retBuf = (TBuffer*)cl->OnDataRecive(&Buffer, cl->ptr);
							if(retBuf != NULL)
							{
								int len = send(cl->newsock, retBuf->BUF, retBuf->DataLen, MSG_NOSIGNAL);
								if(len > 0)
								{
									Log::DEBUG( "[ServerPort] send ok: [" + string((char*)retBuf->BUF, len)+"]");
								}
								delete retBuf;
							}
						}
					}
					else
					{
						if(Buffer.DataLen == 0)
						{// disconnect
							//cl->isConnected = false;
						}
						else
						{// if < 0 error ->close
							break;
						}
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[ServerPort::wait_clients_proc]" + toString( e.what() ) );
					sleep(1);
				}
			}
		}
		Log::INFO( "[ServerPort] exit" );
		cl->Stop();
		return 0;
	}

	//=======================================
	//*** ServerPortUdp class ******************
	//=======================================
	ServerPortUdp::ServerPortUdp()
	{
		sock 		= 0;
		newsock 	= 0;
		isConnected = false;
		isRuning = false;
		wait_clients_thread = 0;
		OnDataRecive = NULL;
	}

	ServerPortUdp::~ServerPortUdp()
	{
		Stop();
	}

	void ServerPortUdp::Init(void *sockSettings)
	{
			Log::INFO( "[ServerPortUdp::init]" );

			SocketSettings *conf = (SocketSettings *)sockSettings;
			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(conf->IpPort);
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			if(conf->SocketRecvTimeout < 1000)
			{
				recvTimeout.tv_usec = conf->SocketRecvTimeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec =0;
				recvTimeout.tv_sec = conf->SocketRecvTimeout/1000;
			}
			sendTimeout.tv_sec = 1;
	}

	void ServerPortUdp::Stop(void)
	{
		try
		{
			isRuning = false;

			if(newsock == sock)
			{
				if(newsock > 0)
				{
					shutdown(newsock, SHUT_RDWR);
					close(newsock);
					newsock = 0;
					sock    = 0;
				}
				isConnected = false;
			}
			else
			{
				if(newsock > 0)
				{
					shutdown(newsock, SHUT_RDWR);
					close(newsock);
					newsock = 0;
				}
				isConnected = false;

				if(sock > 0)
				{
					shutdown(sock, SHUT_RDWR);
					close(sock);
					sock = 0;
				}
			}

		}
		catch(exception &e)
		{
			Log::ERROR( "[ServerPortUdp::Stop] "+toString(e.what()) );
		}
	}

	void ServerPortUdp::Close(void)
	{

	}

	void ServerPortUdp::Start(void)
	{
		try
		{
			if(IsRuning())
				return;

			Log::INFO( "[ServerPortUdp] Starting" );
			sock = socket(AF_INET, SOCK_DGRAM, 0);
			if(sock < 0)
				throw runtime_error("create socket error!");

			int stat = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval*)&recvTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
			if(stat < 0)
				throw runtime_error("SO_RCVTIMEO error!");

			stat = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(struct timeval*)&sendTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
			if(stat < 0)
				throw runtime_error("SO_SNDTIMEO error!");

			if(bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
				throw runtime_error("bind error!");

			pthread_create(&wait_clients_thread, 	NULL, wait_clients_proc, this);
			isRuning = true;
			Log::INFO( "[ServerPortUdp] Starting successfull!" );
		}
		catch(exception &e)
		{
			Log::ERROR( "[ServerPortUdp::Start] "+toString(e.what()) );
			Stop();
		}
	}

	void *ServerPortUdp::wait_clients_proc(void *arg)
	{
		Log::INFO( "[ServerPortUdp] wait for client..." );
		ServerPortUdp *cl = (ServerPortUdp*)arg;
		socklen_t clien = sizeof(cl->cliaddr);
		cl->newsock = cl->sock;
		while(cl->sock > 0 && cl->newsock > 0 )
		{
			try
			{
				fd_set s_set;
				FD_ZERO(&s_set);
				FD_SET(cl->sock, &s_set);
				timeval timeout = {0, 0};
				int select_res = select(cl->sock+1, &s_set, 0, 0, &timeout);
				if(select_res > 0)
				{
					//Log::INFO( "[ServerPortUdp] connecting new client");
					TBuffer	Buffer(2048);
					Buffer.DataLen = recvfrom(cl->newsock, Buffer.BUF, 2048, 0,(struct sockaddr *) &cl->cliaddr, &clien);
					if(Buffer.DataLen > 0)
					{//ok
						//Log::INFO( "[ServerPortUdp] connection from " + toString(inet_ntoa(cl->cliaddr.sin_addr)) );
						Log::DEBUG( "[ServerPortUdp] recv from "+ toString(inet_ntoa(cl->cliaddr.sin_addr)) +" len=" + toString(Buffer.DataLen) + " bytes" );
						if(cl->OnDataRecive != NULL)
						{//obrabotka dannih

							TBuffer* retBuf = (TBuffer*)cl->OnDataRecive(&Buffer, cl->ptr);
							if(retBuf != NULL)
							{
								//char str[INET_ADDRSTRLEN];
								//string addr = string( inet_ntop(AF_INET, &cl->cliaddr.sin_addr, str, INET_ADDRSTRLEN ));

								//Log::DEBUG( "[ServerPortUdp] sendto="+addr+":"+toString(ntohs(cl->cliaddr.sin_port))+ " len=" + toString(retBuf->DataLen) + " bytes" );
								int len = sendto(cl->newsock, retBuf->BUF, retBuf->DataLen, MSG_NOSIGNAL, (struct sockaddr *) &cl->cliaddr, sizeof(cl->cliaddr));
								if(len > 0)
									Log::DEBUG( "[ServerPortUdp] send ok len="+toString(len));
								delete retBuf;
							}
/*
							char str[INET_ADDRSTRLEN];
							string addr = string( inet_ntop(AF_INET, &cl->cliaddr.sin_addr, str, INET_ADDRSTRLEN ));
							Log::DEBUG( "[ServerPortUdp] sendto="+addr+":"+toString(ntohs(cl->cliaddr.sin_port))+ " len=" + toString(Buffer.DataLen) + " bytes" );
							int len = sendto(cl->newsock, Buffer.BUF, Buffer.DataLen, MSG_NOSIGNAL, (struct sockaddr *) &cl->cliaddr, sizeof(cl->cliaddr));
							if(len > 0)
								Log::DEBUG( "[ServerPortUdp] send ok len="+toString(len));
*/
						}
					}
					else
					{
						if(Buffer.DataLen == 0)
						{// disconnect
							//cl->isConnected = false;
						}
						else
						{// if < 0 error ->close
							break;
						}
					}
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[ServerPortUdp::wait_clients_proc] error" + toString( e.what() ) );
				sleep(1);
			}
		}
		Log::INFO( "[ServerPortUdp] exit" );
		cl->Stop();
		return 0;
	}

	bool ServerPortUdp::IsRuning(void)
	{
		return isRuning;
	}

	bool ServerPortUdp::IsConnected(void)
	{
		return isConnected;
	}


	//=======================================
	//*** TcpServer class ******************
	//=======================================
	TcpServer::TcpServer()
	{
		sock 		= 0;
		newsock 	= 0;
		isConnected = false;
		isRuning = false;
		wait_clients_thread = 0;
		do_client_thread 	= 0;
		OnDataRecive = NULL;
		errCNT 		= 0;
	}

	TcpServer::~TcpServer()
	{
	}

	void TcpServer::Start(void)
	{
		try
		{
			if(IsRuning())
			{
				return;
			}
			Log::INFO( "[TcpServer] Starting" );
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock < 0) return;


			int bi = bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
			if( bi < 0)
			{
				Stop();
				sleep(5);
				throw runtime_error( "server bind error code=" + string( strerror(errno) ) );
			}
			if(listen(sock, 100) < 0)
			{
				Stop();
				throw runtime_error("server listen error!");
			}
			Log::INFO( "[TcpServer] listen");

			pthread_create(&wait_clients_thread, 	NULL, wait_clients_proc, this);

			isRuning = true;
			Log::INFO( "[TcpServer] Starting successfull!" );
		}
		catch(exception &e)
		{
			Log::ERROR( "[TcpServer::Start] "+toString(e.what()) );
			Stop();
		}
	}

	void TcpServer::Stop(void)
	{
		try
		{
			if(newsock > 0)
			{
				shutdown(newsock, SHUT_RDWR);
				close(newsock);
				newsock = 0;
			}
			isConnected = false;

			if(sock > 0)
			{
				shutdown(sock, SHUT_RDWR);
				close(sock);
				sock = 0;
			}
			isRuning = false;
			Log::INFO( "[TcpServer] Stop" );
		}
		catch(exception &e)
		{
			Log::ERROR( "[TcpServer::Stop] "+toString(e.what()) );
			isRuning = false;
		}
	}

	void TcpServer::Close(void)
	{
		try
		{
			if(newsock > 0)
			{
				Log::INFO( "[TcpServer] Closing socket="+toString(newsock) );
				shutdown(newsock, SHUT_RDWR);
				close(newsock);
				newsock = 0;
			}
			isConnected = false;
			Log::INFO( "[TcpServer] Close newsock" );
		}
		catch(exception &e)
		{
			Log::ERROR( "[TcpServer::Close] "+toString(e.what()) );
		}
	}

	void TcpServer::Init(void *sockSettings)
	{
			Log::INFO( "TcpServer::init" );

			SocketSettings *conf = (SocketSettings *)sockSettings;
			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(conf->IpPort);
			servaddr.sin_addr.s_addr = inet_addr(conf->IpAddress.c_str());//htonl(INADDR_ANY);

			Log::INFO( "TcpServer::IpAddress="+conf->IpAddress );
			Log::INFO( "TcpServer::Port="+toString(conf->IpPort) );

			if(conf->SocketRecvTimeout < 1000)
			{
				recvTimeout.tv_usec = conf->SocketRecvTimeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec =0;
				recvTimeout.tv_sec = conf->SocketRecvTimeout/1000;
			}
			sendTimeout.tv_sec = 0;
	}

	bool TcpServer::IsRuning(void)
	{
		try
		{
		}
		catch(exception &e)
		{
			Log::ERROR( "[TcpServer] "+toString(e.what()) );
		}
		return isRuning;
	}

	bool TcpServer::IsConnected(void)
	{
		return isConnected;
	}

	void *TcpServer::wait_clients_proc(void *arg)
	{
		Log::INFO( "[TcpServer] wait for clients..." );
		TcpServer *cl = (TcpServer*)arg;
		socklen_t clien = sizeof(cl->cliaddr);

		while(cl->sock > 0)
		{
			while(cl->sock > 0)
			{
				fd_set s_set;
				FD_ZERO(&s_set);
				FD_SET(cl->sock, &s_set);
				timeval timeout = {0, 0};
				//Log::INFO( "[TcpServer] select" );
				int select_res = select(cl->sock+1, &s_set, 0, 0, &timeout);
				if(select_res > 0) break;
			}
			if(cl->sock > 0)
			{
				Log::INFO( "[TcpServer] accept" );

				int new_client = accept(cl->sock, (struct sockaddr *) &cl->cliaddr, &clien);
				if(new_client < 0)
				{
					Log::INFO( "[TcpServer] new_client < 0 exit" );
					cl->Stop();
					return 0;
				}
				else
				{//set options
					/*if(cl->isConnected)
					{
						Log::INFO( "[TcpServer] isConnected drop incoming connection!" );
						close(new_client);
						continue;
					}*/
					//cl->Close();//new
					/*
					if(cl->do_client_thread != 0)
					{
						if(cl->do_client_thread != 0)
						{
							Log::INFO( "[TcpServer] pthread_join" );
							pthread_join(cl->do_client_thread, NULL);//wait olt thread//new
							Log::INFO( "[TcpServer] pthread_join OK" );
						}
					}*/

					cl->newsock = new_client;
					int stat = setsockopt(cl->newsock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&cl->recvTimeout, sizeof(struct timeval));
					if(stat < 0)
					{
						Log::ERROR( "[TcpServer] Reconnect SO_RCVTIMEO" );
						continue;
					}
					Log::INFO( "[TcpServer] accept client OK" );
					Log::INFO( "[TcpServer] connection from " + toString(inet_ntoa(cl->cliaddr.sin_addr)) );
					pthread_create(&cl->do_client_thread, 	NULL, cl->do_client_proc, arg);//new
					//cl->isConnected = true;
				}
			}
		}
		Log::INFO( "[TcpServer] exit" );
		cl->Stop();
		return 0;
	}

	void *TcpServer::do_client_proc(void *arg)
	{
		Log::INFO( "[TcpServer] do_client_proc..." );
		TcpServer *server = (TcpServer*)arg;
		//Log::DEBUG( "server == NULL"+toString(server == NULL));
		Log::DEBUG( "sock="+toString(server->sock)+" newsock="+toString(server->newsock));
		while(server->sock > 0 && server->newsock > 0 )
		{
			server->isConnected = true;
			//Log::DEBUG( "sock="+toString(server->sock)+" newsock="+toString(server->newsock));
			try
			{
				if(!server->messagess.empty())
				{
					TBuffer *currMessage = server->messagess.front();
					if(currMessage != NULL)
					{
						Log::DEBUG( "[TcpServer] sending DataLen="+toString(currMessage->DataLen));
						int len = send(server->newsock, currMessage->BUF, currMessage->DataLen, MSG_NOSIGNAL);
						if(len >= 0)
						{
							server->errCNT = 0;
							server->messagess.pop_front();
							if(currMessage != NULL)
							{
								delete currMessage;
								currMessage = NULL;
							}
							Log::DEBUG( "[TcpServer] send ok len="+toString(len));
						}
						else
						{// disconnect
							if(server->errCNT++ > 10)
							{
								server->errCNT = 0;
								server->messagess.pop_front();
								if(currMessage != NULL)
								{
									Log::DEBUG( "[TcpServer] errCNT > 10");
									delete currMessage;
									currMessage = NULL;
								}
							}
							Log::ERROR( "[TcpServer] send error len="+toString(len));
							server->Close();
							continue;
						}
					}
				}
				TBuffer	Buffer(server->mtu);
				Buffer.DataLen = recv(server->newsock, Buffer.BUF, server->mtu, 0);
				if(Buffer.DataLen > 0)
				{//ok
					Log::INFO( "[TcpServer] recv:" + toString(Buffer.DataLen) + " bytes" );
					if(server->ptr != NULL)
					{
						sync_deque<TBuffer *> *messagess = (sync_deque<TBuffer *> *)server->ptr;
						//Log::DEBUG("[TcpServer] messagess->size()="+toString(messagess->size()) );
						if(messagess->size() < 10){
							TBuffer *nbuf = new TBuffer(Buffer.DataLen);
							for(int i = 0; i < Buffer.DataLen; i++)
								nbuf->BUF[i] = Buffer.BUF[i];
							nbuf->DataLen = Buffer.DataLen;
							messagess->push_back(nbuf);
							Log::INFO("[TcpServer] messagess.size="+toString( messagess->size() ));
						}
					}
				}
				else if(Buffer.DataLen == 0)
				{
					Log::ERROR( "[TcpServer::do_client_proc] errno=" + toString(errno) );
					server->Close();
					continue;
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[TcpServer::do_client_proc]" + toString( e.what() ) );
				break;
			}
		}
		server->Close();
		Log::DEBUG( "ending do_client_proc");
		return 0;
	}


	//=======================================
	//*** ClientServer class ******************
	//=======================================

	void ClientServer::Init(void *sockSettings)
	{
		try
		{
			Log::INFO( "[ClientServer::Init start]" );

			SocketSettings *cfg = (SocketSettings *)sockSettings;
/*
			Log::DEBUG("[ClientServer::Init cfg=");
			Log::DEBUG("BindPort="+toString(cfg->BindPort));
			Log::DEBUG("IpPort="+toString(cfg->IpPort));
			Log::DEBUG("IpAddress="+toString(cfg->IpAddress));*/

			_server = new TcpListener(cfg);

			Log::INFO( "[ClientServer::Init OK]" );
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			Close();
		}
	}

	bool ClientServer::ConnectOK(void)
	{
		if(!IsConnected())
		{
			Log::DEBUG("ClientServer::ConnectOK IsConnected==false Reconnect");
			Reconnect(0);
		}
		return IsConnected();
	}

	bool ClientServer::ConnectOK(int timeout)
	{
		if(!IsConnected())
		{
			Log::DEBUG("ClientServer::ConnectOK IsConnected==false Reconnect");
			Reconnect(timeout);
		}
		return IsConnected();
	}

	void ClientServer::Reconnect(int timeout)
	{
		//pthread_mutex_lock(&sych);
		try
		{
			if(_server != NULL)
			{
				_server->Start();
				IPort *client = _server->AcceptSocket();
				if(client != NULL)
				{
					_client = client;
					Log::INFO( "[ClientServer] accept client OK, connection from addr "+client->GetSettingsString());

				}
			}
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
			Close();
		}
		//pthread_mutex_unlock(&sych);
	}

	int ClientServer::GetDescriptor()
	{
		return _client == NULL ? 0 : _client->GetDescriptor();
	}

	string ClientServer::GetSettingsString(void)
	{
		string ret = "";

		ret+= "client=";
		ret+=  _client != NULL ?  _client->GetSettingsString() : "";
		ret+= ", server=";
		ret+=  _server != NULL ?  _server->GetSettingsString() : "";

		return ret;
	}

	sWord ClientServer::Send(Byte *Buf, Word Len)
	{
		int len = 0;
		if(_client != NULL)
			len = _client->Send(Buf, Len);

		return len;
	}

	sWord ClientServer::Recv(Byte *Buf, Word Len)
	{
		//pthread_mutex_lock(&sych);
		int len = 0;

		if(_client != NULL)
			len = _client->Recv(Buf, Len);

		return len;
	}
	sWord ClientServer::Recv(Byte *Buf, Word Len, Word MaxLen)
	{
		if(Len > MaxLen)
			Len = MaxLen;

		int len = 0;

		if(_client != NULL)
			len = _client->Recv(Buf, Len, MaxLen);

		return len;
	}

	sWord ClientServer::RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol)
	{
		int len = 0;

		if(_client != NULL)
			len = _client->RecvTo(Buf, MaxLen, StopSymbol);

		return len;
	}

	////////////////////////////////////////////////////////
	//=======================================
	//*** Endpoint class ******************
	//=======================================
	Endpoint::Endpoint()  {
		reset_address();
	}
	Endpoint::~Endpoint() {}

	void Endpoint::reset_address(void) {
		memset(&_remoteHost, 0, sizeof(struct sockaddr_in));
		_ipAddress[0] = '\0';
	}

	int Endpoint::set_address(const char* host, const int port) {
		reset_address();

		/*
		// IP Address
		char address[5];
		char *p_address = address;

		// Dot-decimal notation
		int result = std::sscanf(host, "%3u.%3u.%3u.%3u",
			(unsigned int*)&address[0], (unsigned int*)&address[1],
			(unsigned int*)&address[2], (unsigned int*)&address[3]);

		if (result != 4) {
			// Resolve address with DNS
			struct hostent *host_address = gethostbyname(host);
			if (host_address == NULL)
				return -1; //Could not resolve address
			p_address = (char*)host_address->h_addr_list[0];
		}
		memcpy((char*)&_remoteHost.sin_addr.s_addr, p_address, 4);
		*/
		if (inet_addr(host)!=INADDR_NONE)
		{
			_remoteHost.sin_addr.s_addr = inet_addr(host);
			Log::DEBUG( "[Endpoint::set_address] inet_addr != INADDR_NONE" );
		}
		else
		{
			Log::DEBUG( "[Endpoint::set_address] inet_addr == INADDR_NONE start gethostbyname" );
			struct hostent *hst = gethostbyname(host);
		    if ( hst ){
		    	_remoteHost.sin_addr = *(struct in_addr *) hst->h_addr_list[0];
		    	Log::DEBUG( "[Endpoint::set_address] gethostbyname finish add="+toString(inet_ntoa(_remoteHost.sin_addr)) );
		    }
		}


		// Address family
		_remoteHost.sin_family = AF_INET;

		// Set port
		_remoteHost.sin_port = htons(port);

		return 0;
	}

	char* Endpoint::get_address() {
		if ((_ipAddress[0] == '\0') && (_remoteHost.sin_addr.s_addr != 0)){
			//inet_ntoa(_remoteHost.sin_addr, _ipAddress);
			sprintf(_ipAddress, "%s", inet_ntoa(_remoteHost.sin_addr));
		}
		return _ipAddress;
	}

	int   Endpoint::get_port() {
		return ntohs(_remoteHost.sin_port);
	}

	//=======================================
	//*** Socket class ******************
	//=======================================
	Socket::Socket() : _sock_fd(-1), _blocking(true), _timeout(1500)
	{

	}


	void Socket::Set_blocking(bool blocking, unsigned int timeout) {
		_blocking = blocking;
		_timeout = timeout;
	}

	int Socket::Init_socket(int type) {
		if (_sock_fd != -1)
			return -1;

		int fd = socket(AF_INET, type, 0);
		if (fd < 0)
			return -1;

		_sock_fd = fd;
		return 0;
	}

	int Socket::Set_option(int level, int optname, const void *optval, socklen_t optlen) {
		return setsockopt(_sock_fd, level, optname, optval, optlen);
	}

	int Socket::Get_option(int level, int optname, void *optval, socklen_t *optlen) {
		return getsockopt(_sock_fd, level, optname, optval, optlen);
	}

	int Socket::Select(struct timeval *timeout, bool read, bool write) {
		fd_set fdSet;
		FD_ZERO(&fdSet);
		FD_SET(_sock_fd, &fdSet);

		fd_set* readset  = (read ) ? (&fdSet) : (NULL);
		fd_set* writeset = (write) ? (&fdSet) : (NULL);

		int ret = select(FD_SETSIZE, readset, writeset, NULL, timeout);
		return (ret <= 0 || !FD_ISSET(_sock_fd, &fdSet)) ? (-1) : (0);
	}


	int Socket::Wait_readable(TimeInterval& timeout) {
		return Select(&timeout._time, true, false);
	}

	int Socket::Wait_writable(TimeInterval& timeout) {
		return Select(&timeout._time, false, true);
	}


	int Socket::Close(bool needshutdown)
	{
		Log::DEBUG("Socket::Close");
		if (_sock_fd < 0)
			return -1;

		if (needshutdown)
			shutdown(_sock_fd, SHUT_RDWR);
		close(_sock_fd);
		_sock_fd = -1;

		return 0;
	}

	Socket::~Socket() {
		Close(); //Don't want to leak
	}

	TimeInterval::TimeInterval(unsigned int ms) {
		_time.tv_sec = ms / 1000;
		_time.tv_usec = (ms - (_time.tv_sec * 1000)) * 1000;
	}




