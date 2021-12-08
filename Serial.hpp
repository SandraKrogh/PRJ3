#include <boost/asio/io_context.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/asio/serial_port.hpp>
#include <string>

class Serial
{
public:
  Serial(uint32_t baudRate=57600, std::string portName="/dev/ttyS0")
    : serialPort_(io_, portName.c_str())
  {
    typedef boost::asio::serial_port_base spb;
    serialPort_.set_option(spb::baud_rate(baudRate));
  }
  
  size_t readSome(char* buf, size_t n)
  {
    return serialPort_.read_some(boost::asio::buffer(buf, n));
  }

  size_t readCString(char* buf, size_t maxSize)
  {
    size_t n = 0;
    size_t r = 0;
    do
      {
        r = readSome(buf + n, 1);
      }
    while ((r == 1) &&  (buf[n] != 0x00) && (n < maxSize));

    if(n == maxSize) buf[n-1] = '\0'; // Ensure zero termination
    return r == 1 ? n : r;
  }
  
  
  size_t writeSome(const char* buf, size_t n)
  {
    return serialPort_.write_some(boost::asio::buffer(buf, n));
  }
  
  size_t writeSome(const char* buf, bool incZero = true) // For zero terminated strings
  {
    size_t n = strlen(buf);
    if(incZero) ++n;
    
    return serialPort_.write_some(boost::asio::buffer(buf, n));
  }
  
private:
    boost::asio::io_context io_;
    boost::asio::serial_port serialPort_;
};