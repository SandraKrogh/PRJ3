#include <boost/asio/io_context.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/asio/serial_port.hpp>
#include <string>
#include <unistd.h>
#include <iostream>

class Serial
{
public:
  Serial(uint32_t baudRate, std::string portName)
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


int main(int argc, char* argv[])
{
 
std::cout << "Programmet kører" << std::endl; 
  Serial s(57600, "/dev/ttyS0");

  char buf[5];

  for(;;)
  {  
  char besked[5];

  std::cout << "Indtast 4 tegn:" << std::endl;
  std::cin.getline(besked, 5);
  
  size_t n = sprintf(buf, besked);

  s.writeSome(buf, n);
  sleep(5);

  std::cin.sync();
  
  }

  //s.readCString(buf, 256);  

  return 0;
}