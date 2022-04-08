#include <Windows.h>
#include <iostream>
#include <vector>
#include "Serial.h"


namespace Serial{

    void  SerialParametersInit(SerialParameter_t& parameters, uint32_t BaudRate, uint8_t ByteSize, Parameters StopBits, Parameters Parity)
    {
        parameters.BaudRate = BaudRate;
        parameters.StopBits = StopBits;
        parameters.Parity = Parity;
        parameters.ByteSize = ByteSize;
    }

   std::vector<std::string> GetAvailablePorts()
    {
        std::vector<std::string> listOfPorts;
        
        unsigned char* lpTargetPath[1000]; // buffer to store the path of the COMPORTS
        unsigned long test;

        for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
        {
            std::string PortName;
            PortName = "COM" + std::to_string(i);
           

            test = QueryDosDevice(PortName.c_str(), (LPSTR)lpTargetPath, 5000);

            // Test the return value and error if any
            if (test != 0) //QueryDosDevice returns zero if it didn't find an object
            {
                listOfPorts.push_back(PortName);
            }

        }


        return listOfPorts;
    }


  


Connection::~Connection()
{
    Disconnect();
}

void Connection::Disconnect()
{

    if(m_Connected)
    {
        m_Connected = false;
    CloseHandle(m_IO_Handle);
    }
}
bool Connection::Connect(std::string PortName, SerialParameter_t* SerialParameters)
{
    m_Connected = false;
    m_IO_Handle = CreateFile(PortName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!m_IO_Handle)
    {
        std::cout << "Can not create serial handle" << std::endl;
        return false;
    }
    
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(m_IO_Handle, &timeout))
    {
        std::cout << "Could not set ports timeout" << std::endl;
        return false;
    }
    
    memset(&m_serialParams, 0, sizeof(m_serialParams));
    GetCommState(m_IO_Handle, &m_serialParams);
    m_serialParams.DCBlength = sizeof(m_serialParams);
    m_serialParams.BaudRate =SerialParameters->BaudRate;
    m_serialParams.StopBits =(BYTE) SerialParameters->StopBits;
    m_serialParams.Parity =  (BYTE)SerialParameters->Parity;
    m_serialParams.ByteSize = 8;// SerialParameters->ByteSize;

    if (!SetCommState(m_IO_Handle, &m_serialParams))
    {
        std::cout << "Could not set ports parameters" << std::endl;
        return false;
    }
    else {
        m_Connected = true;
        PurgeComm(m_IO_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }

  
    return true;
}


unsigned int Connection::Write(uint8_t* data, unsigned int length)
{
    bool state = false;
    if (!m_Connected)
        return 0;
    
 
    unsigned long written_count;
    //create overlapped event

    if (!WriteFile(m_IO_Handle, data, length, &written_count, NULL))
    {
        std::cout << "Error while writting" << std::endl;
    }
    else
    {
        state = true;
    }

    if (state)
        return written_count;
    else return 0;
}

bool Connection::CheckConnection()
{
    if (m_Connected)
    {
        unsigned long t;
        bool v = GetCommModemStatus(m_IO_Handle, &t);
        if (v)
            return true;
        else
        {
            m_Connected = false;
            return false;
        }
    }
    return false;
}
bool Connection::ReadByte(uint8_t *buffer)
{
    DWORD l = 0;
    if (m_Connected)
    {
        ReadFile(m_IO_Handle, buffer, 1, &l, 0);
    }
    if(l==0)
        return false;
    else 
        return true;
    
}
void Connection::Flush()
{
    if (m_Connected)
    {
        PurgeComm(m_IO_Handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

unsigned int Connection::ReadPendingBytes(uint8_t* buffer, unsigned int bufferSize)
{
    bool readComplete = false;
    unsigned long bytes_received;
    unsigned long comm_event = 0;
    if (!SetCommMask(m_IO_Handle, EV_RXCHAR))
    {
        std::cout << "Error while creating interrupt mask" << std::endl;
    }

 
    if (WaitCommEvent(m_IO_Handle, &comm_event, NULL))
    {
     
        // Issue read operation.
        if (!ReadFile(m_IO_Handle, buffer, bufferSize, &bytes_received, 0))
        {
            std::cout << "Error while reading serial" << std::endl;
        }
        else
        {
            readComplete = true;
        }
        
        
    }
   
    if (readComplete)
        return bytes_received;
    else return 0;
}
void Connection::StopReading()
{
    if (!m_Connected)
        return;
    m_ForceStop = true;
    if(m_IsReading)
    if (!SetCommMask(m_IO_Handle, (unsigned long)0))
    {
        std::cout << "Error setting intterupt mask" << std::endl;
    }
}

}