#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <SocketPort.h>

/**
TCP socket connection
*/
class TCPSocketConnection : public Socket, public Endpoint
{

public:
    /** TCP socket connection
    */
    TCPSocketConnection();

    /** Connects this TCP socket to the server
    \param host The host to connect to. It can either be an IP Address or a hostname that will be resolved with DNS.
    \param port The host's port to connect to.
    \return 0 on success, -1 on failure.
    */
    int Connect(const char* host, const int port, const int timeout = 0);

    /** Check if the socket is connected
    \return true if connected, false otherwise.
    */
    bool Is_connected(void);

    /** Send data to the remote host.
    \param data The buffer to send to the host.
    \param length The length of the buffer to send.
    \return the number of written bytes on success (>=0) or -1 on failure
     */
    int Send(char* data, int length);

    /** Send all the data to the remote host.
    \param data The buffer to send to the host.
    \param length The length of the buffer to send.
    \return the number of written bytes on success (>=0) or -1 on failure
    */
    int Send_all(char* data, int length);

    /** Receive data from the remote host.
    \param data The buffer in which to store the data received from the host.
    \param length The maximum length of the buffer.
    \return the number of received bytes on success (>=0) or -1 on failure
     */
    int Receive(char* data, int length);

    /** Receive all the data from the remote host.
    \param data The buffer in which to store the data received from the host.
    \param length The maximum length of the buffer.
    \return the number of received bytes on success (>=0) or -1 on failure
    */
    int Receive_all(char* data, int length);

private:
    bool _is_connected;

};

#endif
