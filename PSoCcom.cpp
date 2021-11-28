

class PSoCcom
{
    public:

    PSoCcom();
    void Initaliser(); //Sender start signal 
    void startSpil(data); //sender data til PSoC over UART

    private:
    data //skal være et object
}

void PSoCcom::startSpil(data)
{   
    //pak data antal spillere + spil ud 
    
    //kalder UART funktion UART.send(antal spillere + spil) - protokol
}

Initaliser()
{
    //UART.send(start signal)
}


