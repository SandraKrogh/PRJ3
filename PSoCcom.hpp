#include "Serial.hpp"
#include <unistd.h>
#include <iostream>
#include <cstring>

class PSoCcom
{
    public:

    PSoCcom() : mySerial_(new Serial)
    {

    }
    ~PSoCcom()
    {
        delete mySerial_;
    }
    
    void startSpil(std::string spil, std::string antalSpillere) //sender data til PSoC over UART
    { 
        std::cout << "Start spil kaldt" << std::endl;
        std::string spilTemp;
        
        if(spil == "Fisk")
        {
            spilTemp = "1";
        }
        if(spil == "500")
        {
            spilTemp = "2";
        }
        if(spil == "Røvhul")
        {
            spilTemp = "3";
        }
        
        char message[5];
        std::string message_temp= "R"+spilTemp+antalSpillere+"C";

        strcpy(message,message_temp.c_str());
        
        size_t n = sprintf(buf, message);

        std::cout << "Det der er i bufferen " << buf << std::endl;

        mySerial_->writeSome(buf, n);

        //clear buffer?

    }

    void test()
    {
        std::cout << "Indholdet af bufferen: " << buf << std::endl;
    }

    private:
    Serial *mySerial_;
    char buf[5];
};





