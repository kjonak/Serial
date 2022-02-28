#pragma once
#include <Windows.h>
#include <vector>
#include <string>


namespace Serial{
	enum class Parameters
	{

		ONE_STOP_BIT = 0,
		TWO_STOP_BITS = 2,
		NO_PARITY = 0,
		ODD_PARITY = 1,
		EVEN_PARITY = 2

	};

	typedef struct _SerialParameters
	{
		Parameters Parity;
		Parameters StopBits;
		uint32_t BaudRate;
		uint8_t ByteSize;
	}SerialParameter_t;

	void SerialParametersInit(SerialParameter_t& parameters, uint32_t BaudRate, uint8_t ByteSize, Parameters StopBits, Parameters Parity);
	std::vector<std::string> GetAvailablePorts();

class Connection
{
private:
	bool m_Connected = false;
	bool m_ForceStop = false;
	bool m_IsReading = false;
	DCB		m_serialParams;;
	HANDLE	m_IO_Handle;
	COMSTAT m_PortStat;

public:
	Connection()
		:m_IO_Handle(NULL), m_PortStat({ 0 }), m_serialParams({0}) {}
	~Connection();
	void Disconnect();
	bool Connect(std::string PortName, SerialParameter_t* SerialParameters);
	unsigned int Write(uint8_t* data, unsigned int length);
	unsigned int ReadPendingBytes(uint8_t* buffer, unsigned int length);
	void StopReading();
	bool CheckConnection();
	void Flush();
	void StartAutoConnectionCheck();
};

}