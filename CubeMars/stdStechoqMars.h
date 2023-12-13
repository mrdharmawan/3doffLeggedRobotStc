#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

enum mode {
	COMM_FW_VERSION = 0,
	COMM_JUMP_TO_BOOTLOADER,
	COMM_ERASE_NEW_APP,
	COMM_WRITE_NEW_APP_DATA,
	COMM_GET_VALUES, //Get motor running parameters
	COMM_SET_DUTY, //Motor runs in duty cycle mode
	COMM_SET_CURRENT, //Motor runs in current loop mode
	COMM_SET_CURRENT_BRAKE, //Motor current brake mode operation
	COMM_SET_RPM, //Motor runs in current loop mode
	COMM_SET_POS, //Motor runs in position loop mode
	COMM_SET_HANDBRAKE, //Motor runs in handbrake current loop mode
	COMM_SET_DETECT, //Motor real-time feedback current position command
	COMM_ROTOR_POSITION = 22,//Motor feedback current position
	COMM_GET_VALUES_SETUP = 50,//Motor single or multiple parameter acquisition instructions
	COMM_SET_POS_SPD = 91, // Motor runs in position velocity loop mode
	COMM_SET_POS_MULTI = 92, // Set the motor movement to single-turn mode
	COMM_SET_POS_SINGLE = 93, // Set the motor motion to multi-turn mode, the range is �100 turns
	COMM_SET_POS_UNLIMITED = 94, //Save
	COMM_SET_POS_ORIGIN = 95, //Set the motor origin
};



const int ARDUINO_WAIT_TIME = 2000;




class Mars
{
public:
	////////////////////////////////////////////////////////
	// Default Constructor
	Mars();

	////////////////////////////////////////////////////////
	// Initialization/Admin Stuff
	void setPort(unsigned int _comPortNum);

	void begin(unsigned int _baud, unsigned int _comPortNum);
	bool connected();
	void end();

	////////////////////////////////////////////////////////
	// Random Stuff
	unsigned int available();
	operator bool();

	////////////////////////////////////////////////////////
	// I/O Functions
	int read();
	int read(uint8_t buffer[], unsigned int bufSize);
	bool print(const std::string message);
	bool print(const long double message);
	bool print(const long message);

	bool write(char c);
	bool write(const char buffer[], unsigned int bufSize);
	
	bool Update();

	//Observable Variable
	double Current;
	double Duty_Cycle;
	double TempMotor;
	double TempFet;
	double ERPM;
	double Inductance;
	double yPos;
	double Voltage;

	//Control Variable
	double xPos;
	double xSpd;
	double xAcc;
	double xTorq;
	double xCurr;
	double xDuty;
	double HandBrakeCurr;

	//Mode Variable
	uint16_t _mode;
	int successco = 0;
	int faultco = 0;

	void getParam();
	void setPos();
	void setCurr();
	void setSpd();
	void getStream();
	void setDuty();
	void flush();
	void setPosSpd();
	void setOrigin();
	void setHandBrake();

	bool baaaa;

	//YParam
	float MOSTempC;// = (float)append16(buf, &ind) / 10.0;
	float MotorTempC;// = (float)append16(buf, &ind) / 10.0;
	float Outputcurrent;// = (float)append32(buf, &ind) / 100.0;
	float Inputcurrent;// = (float)append32(buf, &ind) / 100.0;
	float Idcurrent;// = (float)append32(buf, &ind) / 100.0;
	float Iqcurrent;// = (float)append32(buf, &ind) / 100.0;
	float Motorthrottlevalue;// = (float)append16(buf, &ind) / 1000.0;
	float Motorspeed;// = (float)append32(buf, &ind);
	float Iinputvoltage;// = (float)append16(buf, &ind) / 10.0;
	float Motorouterringposition;// = (float)append32(buf, &ind) / 1000000.0;
	float MotorIDnumber;// = buf[(ind)++];
	float MotorVdvoltage;// = (float)append32(buf, &ind) / 1000.0;
	float MotorVqvoltage;// = (float)append32(buf, &ind) / 1000.0;

	bool getstream[32];
	uint8_t sizevar[32] = { 2,2,4,4,4,4,2,4,2,0,0,0,0,0,0,1,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	double scaledata[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1000000,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	double datate[32];
private:
	HANDLE handler;
	bool isConnected;
	COMSTAT status;
	DWORD errors;
	
	void begin(unsigned int _baud);

	uint8_t parseState, yparseState;

	unsigned int baud = CBR_9600;
	unsigned int comPortNum = 0;

	void initializePort(char portName[], unsigned int _baud);
	bool ParseIn();
	bool ParseOut();
	unsigned short crc16(unsigned char* buf, unsigned int len);
	union Crc
	{
		std::uint8_t _8[2];
		std::uint16_t _16;
	};
	Crc crc,crcparsein;
	union tigadua
	{
		std::uint32_t _u32;
		std::uint8_t _8[4];
		std::uint16_t _16[2];
		std::int32_t _32;
	};

	bool UpdateVal(unsigned char* buf, unsigned int len);
	int32_t  buffer_get_int132(uint8_t buf[255], unsigned int len);
};




extern Mars Motor[10];