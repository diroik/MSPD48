#ifndef IHTTPDATA_H
#define IHTTPDATA_H

#include <cstring>

using std::size_t;

///This is a simple interface for HTTP data storage (impl examples are Key/Value Pairs, File, etc...)
class IHTTPDataOut
{
protected:
  friend class HTTPClient;


  virtual ~IHTTPDataOut() {}

  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
  virtual void readReset() = 0;

  /** Read a piece of data to be transmitted
   * @param[out] buf Pointer to the buffer on which to copy the data
   * @param[in] len Length of the buffer
   * @param[out] pReadLen Pointer to the variable on which the actual copied data length will be stored
   */
  virtual int read(char* buf, size_t len, size_t* pReadLen) = 0;

  /** Get MIME type
   * @param[out] type Internet media type from Content-Type header
   * @param[in] maxTypeLen is the size of the type buffer to write to
   */
  virtual int getDataType(char* type, size_t maxTypeLen) = 0; //Internet media type for Content-Type header

  /** Determine whether the HTTP client should chunk the data
   *  Used for Transfer-Encoding header
   */
  virtual bool getIsChunked() = 0;

  /** If the data is not chunked, get its size
   *  Used for Content-Length header
   */
  virtual size_t getDataLen() = 0;

};

///This is a simple interface for HTTP data storage (impl examples are Key/Value Pairs, File, etc...)
class IHTTPDataIn
{
protected:
  friend class HTTPClient;

  virtual ~IHTTPDataIn()
  {}


  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
  virtual void writeReset() = 0;

  /** Write a piece of data transmitted by the server
   * @param buf Pointer to the buffer from which to copy the data
   * @param len Length of the buffer
   */
  virtual int write(const char* buf, size_t len) = 0;

  /** Set MIME type
   * @param type Internet media type from Content-Type header
   */
  virtual void setDataType(const char* type) = 0;

  /** Determine whether the data is chunked
   *  Recovered from Transfer-Encoding header
   */
  virtual void setIsChunked(bool chunked) = 0;

  /** If the data is not chunked, set its size
   * From Content-Length header
   */
  virtual void setDataLen(size_t len) = 0;

};

#endif
