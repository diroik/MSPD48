#include "TCPSocketConnection.h"
#include <cstring>



	TCPSocketConnection::TCPSocketConnection() :
	        _is_connected(false) {
	}

	int TCPSocketConnection::Connect(const char* host, const int port, const int timeout)
	{
	    if (Init_socket(SOCK_STREAM) < 0)
	        return -1;

	    Log::DEBUG("TCPSocketConnection::Connect Init_socket ok");

	    if (set_address(host, port) != 0)
	        return -1;
	    Log::DEBUG("TCPSocketConnection::Connect set_address ok");

	    if(timeout > 0)
	    {
	    	struct timeval 		recvTimeout;
			if(timeout < 1000)
			{
				recvTimeout.tv_usec = timeout*1000;
				recvTimeout.tv_sec  = 0;
			}
			else
			{
				recvTimeout.tv_usec = 0;
				recvTimeout.tv_sec = timeout/1000;
			}
			if(Set_option(SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&recvTimeout, sizeof(struct timeval)) < 0)
				return -1;
			Log::DEBUG("TCPSocketConnection::Connect Set_option ok");
	    }

	    Log::INFO( "[TCPSocketConnection] connecting to " + toString(inet_ntoa(_remoteHost.sin_addr))+":"+toString(ntohs(_remoteHost.sin_port)) );
	    if (connect(_sock_fd, (const struct sockaddr *) &_remoteHost, sizeof(_remoteHost)) < 0) {
	        Close();
	        return -1;
	    }
	    _is_connected = true;
	    Log::DEBUG("TCPSocketConnection::Connect ok");
	    return 0;
	}

	bool TCPSocketConnection::Is_connected(void) {
	    return _is_connected;
	}

	int TCPSocketConnection::Send(char* data, int length) {
	    if ((_sock_fd < 0) || !_is_connected)
	        return -1;

	    if (!_blocking) {
	        TimeInterval timeout(_timeout);
	        if (Wait_writable(timeout) != 0)
	            return -1;
	    }

	    int n = send(_sock_fd, data, length, 0);
	    _is_connected = (n != 0);

	    return n;
	}

	// -1 if unsuccessful, else number of bytes written
	int TCPSocketConnection::Send_all(char* data, int length) {
	    if ((_sock_fd < 0) || !_is_connected)
	        return -1;

	    int writtenLen = 0;
	    TimeInterval timeout(_timeout);
	    while (writtenLen < length) {
	        if (!_blocking) {
	            // Wait for socket to be writeable
	            if (Wait_writable(timeout) != 0)
	                return writtenLen;
	        }

	        int ret = send(_sock_fd, data + writtenLen, length - writtenLen, 0);
	        if (ret > 0) {
	            writtenLen += ret;
	            continue;
	        } else if (ret == 0) {
	            _is_connected = false;
	            return writtenLen;
	        } else {
	            return -1; //Connnection error
	        }
	    }
	    return writtenLen;
	}

	int TCPSocketConnection::Receive(char* data, int length) {
	    if ((_sock_fd < 0) || !_is_connected)
	        return -1;

	    if (!_blocking) {
	        TimeInterval timeout(_timeout);
	        if (Wait_readable(timeout) != 0)
	            return -1;
	    }

	    int n = recv(_sock_fd, data, length, 0);
	    _is_connected = (n != 0);

	    return n;
	}

	// -1 if unsuccessful, else number of bytes received
	int TCPSocketConnection::Receive_all(char* data, int length) {
	    if ((_sock_fd < 0) || !_is_connected)
	        return -1;

	    int readLen = 0;
	    TimeInterval timeout(_timeout);
	    while (readLen < length) {
	        if (!_blocking) {
	            //Wait for socket to be readable
	            if (Wait_readable(timeout) != 0)
	                return readLen;
	        }

	        int ret = recv(_sock_fd, data + readLen, length - readLen, 0);
	        if (ret > 0) {
	            readLen += ret;
	        } else if (ret == 0) {
	            _is_connected = false;
	            return readLen;
	        } else {
	            return -1; //Connnection error
	        }
	    }
	    return readLen;
	}
