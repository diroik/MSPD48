/*
 * SocketPort.h
 *
 *  Created on: Sep 22, 2014
 *      Author: user
 */

#ifndef SOCKETPORT_H_
#define SOCKETPORT_H_

#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <algorithm>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <Log.h>
#include <Classes.h>



using namespace std;
using namespace log4net;


/*
	class Socket
	{
	public:

		static void getMACAddress(std::string _iface, unsigned char MAC[6]) {
	        int fd = socket(AF_INET, SOCK_DGRAM, 0);
	        struct ifreq ifr;
	        ifr.ifr_addr.sa_family = AF_INET;
	        strncpy(ifr.ifr_name , _iface.c_str() , IFNAMSIZ-1);
	        ioctl(fd, SIOCGIFHWADDR, &ifr);
	        bzero(MAC, sizeof(MAC));
	        for(unsigned int i=0;i<6;i++)
	            MAC[i] = ifr.ifr_hwaddr.sa_data[i];
	        ioctl(fd, SIOCGIFMTU, &ifr);
	        close(fd);
	        printf("MTU: %d\n",ifr.ifr_mtu);
	        printf("MAC:[%.2x:%.2x:%.2x:%.2x:%.2x:%.2x]\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
	    }

	};
*/

	class SocketPortUdp:public IPort
	{
	public:

		SocketPortUdp();

		SocketPortUdp(int s, struct sockaddr_in recv_addr, struct sockaddr_in snd_addr, struct timeval s_tim);

		virtual ~SocketPortUdp();

		void Init(void *sockSettings);

		bool ConnectOK(void);

		bool ConnectOK(int timeout);

		void Reconnect(int timeout = 0);

		bool Close(void);

		bool IsConnected(void);

		sWord Send(Byte *Buf, Word Len);

		sWord Recv(Byte *Buf, Word Len);

		sWord Recv(Byte *Buf, Word Len, Word MaxLen);

		sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol);

		int GetDescriptor()
		{
			return sock;
		}

		string GetSettingsString(void)
		{
			string ret = "";
			char str[INET_ADDRSTRLEN];


			ret+="[addr.sin_addr="+string( inet_ntop(AF_INET, &bind_addr.sin_addr, str, INET_ADDRSTRLEN ) )+";";
			ret+="port="+toString(ntohs(bind_addr.sin_port))+";";

			if(recvTimeout.tv_usec > 0){
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_usec/1000)+";";}
			else{
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_sec*1000)+";";}

			if(sendTimeout.tv_usec > 0){
				ret+="SendTimeout="+toString((int)sendTimeout.tv_usec/1000)+"]";}
			else{
				ret+="SendTimeout="+toString((int)sendTimeout.tv_sec*1000)+"]";}

			return ret;
		}

		string ToString(void);

		int GetBytesInBuffer(void);

		bool PortIsLAN()
		{
			return true;
		}

	private:

		int sock;
		struct sockaddr_in bind_addr;
		struct sockaddr_in send_addr;
		struct sockaddr_in recv_addr;
		struct timeval recvTimeout;
		struct timeval sendTimeout;
		int 	nodelayFlag;
		bool 	isConnected;
		SocketSettings config;
		pthread_mutex_t sych;
	};


	class SocketPort:public IPort
	{
	public:
		SocketPort();

		SocketPort(void *sockSettings);

		SocketPort(int s, struct sockaddr_in s_addr, struct timeval s_tim );

		~SocketPort();

		void Init(void *sockSettings);

		bool ConnectOK(void);

		bool ConnectOK(int timeout);

		void Reconnect(int timeout = 0);

		bool Close(void);

		bool IsConnected(void);

		sWord Send(Byte *Buf, Word Len);

		sWord Recv(Byte *Buf, Word Len);

		sWord Recv(Byte *Buf, Word Len, Word MaxLen);

		sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol);

		int GetDescriptor()
		{
			return sock;
		}

		string GetSettingsString(void)
		{
			string ret = "";
			char str[INET_ADDRSTRLEN];


			ret+="[addr.sin_addr="+string( inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN ) )+";";
			ret+="port="+toString(ntohs(addr.sin_port))+";";

			if(recvTimeout.tv_usec > 0){
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_usec/1000)+";";}
			else{
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_sec*1000)+";";}

			if(sendTimeout.tv_usec > 0){
				ret+="SendTimeout="+toString((int)sendTimeout.tv_usec/1000)+"]";}
			else{
				ret+="SendTimeout="+toString((int)sendTimeout.tv_sec*1000)+"]";}

			return ret;
		}

		string ToString();

		struct sockaddr_in *GetSockAddr(void)
		{
			return &addr;
		}

		int GetBytesInBuffer(void);

		bool PortIsLAN()
		{
			return true;
		}

	private:
		int sendall(int s, char *buf, int len, int flags);

		int 				sock;
		struct sockaddr_in 	addr;
		struct timeval 		recvTimeout;
		struct timeval 		sendTimeout;
		int 				nodelayFlag;
		bool 				isConnected;
		SocketSettings 		config;
		pthread_mutex_t 	sych;

		string remoute_host;
	};

	class SocketPortTcp:public IPort
	{
	public:
		SocketPortTcp()
		{
			sock = 0;
			isConnected 	= false;
			nodelayFlag  	= 1;

			sendTimeout.tv_usec = 100;
			sendTimeout.tv_sec = 0;
		}

		SocketPortTcp(void *sockSettings)
		{
			sock = 0;
			isConnected 	= false;
			nodelayFlag  	= 1;
			sendTimeout.tv_usec = 100;
			sendTimeout.tv_sec = 0;

			Init(sockSettings);
		}

		SocketPortTcp( int s, struct sockaddr_in s_addr, struct timeval s_tim )
		{
			sock 			= s;
			recvTimeout 	= s_tim;
			addr			= s_addr;
			nodelayFlag 	= 1;
			sendTimeout.tv_usec = 100;
			sendTimeout.tv_sec = 0;

			fcntl(sock, F_SETFL, O_NONBLOCK);
			isConnected = true;
			Log::INFO( "[SocketPortTcp::SocketPort]  is created" );
		}

		~SocketPortTcp(){}

		void Init(void *sockSettings)
		{
			try
			{
				Log::INFO( "[SocketPortTcp::Init]" );

				SocketSettings *conf = (SocketSettings *)sockSettings;

				bzero(&addr, sizeof(addr));
				addr.sin_family 		= AF_INET;
				addr.sin_port 			= htons(conf->IpPort);
				addr.sin_addr.s_addr 	= inet_addr(conf->IpAddress.c_str());

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
				Log::INFO( "[SocketPortTcp::Init OK]" );
			}
			catch(exception &e)
			{
				Log::ERROR("[SocketPortTcp::Init] "+ toString(e.what()) );
				isConnected = false;
			}
		}

		bool ConnectOK(void)
		{
			if( isConnected == false)
			{
				Reconnect();
			}
			return isConnected;
		}

		bool ConnectOK(int timeout)
		{
			if( isConnected == false)
			{
				Reconnect(timeout);
			}
			return isConnected;
		}

		void Reconnect(int timeout = 0)
		{
			//pthread_mutex_lock(&sych);
			try
			{
				Log::INFO( "[SocketPortTcp] reconnectSocket" );
				if(sock > 0 || isConnected == true)
				{	Close();}
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if(sock < 0)
					throw runtime_error( "socket socket() error code=" + string( strerror(errno) ) );

				//int stat = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelayFlag, sizeof(int));
				//if(stat < 0) Log::ERROR( "[SocketPortTcp] Reconnect TCP_NODELAY" );




				Log::INFO( "[SocketPortTcp] connecting to " + toString(inet_ntoa(addr.sin_addr))+":"+toString(ntohs(addr.sin_port)) );
				if(  connect(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0 )
				{
					if(fcntl(sock, F_SETFL, O_NONBLOCK))
						throw runtime_error( "socket fcntl error code=" + string( strerror(errno) ) );
					Log::INFO( "[SocketPortTcp] connect OK socket="+toString(sock) );
					isConnected = true;
				}
				else
					throw runtime_error( "socket fail to connect error code=" + string( strerror(errno) ) );


			}
			catch(exception &e)
			{
				Log::ERROR( "[SocketPortTcp::Reconnect] "+toString(e.what()) );
				isConnected = false;
			}
			//pthread_mutex_unlock(&sych);
		}

		bool Close(void)
		{
			bool ret = false;
			try
			{
				if(!isvirtual)
				{
					if(sock > 0)
					{
						Log::INFO( "[SocketPortTcp] Closing socket="+toString(sock) );
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

		bool IsConnected(void)
		{
			return isConnected;
		}

		sWord Send(Byte *Buf, Word Len)
		{
			//pthread_mutex_lock(&sych);
			int len = 0;
			try
			{
				len = send(sock, Buf, Len, MSG_NOSIGNAL);
				if(len >= 0 )
					isConnected = true;
				else
					throw runtime_error( "socket Send error code=" + string( strerror(errno) ) );

			}
			catch(exception &e)
			{
				Log::ERROR( "[SocketPortTcp::Send] "+toString(e.what()) );
				isConnected = false;
				len = 0;
			}
			//pthread_mutex_unlock(&sych);
			return len;
		}

		sWord Recv(Byte *Buf, Word Len)
		{
			//pthread_mutex_lock(&sych);
			int len = 0;
			try
			{
				fd_set s_set;
				FD_ZERO(&s_set);
				FD_SET(sock, &s_set);
				//timeval timeout = {0, 0};
				int select_res = select(sock+1, &s_set, 0, 0, &recvTimeout);
				if(select_res > 0)
				{
					if(FD_ISSET(sock, &s_set))
					{
						Log::DEBUG( "[SocketPortTcp::Recv] there data for reading!");
						len = recv(sock, Buf, Len, 0);
						if(len <= 0)
						{
							//if(errno != 0)
							throw runtime_error( "socket "+toString(sock)+" Recv error code=" +string(strerror(errno))+" len="+toString(len) ); /*string(strerror(errno)) );*/
						}
					}
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[SocketPortTcp::Recv] "+toString(e.what()) );
				isConnected = false;
				len = 0;
			}
			//pthread_mutex_unlock(&sych);
			return len;
		}

		sWord Recv(Byte *Buf, Word Len, Word MaxLen)
		{
			if(Len > MaxLen)
				Len = MaxLen;
			//pthread_mutex_lock(&sych);
			int len = 0;
			try
			{
				fd_set s_set;
				FD_ZERO(&s_set);
				FD_SET(sock, &s_set);
				//timeval timeout = {0, 0};
				int select_res = select(sock+1, &s_set, 0, 0, &recvTimeout);
				if(select_res > 0)
				{
					//Log::DEBUG( "[SocketPortTcp::Recv] there data for reading!");
					if(FD_ISSET(sock, &s_set))
					{
						len = recv(sock, Buf, Len, 0);
						if(len <= 0)
							throw runtime_error( "socket Recv error code=" + string( strerror(errno) ) );
					}
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[SocketPortTcp::Recv] "+toString(e.what()) );
				isConnected = false;
				len = 0;
			}
			//pthread_mutex_unlock(&sych);
			return len;
		}

		sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol)
		{
			//pthread_mutex_lock(&sych);
			int len = 0;
			try
			{
				fd_set s_set;
				FD_ZERO(&s_set);
				FD_SET(sock, &s_set);
				int select_res = select(sock+1, &s_set, 0, 0, &recvTimeout);
				if(select_res > 0)
				{
					if(FD_ISSET(sock, &s_set))
					{
						char rb[1] {0};
			            int bytes;
			            isConnected = true;
			            do
			            {
			                bytes = recv(sock, rb, 1, 0);
			    			if(bytes == 1)
			    			{//ok
			    				Buf[len++] = rb[0];
			    				if(len >= MaxLen)
			    					break;
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
				}
	            if(len > 0)
	            {
	            	Log::DEBUG( "[SocketPortTcp] recvto ok: ["+ string( (char*)Buf, len )+ "] len="+toString(len));
	            }
			}
			catch(exception &e)
			{
				Log::ERROR( "[SocketPortTcp::RecvTo] "+toString(e.what()) );
				isConnected = false;
				len = 0;
			}
			//pthread_mutex_unlock(&sych);
			return len;
		}

		int GetDescriptor()
		{
			return sock;
		}

		string GetSettingsString(void)
		{
			string ret = "";
			char str[INET_ADDRSTRLEN];


			ret+="[addr.sin_addr="+string( inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN ) )+";";
			ret+="port="+toString(ntohs(addr.sin_port))+";";

			if(recvTimeout.tv_usec > 0){
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_usec/1000)+";";}
			else{
				ret+="RecvTimeout="+toString((int)recvTimeout.tv_sec*1000)+";";}

			if(sendTimeout.tv_usec > 0){
				ret+="SendTimeout="+toString((int)sendTimeout.tv_usec/1000)+"]";}
			else{
				ret+="SendTimeout="+toString((int)sendTimeout.tv_sec*1000)+"]";}

			return ret;
		}

		struct sockaddr_in *GetSockAddr(void)
		{
			return &addr;
		}

		int GetBytesInBuffer(void)
		{
			int fd;
			int bytes;

			ioctl(sock, FIONREAD, &bytes);

			return bytes;
		}

		bool PortIsLAN()
		{
			return true;
		}

		string ToString(){
			return config.IpAddress+":"+toString(config.IpPort);
		}

	private:
		int sendall(int s, char *buf, int len, int flags);

		int 				sock;
		struct sockaddr_in 	addr;
		struct timeval 		recvTimeout;
		struct timeval 		sendTimeout;
		int 				nodelayFlag;
		bool 				isConnected;
		SocketSettings 		config;

		pthread_mutex_t 	sych;
	};


	class ServerPort:public IServer
	{
	public:
			ServerPort();
			~ServerPort();

			void Init(void *sockSettings);

			void Start(void);

			void Stop(void);

			void Close(void);

			bool IsRuning(void);

			bool IsConnected(void);

			//PFVptr  OnDataRecive;//in IServer


			private:

			    int sock, newsock;
				struct sockaddr_in servaddr, cliaddr;
				struct timeval recvTimeout;
				struct timeval sendTimeout;
				//int nodelayFlag;
				bool isRuning, isConnected;
				pthread_t wait_clients_thread;

				static void *wait_clients_proc(void *arg);

	};

	class ServerPortUdp:public IServer
	{
	public:
		ServerPortUdp();
			~ServerPortUdp();

			void Init(void *sockSettings);

			void Start(void);

			void Stop(void);

			void Close(void);

			bool IsRuning(void);

			bool IsConnected(void);

			//PFVptr  OnDataRecive;//in IServer


			private:

			    int sock, newsock;
				struct sockaddr_in servaddr, cliaddr;
				struct timeval recvTimeout;
				struct timeval sendTimeout;
				//int nodelayFlag;
				bool isRuning, isConnected;
				pthread_t wait_clients_thread;

				static void *wait_clients_proc(void *arg);

	};


	class TcpServer:public IServer
	{
	public:
		TcpServer();
			~TcpServer();

			void Init(void *sockSettings);

			void Start(void);

			void Stop(void);

			void Close(void);

			bool IsRuning(void);

			bool IsConnected(void);

			//PFVptr  OnDataRecive;//in IServer

			struct sockaddr_in *GetSockAddr(void)
			{
				return &servaddr;
			}

	private:
				int errCNT;
			    int sock, newsock;
				struct sockaddr_in servaddr, cliaddr;
				struct timeval recvTimeout;
				struct timeval sendTimeout;
				//int nodelayFlag;
				bool isRuning, isConnected;
				pthread_t wait_clients_thread, do_client_thread;

				static void *wait_clients_proc(void *arg);
				static void *do_client_proc(void *arg);

	};

	class TcpListener: public IListener
	{
	public:
			TcpListener(void *localsockSett)
			{
				listener = -1;
				isRuning    = false;

				Log::INFO( "TcpListener::init" );

				SocketSettings *conf = (SocketSettings *)localsockSett;
				bzero(&servaddr, sizeof(servaddr));
				servaddr.sin_family 		= AF_INET;
				servaddr.sin_port 			= htons(conf->IpPort);
				servaddr.sin_addr.s_addr 	= inet_addr(conf->IpAddress.c_str());//htonl(INADDR_ANY);

				Log::INFO( "TcpListener::IpAddress="+conf->IpAddress );
				Log::INFO( "TcpListener::Port="+toString(conf->IpPort) );

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

			~TcpListener(){
				Stop();
			}

			IPort *AcceptSocket()
			{
				IPort *ret = NULL;
				try
				{
					Log::INFO( "[TcpServer::AcceptSocket] wait for clients..." );
					while(listener >= 0 && isRuning)
					{
						fd_set s_set;
						FD_ZERO(&s_set);
						FD_SET(listener, &s_set);
						timeval timeout = {0, 0};
						int select_res = select(listener+1, &s_set, 0, 0, &timeout);
						if(select_res > 0)
						{
							Log::INFO( "[TcpServer] do accept client");
							struct sockaddr_in 	new_client_addr;
							socklen_t 			new_client_addr_len = sizeof(new_client_addr);
							int new_client = accept(listener, (struct sockaddr *) &new_client_addr, &new_client_addr_len);
							if(new_client < 0)
							{
								throw runtime_error( "server accept error code=" + string( strerror(errno) ) );
							}
							else
							{
								ret = new SocketPortTcp(new_client, new_client_addr, recvTimeout);
								Log::INFO( "[TcpServer] accept client OK, connection from addr " + toString(inet_ntoa(new_client_addr.sin_addr)) );
								break;
							}
						}
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[TcpListener::AcceptSocket] "+toString(e.what()) );
					Stop();
				}
				return ret;
			}

			void Start(void *cfg=NULL)
			{
				try
				{
					if(isRuning)
						return;

					Log::INFO( "[TcpListener] Starting" );
					listener = socket(AF_INET, SOCK_STREAM, 0);
					if(listener < 0)
						throw runtime_error("create socket error!");

					int stat = setsockopt(listener, SOL_SOCKET, SO_RCVTIMEO,(struct timeval*)&recvTimeout, sizeof(struct timeval)); //SOL_SOCKET, );
					if(stat < 0)
						throw runtime_error("SO_RCVTIMEO error!");

					int bi = bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr));
					if( bi < 0)
					{
						Stop();
						sleep(1);
						throw runtime_error( "server bind error code=" + string( strerror(errno) ) );
					}
					if(listen(listener, 50) < 0)
					{
						Stop();
						throw runtime_error("server listen error!");
					}
					Log::INFO( "[TcpListener::Start] Starting successfull!" );
					isRuning = true;
				}
				catch(exception &e)
				{
					Log::ERROR( "[TcpListener::Start] "+toString(e.what()) );
					Stop();
					//isRuning = false;
				}
			}

			bool Stop(void)
			{
				bool ret = false;
				try
				{
					isRuning = false;
					if(listener >= 0)
					{
						Log::INFO( "[TcpListener] Stoping" );
						shutdown(listener, SHUT_RDWR);
						close(listener);
						listener = -1;
					}
					Log::INFO( "[TcpListener] Stop" );
					ret = true;
				}
				catch(exception &e)
				{
					Log::ERROR( "[TcpListener::Stop] "+toString(e.what()) );
				}
				return ret;
			}

			bool IsRuning(void)
			{
				try
				{
				}
				catch(exception &e)
				{
					Log::ERROR( "[TcpListener::IsRuning] "+toString(e.what()) );
				}
				return isRuning;
			}

			struct sockaddr_in *GetSockAddr(void)
			{
				return &servaddr;
			}

	private:

			    int 				listener;
				struct sockaddr_in 	servaddr;
				struct timeval 		recvTimeout;
				struct timeval 		sendTimeout;
				bool 				isRuning;
	};


	class UdpListener: public IListener
	{
	public:
			UdpListener(void *localsockSett)
			{
				listener = -1;
				isRuning    = false;
				extRunFlg = NULL;
				Log::INFO( "UdpListener::init" );

				SocketSettings *conf = (SocketSettings *)localsockSett;
				bzero(&servaddr, sizeof(servaddr));
				servaddr.sin_family 		= AF_INET;
				servaddr.sin_port 			= htons(conf->BindPort);
				if(conf->IpAddress=="0.0.0.0"){
					servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
				}
				else
					servaddr.sin_addr.s_addr 	= inet_addr(conf->IpAddress.c_str());

				//toString(inet_ntoa(new_client_addr.sin_addr))
				Log::INFO( "UdpListener::IpAddress="+toString(inet_ntoa(servaddr.sin_addr)) );
				Log::INFO( "UdpListener::Port="+toString(conf->BindPort) );

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

			~UdpListener(){
				Stop();
			}

			/*
			void Start(bool *flg)
			{
				if(extRunFlg!=flg)
					extRunFlg = flg;
				Start();
			}*/


			void Start(void *cfg=NULL)
			{
				try
				{
					bool *flg = (bool*)cfg;
					if(extRunFlg!=flg)
						extRunFlg = flg;

					if(IsRuning())
						return;

					//Log::INFO( "[UdpListener] Starting" );
					listener = socket(AF_INET, SOCK_DGRAM, 0);
					if(listener < 0)
						throw runtime_error( "create socket error code=" + string( strerror(errno) ) );

					int bi = bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr));
					if( bi < 0)
					{
						Stop();
						sleep(1);
						throw runtime_error( "bind error code=" + string( strerror(errno) ) );
					}

					/*
					if(listen(listener, 50) < 0)
					{
						Stop();
						throw runtime_error("server listen error!");
					}*/

					//Log::INFO( "[UdpListener::Start] Starting successfull!" );
					isRuning = true;
				}
				catch(exception &e)
				{
					Log::ERROR( "[UdpListener::Start] "+toString(e.what()) );
					Stop();
				}
			}

			IPort *AcceptSocket()
			{
				IPort *ret = NULL;
				try
				{
					//Log::INFO( "[UdpListener::AcceptSocket] wait for clients..." );
					while( IsRuning() )
					{
						fd_set s_set;
						FD_ZERO(&s_set);
						FD_SET(listener, &s_set);
						timeval timeout = {0, 0};
						int select_res = select(listener+1, &s_set, 0, 0, &timeout);
						if(select_res > 0)
						{
							//Log::INFO( "[UdpListener] do accept client");
							struct sockaddr_in 	new_client_addr;
							socklen_t 			new_client_addr_len = sizeof(new_client_addr);
							Byte buff[0];

							int st = recvfrom(listener, buff, sizeof(buff), MSG_PEEK,
									(struct sockaddr *) &new_client_addr, &new_client_addr_len);
							//int st = getpeername(listener, (struct sockaddr *)&new_client_addr, &new_client_addr_len);
							if(st < 0){
								throw runtime_error( "server accept error code=" + string( strerror(errno) ) );
							}
							else
							{
								struct sockaddr_in 	send_client_addr;
								send_client_addr 			= new_client_addr;
								send_client_addr.sin_port 	= servaddr.sin_port;

								ret = new SocketPortUdp(listener, new_client_addr, send_client_addr, recvTimeout);
								/*
								Log::INFO( "[UdpListener] accept client OK, connection from addr " +
										toString(inet_ntoa(new_client_addr.sin_addr))+
											":"+toString(ntohs(new_client_addr.sin_port)) );*/
								listener = -1;
								break;
							}
						}
						usleep(1000);
					}
					//Log::INFO( "[UdpListener::AcceptSocket] end AcceptSocket");
				}
				catch(exception &e)
				{
					Log::ERROR( "[UdpListener::AcceptSocket] "+toString(e.what()) );
					Stop();
				}
				return ret;
			}

			bool Stop(void)
			{
				bool ret = false;
				try
				{
					isRuning = false;
					if(listener >= 0)
					{
						shutdown(listener, SHUT_RDWR);
						close(listener);
						listener = -1;
					}
					//Log::INFO( "[UdpListener] Stop" );
					ret = true;
				}
				catch(exception &e)
				{
					Log::ERROR( "[UdpListener::Stop] "+toString(e.what()) );
				}
				return ret;
			}

			bool IsRuning(void)
			{
				return isRuning && (extRunFlg == NULL ? true : *extRunFlg) && listener >= 0;
			}

			struct sockaddr_in *GetSockAddr(void)
			{
				return &servaddr;
			}


			string GetSettingsString(void)
			{
				string ret = "";
				char str[INET_ADDRSTRLEN];


				ret+="[addr.sin_addr="+string( inet_ntop(AF_INET, &servaddr.sin_addr, str, INET_ADDRSTRLEN ) )+";";
				ret+="port="+toString(ntohs(servaddr.sin_port))+";";

				if(recvTimeout.tv_usec > 0){
					ret+="RecvTimeout="+toString((int)recvTimeout.tv_usec/1000)+";";}
				else{
					ret+="RecvTimeout="+toString((int)recvTimeout.tv_sec*1000)+";";}

				if(sendTimeout.tv_usec > 0){
					ret+="SendTimeout="+toString((int)sendTimeout.tv_usec/1000)+"]";}
				else{
					ret+="SendTimeout="+toString((int)sendTimeout.tv_sec*1000)+"]";}

				return ret;
			}


	private:

				bool 				*extRunFlg;
			    int 				listener;
				struct sockaddr_in 	servaddr;
				struct timeval 		recvTimeout;
				struct timeval 		sendTimeout;
				bool 				isRuning;
	};

	class ClientServer: public IPort
	{
		public:

			ClientServer(){
				_client = NULL;
				_server = NULL;
			}

			ClientServer(IPort *cl, IListener *ser){
				_client = cl;
				_server = ser;
			}

			~ClientServer(){
				Close();
			}

			void  Init(void *cfg);

			bool  ConnectOK(void);

			bool  ConnectOK(int timeout);

			void  Reconnect(int timeout);

			bool  IsConnected(void){

				bool cf = _client != NULL ? _client->IsConnected() : false;
				bool sf = _server != NULL ? _server->IsRuning() : false;

				//Log::DEBUG("ClientServer IsConnected cf="+toString(cf));
				//Log::DEBUG("ClientServer IsConnected sf="+toString(sf));

				bool ret = cf && sf;
				//Log::DEBUG("ClientServer IsConnected ret="+toString(ret));
				return ret;
			}

			sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol);

			sWord Recv(Byte *Buf, Word Len, Word MaxLen);

			sWord Recv(Byte *Buf, Word Len);

			sWord Send(Byte *Buf, Word Len);

			bool Close(void)
			{
				return _client != NULL ? _client->Close() : true && _server != NULL ? _server->Stop() : true;
			}

			string GetSettingsString(void);

			int GetDescriptor(void);

			int GetBytesInBuffer(void){
				return 0;
			}

			bool PortIsLAN()
			{
				return true;
			}
		private:

			IPort 		*_client;
			IListener 	*_server;

	};

/////////////////////////////////////////
	class Endpoint
	{

	public:
		/** IP Endpoint (address, port)
		 */
		Endpoint(void);

		~Endpoint(void);

		/** Reset the address of this endpoint
		 */
		void reset_address(void);

		/** Set the address of this endpoint
		\param host The endpoint address (it can either be an IP Address or a hostname that will be resolved with DNS).
		\param port The endpoint port
		\return 0 on success, -1 on failure (when an hostname cannot be resolved by DNS).
		 */
		int  set_address(const char* host, const int port);

		/** Get the IP address of this endpoint
		\return The IP address of this endpoint.
		 */
		char* get_address(void);

		/** Get the port of this endpoint
		\return The port of this endpoint
		 */
		int get_port(void);

	protected:
		char _ipAddress[17];
		struct sockaddr_in _remoteHost;

	};

	class TimeInterval;

	class Socket {
	public:
	    /** Socket
	     */
	    Socket();

	    /** Set blocking or non-blocking mode of the socket and a timeout on
	        blocking socket operations
	    \param blocking  true for blocking mode, false for non-blocking mode.
	    \param timeout   timeout in ms [Default: (1500)ms].
	    */
	    void Set_blocking(bool blocking, unsigned int timeout=1500);

	    /** Set socket options
	    \param level     stack level (see: lwip/sockets.h)
	    \param optname   option ID
	    \param optval    option value
	    \param socklen_t length of the option value
	    \return 0 on success, -1 on failure
	    */
	    int Set_option(int level, int optname, const void *optval, socklen_t optlen);

	    /** Get socket options
	        \param level     stack level (see: lwip/sockets.h)
	        \param optname   option ID
	        \param optval    buffer pointer where to write the option value
	        \param socklen_t length of the option value
	        \return 0 on success, -1 on failure
	        */
	    int Get_option(int level, int optname, void *optval, socklen_t *optlen);

	    /** Close the socket
	        \param shutdown   free the left-over data in message queues
	     */
	    int Close(bool needshutdown=true);

	    ~Socket();


		static string getIpAddress(std::string _iface)
		{
			unsigned char IP[4];
	        int fd = socket(AF_INET, SOCK_DGRAM, 0);
	        struct ifreq ifr;
	        ifr.ifr_addr.sa_family = AF_INET;
	        strncpy(ifr.ifr_name , _iface.c_str() , IFNAMSIZ-1);
	        bzero(IP, sizeof(IP));
	        ioctl(fd, SIOCGIFADDR, &ifr);
	        for(unsigned int i=0;i<4;i++)
	            IP[i] = ifr.ifr_addr.sa_data[i+2];
	        close(fd);
	        //printf("MTU: %d\n",ifr.ifr_mtu);
	        //printf("IP:[%u.%u.%u.%u]\n",IP[0],IP[1],IP[2],IP[3]);

	        string s = toString((int)IP[0])+"."+toString((int)IP[1])+"."+toString((int)IP[2])+"."+toString((int)IP[3]);

	        return s;
	    }


	protected:
	    int _sock_fd;
	    int Init_socket(int type);

	    int Wait_readable(TimeInterval& timeout);
	    int Wait_writable(TimeInterval& timeout);

	    bool _blocking;
	    unsigned int _timeout;

	private:
	    int Select(struct timeval *timeout, bool read, bool write);
	};

	/** Time interval class used to specify timeouts
	 */
	class TimeInterval {
	    friend class Socket;

	public:
	    /** Time Interval
	     \param ms time interval expressed in milliseconds
	      */
	    TimeInterval(unsigned int ms);

	private:
	    struct timeval _time;
	};




#endif /* SOCKETPORT_H_ */
