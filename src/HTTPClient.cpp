#ifndef UTILITY_H
#define swMalloc std::malloc         // use the standard
#define swFree std::free
#endif


#include <cstdio>
#define OK 0

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

#define CHUNK_SIZE 256
#define MAXLEN_VALUE 120    /* Max URL and Max Value for Name:Value */

#include <cstring>
#include "HTTPClient.h"

HTTPClient::HTTPClient() :
    m_sock(), m_basicAuthUser(NULL), m_basicAuthPassword(NULL),
    m_nCustomHeaders(0), m_httpResponseCode(0),
    m_maxredirections(1), m_location(NULL), m_customHeaders(NULL), m_timeout(0)
{
}

HTTPClient::~HTTPClient()
{
    if (m_basicAuthUser)
        swFree(m_basicAuthUser);
    if (m_basicAuthPassword)
        swFree(m_basicAuthPassword);
    if (m_location) // if any redirection was involved, clean up after it.
        swFree(m_location);
    m_location = NULL;      // this step isn't necessary...
}


const char * HTTPClient::GetErrorMessage(HTTPResult res)
{
    const char * msg[HTTP_CLOSED+1] = {
        "HTTP OK",            ///<Success
        "HTTP Processing",    ///<Processing
        "HTTP URL Parse error",         ///<url Parse error
        "HTTP DNS error",           ///<Could not resolve name
        "HTTP Protocol error",         ///<Protocol error
        "HTTP 404 Not Found",      ///<HTTP 404 Error
        "HTTP 403 Refused",       ///<HTTP 403 Error
        "HTTP ### Error",         ///<HTTP xxx error
        "HTTP Timeout",       ///<Connection timeout
        "HTTP Connection error",          ///<Connection error
        "HTTP Closed by remote host"         ///<Connection was closed by remote host
    };
    if (res <= HTTP_CLOSED)
        return msg[res];
    else
        return "HTTP Unknown Code";
};


void HTTPClient::basicAuth(const char* user, const char* password) //Basic Authentification
{
#if 1
    if (m_basicAuthUser)
        swFree(m_basicAuthUser);
    m_basicAuthUser = (char *)swMalloc(strlen(user)+1);
    strcpy(m_basicAuthUser, user);
    if (m_basicAuthPassword)
        swFree(m_basicAuthPassword);
    m_basicAuthPassword = (char *)swMalloc(strlen(password)+1);
    strcpy(m_basicAuthPassword, password);
#else
    m_basicAuthUser = user;
    m_basicAuthPassword = password;
#endif
}

void HTTPClient::customHeaders(const char **headers, size_t pairs)
{
    m_customHeaders = headers;
    m_nCustomHeaders = pairs;
}

HTTPResult HTTPClient::get(const char* url, IHTTPDataIn* pDataIn, int timeout /*= HTTP_CLIENT_DEFAULT_TIMEOUT*/) //Blocking
{
    //Log::INFO("url: "+toString(url)+", timeout: " + toString(timeout));
    return connect(url, HTTP_GET, NULL, pDataIn, timeout);
}

HTTPResult HTTPClient::get(const char* url, char* result, size_t maxResultLen, int timeout /*= HTTP_CLIENT_DEFAULT_TIMEOUT*/) //Blocking
{
    //Log::INFO("url: "+toString(url)+", timeout: " + toString(timeout) + ", maxResultLen: "+ toString(maxResultLen));

    HTTPText str(result, maxResultLen);
    return get(url, &str, timeout);
}

HTTPResult HTTPClient::post(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout /*= HTTP_CLIENT_DEFAULT_TIMEOUT*/) //Blocking
{
    return connect(url, HTTP_POST, (IHTTPDataOut*)&dataOut, pDataIn, timeout);
}

HTTPResult HTTPClient::put(const char* url, const IHTTPDataOut& dataOut, IHTTPDataIn* pDataIn, int timeout /*= HTTP_CLIENT_DEFAULT_TIMEOUT*/) //Blocking
{
    return connect(url, HTTP_PUT, (IHTTPDataOut*)&dataOut, pDataIn, timeout);
}

HTTPResult HTTPClient::del(const char* url, IHTTPDataIn* pDataIn, int timeout /*= HTTP_CLIENT_DEFAULT_TIMEOUT*/) //Blocking
{
    return connect(url, HTTP_DELETE, NULL, pDataIn, timeout);
}


#define CHECK_CONN_ERR(ret) \
  do{ \
    if(ret) { \
      m_sock.Close(); \
      Log::ERROR("Connection error: " +toString((int)ret) ); \
      return HTTP_CONN; \
    } \
  } while(0)

#define PRTCL_ERR() \
  do{ \
    m_sock.Close(); \
    return HTTP_PRTCL; \
  } while(0)

HTTPResult HTTPClient::connect(const char* url, HTTP_METH method, IHTTPDataOut* pDataOut, IHTTPDataIn* pDataIn, int timeout) //Execute request
{
    m_httpResponseCode = 0; //Invalidate code
    m_timeout = timeout;

    //Log::INFO("connect("+toString(url)+", " + toString(method) + ",...,"+ toString(timeout));
    pDataIn->writeReset();
    if( pDataOut ) {
        pDataOut->readReset();
    }

    char scheme[8];
    uint16_t port;
    char host[32];
    char path[MAXLEN_VALUE];
    size_t recvContentLength = 0;
    bool recvChunked = false;
    size_t crlfPos = 0;
    char buf[CHUNK_SIZE];
    size_t trfLen;
    int ret = 0;

    int maxRedirect = m_maxredirections;

    while (maxRedirect--) {
        bool takeRedirect = false;

        Log::INFO("parse: "+toString(url));
        //First we need to parse the url (http[s]://host[:port][/[path]]) -- HTTPS not supported (yet?)
        HTTPResult res = parseURL(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path));
        if(res != HTTP_OK) {
            Log::ERROR("parseURL returned: " +toString((int)res));
            return res;
        }

        if(port == 0) { //TODO do handle HTTPS->443
            port = 80;
        }

        //Log::DEBUG("Scheme: " + toString(scheme));
        //Log::DEBUG("Host: " + toString(host));
        //Log::DEBUG("Port: "+ toString(port));
        //Log::DEBUG("Path: "+ toString(path));

        //Connect
        Log::DEBUG("Connecting socket to server");
        ret = m_sock.Connect(host, port, timeout);
        if (ret < 0) {
            m_sock.Close();
            Log::ERROR("Could not connect");
            return HTTP_CONN;
        }

        // Send request
        Log::DEBUG("Sending request");
        const char* meth = (method==HTTP_GET)?"GET":(method==HTTP_POST)?"POST":(method==HTTP_PUT)?"PUT":(method==HTTP_DELETE)?"DELETE":"";
        snprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\nHost: %s:%d\r\n", meth, path, host, port); //Write request
        Log::INFO(" buf{"+ toString(buf) + "}");
        ret = send(buf);
        if (ret) {
            m_sock.Close();
            Log::ERROR("Could not write request");
            return HTTP_CONN;
        }

        // send authorization
        Log::INFO("send auth (if defined)");
        if (m_basicAuthUser && m_basicAuthPassword) {
            strcpy(buf, "Authorization: Basic ");
            createauth(m_basicAuthUser, m_basicAuthPassword, buf+strlen(buf), sizeof(buf)-strlen(buf));
            strcat(buf, "\r\n");
            //INFO(" (%s,%s) => (%s)", m_basicAuthUser, m_basicAuthPassword, buf);
            ret = send(buf);
            //INFO(" ret = %d", ret);
            if(ret) {
                m_sock.Close();
                Log::ERROR("Could not write request");
                return HTTP_CONN;
            }
        }

        // Send all headers
        //INFO("Send custom header(s) %d (if any)", m_nCustomHeaders);
        for (size_t nh = 0; nh < m_nCustomHeaders * 2; nh+=2) {
            //INFO("hdr[%2d] %s: %s", nh, m_customHeaders[nh], m_customHeaders[nh+1]);
            snprintf(buf, sizeof(buf), "%s: %s\r\n", m_customHeaders[nh], m_customHeaders[nh+1]);
            ret = send(buf);
            if (ret) {
                Log::ERROR("closing");
                usleep(50000);
                m_sock.Close();
                Log::ERROR("Could not write request");
                return HTTP_CONN;
            }
            Log::INFO("   send() returned " +toString(ret));
        }

        //Send default headers
        Log::DEBUG("Sending headers");
        if( pDataOut != NULL ) {
            if( pDataOut->getIsChunked() ) {
                ret = send("Transfer-Encoding: chunked\r\n");
                CHECK_CONN_ERR(ret);
            } else {
                snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", pDataOut->getDataLen());
                ret = send(buf);
                CHECK_CONN_ERR(ret);
            }
            char type[48];
            if( pDataOut->getDataType(type, 48) == HTTP_OK ) {
                snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", type);
                ret = send(buf);
                CHECK_CONN_ERR(ret);
            }
        }

        //Close headers
        Log::DEBUG("Headers sent");
        ret = send("\r\n");
        CHECK_CONN_ERR(ret);

        //Send data (if available)
        if( pDataOut != NULL ) {
        	Log::DEBUG("Sending data [" +toString(buf)+"]");
            while(true) {
                size_t writtenLen = 0;
                pDataOut->read(buf, CHUNK_SIZE, &trfLen);
                //Log::DEBUG("  trfLen: " + toString(trfLen));
                if( pDataOut->getIsChunked() ) {
                    //Write chunk header
                    char chunkHeader[16];
                    snprintf(chunkHeader, sizeof(chunkHeader), "%X\r\n", trfLen); //In hex encoding
                    ret = send(chunkHeader);
                    CHECK_CONN_ERR(ret);
                } else if( trfLen == 0 ) {
                    break;
                }
                if( trfLen != 0 ) {
                    ret = send(buf, trfLen);
                    CHECK_CONN_ERR(ret);
                }

                if( pDataOut->getIsChunked()  ) {
                    ret = send("\r\n"); //Chunk-terminating CRLF
                    CHECK_CONN_ERR(ret);
                } else {
                    writtenLen += trfLen;
                    if( writtenLen >= pDataOut->getDataLen() ) {
                        break;
                    }
                }

                if( trfLen == 0 ) {
                    break;
                }
            }

        }

        //Receive response
        Log::DEBUG("Receiving response");
        //ret = recv(buf, CHUNK_SIZE - 1, CHUNK_SIZE - 1, &trfLen); //Read n bytes
        ret = recv(buf, 1, CHUNK_SIZE - 1, &trfLen);    // recommended by Rob Noble to avoid timeout wait
        CHECK_CONN_ERR(ret);
        buf[trfLen] = '\0';
        Log::INFO("Received \r\n("+ toString(buf)+"\r\n)");

        char* crlfPtr = strstr(buf, "\r\n");
        if( crlfPtr == NULL) {
            PRTCL_ERR();
        }

        crlfPos = crlfPtr - buf;
        buf[crlfPos] = '\0';

        //Parse HTTP response
        if( sscanf(buf, "HTTP/%*d.%*d %d %*[^\r\n]", &m_httpResponseCode) != 1 ) {
            //Cannot match string, error
            Log::ERROR("Not a correct HTTP answer : {" +toString(buf)+ + "}\n");
            PRTCL_ERR();
        }

        if( (m_httpResponseCode < 200) || (m_httpResponseCode >= 400) ) {
            //Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers
            Log::INFO("Response code " + toString(m_httpResponseCode) );
            PRTCL_ERR();
        }

        Log::DEBUG("Reading headers");

        memmove(buf, &buf[crlfPos+2], trfLen - (crlfPos + 2) + 1); //Be sure to move NULL-terminating char as well
        trfLen -= (crlfPos + 2);

        recvContentLength = 0;
        recvChunked = false;
        //Now get headers
        while( true ) {
            crlfPtr = strstr(buf, "\r\n");
            if(crlfPtr == NULL) {
                if( trfLen < CHUNK_SIZE - 1 ) {
                    size_t newTrfLen = 0;
                    ret = recv(buf + trfLen, 1, CHUNK_SIZE - trfLen - 1, &newTrfLen);
                    trfLen += newTrfLen;
                    buf[trfLen] = '\0';
                    //Log::DEBUG("Read "+ toString(newTrfLen) + " chars; In buf: [" +buf+"]");
                    CHECK_CONN_ERR(ret);
                    continue;
                } else {
                    PRTCL_ERR();
                }
            }

            crlfPos = crlfPtr - buf;

            if(crlfPos == 0) { //End of headers
            	Log::DEBUG("Headers read");
                memmove(buf, &buf[2], trfLen - 2 + 1); //Be sure to move NULL-terminating char as well
                trfLen -= 2;
                break;
            }

            buf[crlfPos] = '\0';

            char key[61];
            char value[MAXLEN_VALUE];

            key[31] = '\0';
            value[MAXLEN_VALUE - 1] = '\0';

            int n = sscanf(buf, "%60[^:]: %160[^\r\n]", key, value);
            if ( n == 2 ) {
            	Log::DEBUG("Read header : " + toString(key) +":"+ toString(value));
                if( !strcmp(key, "Content-Length") ) {
                    sscanf(value, "%d", &recvContentLength);
                    pDataIn->setDataLen(recvContentLength);
                } else if( !strcmp(key, "Transfer-Encoding") ) {
                    if( !strcmp(value, "Chunked") || !strcmp(value, "chunked") ) {
                        recvChunked = true;
                        pDataIn->setIsChunked(true);
                    }
                } else if( !strcmp(key, "Content-Type") ) {
                    pDataIn->setDataType(value);
                } else if ( !strcmp(key, "Location") ) {
                    if (m_location) {
                        swFree(m_location);
                    }
                    m_location = (char *)swMalloc(strlen(value)+1);
                    if (m_location) {
                        strcpy(m_location,value);
                        url = m_location;
                        Log::INFO("Following redirect[" + toString(maxRedirect) + "] to [" + url + "]");
                        m_sock.Close();
                        takeRedirect = true;
                        break;   // exit the while(true) header to follow the redirect
                    } else {
                        Log::ERROR("Could not allocate memory for " + toString(key));
                    }
                }

                memmove(buf, &buf[crlfPos+2], trfLen - (crlfPos + 2) + 1); //Be sure to move NULL-terminating char as well
                trfLen -= (crlfPos + 2);
            } else {
            	Log::ERROR("Could not parse header");
                PRTCL_ERR();
            }

        } // while(true) // get headers
        if (!takeRedirect)
            break;
    } // while (maxRedirect)

    //Receive data
    //Log::DEBUG("Receiving data");
    while(true) {
        size_t readLen = 0;

        if( recvChunked ) {
            //Read chunk header
            bool foundCrlf;
            do {
                foundCrlf = false;
                crlfPos=0;
                buf[trfLen]=0;
                if(trfLen >= 2) {
                    for(; crlfPos < trfLen - 2; crlfPos++) {
                        if( buf[crlfPos] == '\r' && buf[crlfPos + 1] == '\n' ) {
                            foundCrlf = true;
                            break;
                        }
                    }
                }
                if(!foundCrlf) { //Try to read more
                    if( trfLen < CHUNK_SIZE ) {
                        size_t newTrfLen = 0;
                        ret = recv(buf + trfLen, 0, CHUNK_SIZE - trfLen - 1, &newTrfLen);
                        trfLen += newTrfLen;
                        CHECK_CONN_ERR(ret);
                        continue;
                    } else {
                        PRTCL_ERR();
                    }
                }
            } while(!foundCrlf);
            buf[crlfPos] = '\0';
            int n = sscanf(buf, "%x", &readLen);
            if(n!=1) {
            	Log::ERROR("Could not read chunk length");
                PRTCL_ERR();
            }

            memmove(buf, &buf[crlfPos+2], trfLen - (crlfPos + 2)); //Not need to move NULL-terminating char any more
            trfLen -= (crlfPos + 2);

            if( readLen == 0 ) {
                //Last chunk
                break;
            }
        } else {
            readLen = recvContentLength;
        }

        Log::DEBUG("Retrieving " +toString(readLen) + " bytes");

        do {
            Log::INFO("write " + toString(trfLen) + "," +toString(readLen) + ":" + buf);
            pDataIn->write(buf, MIN(trfLen, readLen));
            if( trfLen > readLen ) {
                memmove(buf, &buf[readLen], trfLen - readLen);
                trfLen -= readLen;
                readLen = 0;
            } else {
                readLen -= trfLen;
            }

            if(readLen) {
                ret = recv(buf, 1, CHUNK_SIZE - trfLen - 1, &trfLen);
                CHECK_CONN_ERR(ret);
                Log::INFO("recv'd next chunk ret: " +toString(ret) );
            }
        } while(readLen);

        if( recvChunked ) {
            if(trfLen < 2) {
                size_t newTrfLen;
                //Read missing chars to find end of chunk
                Log::INFO("read chunk");
                ret = recv(buf + trfLen, 2 - trfLen, CHUNK_SIZE - trfLen - 1, &newTrfLen);
                CHECK_CONN_ERR(ret);
                trfLen += newTrfLen;
                Log::INFO("recv'd next chunk ret: " +toString(ret));
            }
            if( (buf[0] != '\r') || (buf[1] != '\n') ) {
                Log::ERROR("Format error");
                PRTCL_ERR();
            }
            memmove(buf, &buf[2], trfLen - 2);
            trfLen -= 2;
        } else {
            break;
        }
    }
    m_sock.Close();
    Log::DEBUG("Completed HTTP transaction");
    return HTTP_OK;
}

HTTPResult HTTPClient::recv(char* buf, size_t minLen, size_t maxLen, size_t* pReadLen) //0 on success, err code on failure
{
    //Log::DEBUG("Trying to read between " + toString(minLen) + " and " +toString(maxLen) + " bytes");
    size_t readLen = 0;

    if (!m_sock.Is_connected()) {
        Log::INFO("Connection was closed by server");
        return HTTP_CLOSED; //Connection was closed by server
    }

    int ret;
    while (readLen < maxLen) {
        if (readLen < minLen) {
            //DBG("Trying to read at most %4d bytes [not Blocking, %d] %d,%d", minLen - readLen, m_timeout, minLen, readLen);
            m_sock.Set_blocking(false, m_timeout);
            ret = m_sock.Receive_all(buf + readLen, minLen - readLen);
        } else {
            //DBG("Trying to read at most %4d bytes [Not blocking, %d] %d,%d", maxLen - readLen, 0, maxLen, readLen);
            m_sock.Set_blocking(false, 0);
            ret = m_sock.Receive(buf + readLen, maxLen - readLen);
        }

        if (ret > 0) {
            readLen += ret;
        } else if ( ret == 0 ) {
            break;
        } else {
            if (!m_sock.Is_connected()) {
                Log::ERROR("Connection error (recv returned " +toString(ret) +")");
                *pReadLen = readLen;
                return HTTP_CONN;
            } else {
                break;
            }
        }
        if (!m_sock.Is_connected()) {
            break;
        }
    }
    Log::DEBUG("Read " +toString(readLen) + " bytes");
    buf[readLen] = '\0';    // DS makes it easier to see what's new.
    *pReadLen = readLen;
    return HTTP_OK;
}

HTTPResult HTTPClient::send(string str)
{
	size_t len = str.size();
	return send((char*)str.c_str(), len);
}


HTTPResult HTTPClient::send(char* buf, size_t len) //0 on success, err code on failure
{
    if(len == 0) {
        len = strlen(buf);
    }
    Log::DEBUG("send(\r\n"+ toString(buf) + "," + toString(len) + ")");
    size_t writtenLen = 0;

    if(!m_sock.Is_connected()) {
        Log::INFO("Connection was closed by server");
        return HTTP_CLOSED; //Connection was closed by server
    }
    //Log::DEBUG("a");
    m_sock.Set_blocking(false, m_timeout);
    //Log::DEBUG("b");
    int ret = m_sock.Send_all(buf, len);
    if(ret > 0) {
        writtenLen += ret;
    } else if( ret == 0 ) {
    	Log::INFO("Connection was closed by server");
        return HTTP_CLOSED; //Connection was closed by server
    } else {
    	Log::ERROR("Connection error (send returned " +toString(ret) + ")");
        return HTTP_CONN;
    }
    Log::DEBUG("Written " +toString(writtenLen) + " bytes");
    return HTTP_OK;
}

HTTPResult HTTPClient::parseURL(const char* url, char* scheme, size_t maxSchemeLen, char* host, size_t maxHostLen, uint16_t* port, char* path, size_t maxPathLen) //Parse URL
{
    char* schemePtr = (char*) url;
    char* hostPtr = (char*) strstr(url, "://");
    //INFO("parseURL(%s,%p,%d,%s,%d,%d,%p,%d",url, scheme, maxSchemeLen, host, maxHostLen, *port, path, maxPathLen);
    if (hostPtr == NULL) {
        Log::INFO("Could not find host");
        return HTTP_PARSE; //URL is invalid
    }

    if ( (uint16_t)maxSchemeLen < hostPtr - schemePtr + 1 ) { //including NULL-terminating char
        //WARN("Scheme str is too small (%d >= %d)", maxSchemeLen, hostPtr - schemePtr + 1);
        return HTTP_PARSE;
    }
    memcpy(scheme, schemePtr, hostPtr - schemePtr);
    scheme[hostPtr - schemePtr] = '\0';

    hostPtr+=3;

    size_t hostLen = 0;

    char* portPtr = strchr(hostPtr, ':');
    if( portPtr != NULL ) {
        hostLen = portPtr - hostPtr;
        portPtr++;
        if( sscanf(portPtr, "%hu", port) != 1) {
        	Log::INFO("Could not find port");
            return HTTP_PARSE;
        }
    } else {
        *port=0;
    }
    Log::INFO("  hostPtr: " + toString(hostPtr));
    Log::INFO("  hostLen: " +toString(hostLen));
    char* pathPtr = strchr(hostPtr, '/');
    if( hostLen == 0 ) {
        hostLen = pathPtr - hostPtr;
    }

    if( maxHostLen < hostLen + 1 ) { //including NULL-terminating char
        //WARN("Host str is too small (%d >= %d)", maxHostLen, hostLen + 1);
        return HTTP_PARSE;
    }
    memcpy(host, hostPtr, hostLen);
    host[hostLen] = '\0';

    size_t pathLen;
    char* fragmentPtr = strchr(hostPtr, '#');
    if(fragmentPtr != NULL) {
        pathLen = fragmentPtr - pathPtr;
    } else {
        pathLen = strlen(pathPtr);
    }

    if( maxPathLen < pathLen + 1 ) { //including NULL-terminating char
        //WARN("Path str is too small (%d >= %d)", maxPathLen, pathLen + 1);
        return HTTP_PARSE;
    }
    memcpy(path, pathPtr, pathLen);
    path[pathLen] = '\0';

    return HTTP_OK;
}

void HTTPClient::createauth (const char *user, const char *pwd, char *buf, int len)
{
    char tmp[80];

    snprintf(tmp, sizeof(tmp), "%s:%s", user, pwd);
    base64enc(tmp, strlen(tmp), &buf[strlen(buf)], len - strlen(buf));
}

// Copyright (c) 2010 Donatien Garnier (donatiengar [at] gmail [dot] com)
int HTTPClient::base64enc(const char *input, unsigned int length, char *output, int len)
{
    static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int c, c1, c2, c3;

    if ((uint16_t)len < ((((length-1)/3)+1)<<2)) return -1;
    for(unsigned int i = 0, j = 0; i<length; i+=3,j+=4) {
        c1 = ((((unsigned char)*((unsigned char *)&input[i]))));
        c2 = (length>i+1)?((((unsigned char)*((unsigned char *)&input[i+1])))):0;
        c3 = (length>i+2)?((((unsigned char)*((unsigned char *)&input[i+2])))):0;

        c = ((c1 & 0xFC) >> 2);
        output[j+0] = base64[c];
        c = ((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4);
        output[j+1] = base64[c];
        c = ((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6);
        output[j+2] = (length>i+1)?base64[c]:'=';
        c = (c3 & 0x3F);
        output[j+3] = (length>i+2)?base64[c]:'=';
    }
    output[(((length-1)/3)+1)<<2] = '\0';
    return 0;
}





int HTTPClient::getHTTPResponseCode()
{
    return m_httpResponseCode;
}

void HTTPClient::setMaxRedirections(int i)
{
    if (i < 1)
        i = 1;
    m_maxredirections = i;
}


