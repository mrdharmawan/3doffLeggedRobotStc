//STANDARD LIBRARY HEADER FOR CUBEMARS AK10-9
//Use for handle UART communication
//Author : Hanif Baskoro
//Env : Windows 10
//First : 20 May 2023
//Last : 20 May 2023
#include "stdStechoqMars.h"
#include <iostream>
#include <ctime>
#include <bitset>
//#include "hlpStechoqAppend.h"
#pragma warning(disable: 4996)


union uniconv16 {
	uint16_t _16;
	uint8_t _8[2];
};
uniconv16 conv16;
uint16_t append16(uint8_t buff[255], int* idx) {

	conv16._8[1] = buff[(*idx)++];

	conv16._8[0] = buff[(*idx)++];
	//std::cout << "aaa16" << (uint16_t)conv16._8[0] << std::endl;
	//std::cout << (uint16_t)conv16._8[1] << std::endl;
	//std::cout << "wooooy" << conv16._16 << std::endl;
	return conv16._16;
}
union uniconv32 {
	uint32_t _u32;
	int32_t _32;
	uint8_t _8[4];
};
uniconv32 conv32;
int32_t append32(uint8_t buff[255], int* idx) {

	conv32._8[3] = buff[(*idx)++];

	conv32._8[2] = buff[(*idx)++];
	conv32._8[1] = buff[(*idx)++];
	conv32._8[0] = buff[(*idx)++];
	//std::cout << "aaa" << (uint16_t)conv32._8[0] << std::endl;
	//std::cout << (uint16_t)conv32._8[1] << std::endl;
	//std::cout << (uint16_t)conv32._8[2] << std::endl;
	//std::cout << (uint16_t)conv32._8[3] << std::endl;
	//std::cout << "wooooy" << conv32._32 << std::endl;
	return conv32._32;
}

Mars::Mars()
{
//this->setPort(_comPortNum);
}




void Mars::setPort(unsigned int _comPortNum)
{
this->comPortNum = _comPortNum;
}




void Mars::begin(unsigned int _baud)
{
	
this->baud = _baud;

if (this->comPortNum != NULL)
{
char format[] = "\\\\.\\COM%i";
char portName[20];

snprintf(portName, 12, format, this->comPortNum);

initializePort(portName, this->baud);
}
}




void Mars::begin(unsigned int _baud, unsigned int _comPortNum)
{
	

char format[] = "\\\\.\\COM%i";
char portName[20];

this->baud = _baud;

setPort(_comPortNum);
snprintf(portName, 12, format, this->comPortNum);

initializePort(portName, this->baud);
}

//WINDOWS ONLY SECTION
// 
//Replace This Section if Program need to be run in other Environtment

void Mars::initializePort(char portName[], unsigned int _baud)
{
this->isConnected = false;

this->handler = CreateFileA(static_cast<LPCSTR>(portName), 
GENERIC_READ | GENERIC_WRITE,  
0,                             
NULL,                          
OPEN_EXISTING,                 
FILE_ATTRIBUTE_NORMAL,        
NULL);                        

if (this->handler == INVALID_HANDLE_VALUE)
{
std::cout<<"Failed Opennn"<<std::endl;
}
else
{
DCB dcbSerialParameters = { 0 };

if (!GetCommState(this->handler, &dcbSerialParameters))
printf("failed to get current serial parameters");
else
{
dcbSerialParameters.BaudRate = _baud;
dcbSerialParameters.ByteSize = 8;
dcbSerialParameters.StopBits = ONESTOPBIT;
dcbSerialParameters.Parity = NOPARITY;
dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

if (!SetCommState(handler, &dcbSerialParameters))
	printf("ALERT: could not set Serial port parameters\n");
else
{
	this->isConnected = true;

	// flush any remaining characters in the buffers 
	PurgeComm(this->handler, PURGE_RXABORT |
		PURGE_RXCLEAR |
		PURGE_TXABORT |
		PURGE_TXCLEAR);

	// wait as the arduino board will be reseting
	Sleep(ARDUINO_WAIT_TIME);
}
}
}
}


void Mars::flush() {
	DCB dcbSerialParameters = { 0 };

	if (!GetCommState(this->handler, &dcbSerialParameters))
		printf("failed to get current serial parameters");
	else
	{
		

			// flush any remaining characters in the buffers 
			PurgeComm(this->handler, PURGE_RXABORT |
				PURGE_RXCLEAR |
				PURGE_TXABORT |
				PURGE_TXCLEAR);

			// wait as the arduino board will be reseting
			Sleep(ARDUINO_WAIT_TIME);
		
	}
}

void Mars::end()
{
if (this->isConnected)
{
this->isConnected = false;
CloseHandle(this->handler);
}
}




unsigned int Mars::available()
{
ClearCommError(this->handler, &this->errors, &this->status);
return this->status.cbInQue;
}




Mars::operator bool()
{
return this->connected();
}




int Mars::read()
{
DWORD bytesRead;
unsigned int toRead = 1;
unsigned int bytesAvailable = this->available();
char buffer[] = { ' ' };

if (bytesAvailable)
if (ReadFile(this->handler, buffer, toRead, &bytesRead, NULL)) {
//std::cout << (int)buffer<<std::endl;
//std::cout<<"bytesRead " << sizeof(buffer) / sizeof(char) <<std::endl;
return buffer[0];
}

return -1;
}

int Mars::read(uint8_t buffer[255], unsigned int bufSize)
{
DWORD bytesRead;
unsigned int toRead;
unsigned int bytesAvailable = this->available();

if (bytesAvailable > bufSize)
toRead = bufSize;
else
toRead = bytesAvailable;

if (ReadFile(this->handler, buffer, toRead, &bytesRead, NULL))
return bytesRead;

return 0;
}





//int Mars::

bool Mars::print(const std::string message)
{
DWORD bytesSend;
size_t bufSize = message.size();

char* buffer = new char[message.size() + 1];
message.copy(buffer, message.size() + 1);
buffer[message.size()] = '\0';

if (!WriteFile(this->handler, buffer, bufSize, &bytesSend, 0))
{
ClearCommError(this->handler, &this->errors, &this->status);
return false;
}
else
return true;
}




bool Mars::print(const long double message)
{
DWORD bytesSend;
char buffer[100];

unsigned int bufSize = sprintf(buffer, "%f", message);

if (!WriteFile(this->handler, buffer, bufSize, &bytesSend, 0))
{
ClearCommError(this->handler, &this->errors, &this->status);
return false;
}
else
return true;
}




bool Mars::print(const long message)
{
DWORD bytesSend;
char buffer[100];

unsigned int bufSize = sprintf(buffer, "%i", message);

if (!WriteFile(this->handler, buffer, bufSize, &bytesSend, 0))
{
ClearCommError(this->handler, &this->errors, &this->status);
return false;
}
else
return true;
}


bool Mars::write(char c)
{
DWORD bytesSend = 1;
unsigned int buf_size = 1;
char buffer[] = { c };

if (!WriteFile(this->handler, buffer, buf_size, &bytesSend, 0))
{
ClearCommError(this->handler, &this->errors, &this->status);
return false;
}
else
return true;
}

bool Mars::write(const char buffer[], unsigned int bufSize)
{
DWORD bytesSend;
//for (int i = 0; i < bufSize;i++) {
//	std::cout <<" TO WRITE " << (int)buffer[i] << std::endl;
//}

if (!WriteFile(this->handler, buffer, bufSize, &bytesSend, 0))
{
ClearCommError(this->handler, &this->errors, &this->status);
return false;
}
else
return true;
}

bool Mars::connected()
{
if (this->isConnected)
return this->isConnected;
else
this->begin(this->baud);

return this->isConnected;
}

int32_t  Mars::buffer_get_int132(uint8_t buf[255],unsigned int len) {
	return 1000;
}

//END OF WINDOWS ONLY SECTION



bool Mars::UpdateVal(uint8_t buf[255], unsigned int len)
{
	uint8_t upValState = 0;
	bool Err = false, TimeOut = false;
	const clock_t begin_time = clock();
	while (!Err && !TimeOut) {
		if ((float(clock() - begin_time) / CLOCKS_PER_SEC) > 1) {
			TimeOut = true;
		}
		switch (upValState) 
		{
			case 0:					//Mode Select
			{


				//std::cout << "FRAME TO UPDATE " << (unsigned int)buf[0] << std::endl;
				switch (buf[0])
				{
					case COMM_FW_VERSION://= 0,
					{

						break;
					}
					case COMM_JUMP_TO_BOOTLOADER://,1
					{
						break;
					}
					case COMM_ERASE_NEW_APP://,2
					{
						break;
					}
					case COMM_WRITE_NEW_APP_DATA://,3
					{
						break;
					}
					case COMM_GET_VALUES://, //Get motor running parameters		4
					{
						int ind = 1;
						baaaa = true;
						MOSTempC = (float)append16(buf, &ind) / 10.0;
						MotorTempC = (float)append16(buf, &ind) / 10.0;
						Outputcurrent = (float)append32(buf, &ind) *10;
						Inputcurrent = (float)append32(buf, &ind) / 100.0;
						Idcurrent = (float)append32(buf, &ind) / 100.0;
						Iqcurrent = (float)append32(buf, &ind) / 100.0;
						Motorthrottlevalue = (float)append16(buf, &ind) / 1000.0;
						Motorspeed = (float)append32(buf, &ind);
						Iinputvoltage = (float)append16(buf, &ind) / 10.0;
						Motorouterringposition = (float)append32(buf, &ind);// / 1000000.0;
						MotorIDnumber = buf[(ind)++];
						MotorVdvoltage = (float)append32(buf, &ind) / 1000.0;
						MotorVqvoltage = (float)append32(buf, &ind) / 1000.0;
						std::cout << " MOSTempC : " << MOSTempC << " MotorTempC : " << MotorTempC << " OutputCurr : " << Outputcurrent<<" " << Inputcurrent
							<< " " << Idcurrent
							<< " " << Iqcurrent
							<< " " << Motorthrottlevalue
							<< " Motor Speed " << Motorspeed
							<< " " << Iinputvoltage
							<< " Motor Pos " << Motorouterringposition 
							<< " " << MotorIDnumber
							<< " " << MotorVdvoltage
							<< " " << MotorVqvoltage
							<< std::endl;
						break;
					}
					case COMM_SET_DUTY://, //Motor runs in duty cycle mode
					{
						break;
					}
					case COMM_SET_CURRENT://, //Motor runs in current loop mode
					{

						break;
					}
					case COMM_SET_CURRENT_BRAKE://, //Motor current brake mode operation
					{
						break;
					}
					case COMM_SET_RPM://, //Motor runs in current loop mode
					{
						break;
					}
					case COMM_SET_POS://, //Motor runs in position loop mode
					{
						break;
					}
					case COMM_SET_HANDBRAKE://, //Motor runs in handbrake current loop mode
					{
						break;
					}
					case COMM_SET_DETECT://, //Motor real-time feedback current position command
					{
						break;
					}
					case COMM_ROTOR_POSITION:// = 22,//Motor feedback current position
					{
						
						int ind = 1;

						yPos = (float)append32(buf, &ind) / 10000.0;
						break;
					}
					case COMM_GET_VALUES_SETUP:// = 50,//Motor single or multiple parameter acquisition instructions
					{
						//for (int i = 0; i < len; i++) {
						//	std::cout << (int)buf[i] << " ";
						//}
						int ind = 1;
						std::cout << std::endl;
						//if (ind < len) {
						append32(buf, &ind);
						//}
						
						for (int i = 0; i < 32; i++) {
							if (getstream[i]) {
								if (sizevar[i] == 1) {
									datate[i] = buf[ind] / scaledata[i];
									ind++;
								}
								else if (sizevar[i] == 2) {
									datate[i] = (float)append16(buf, &ind) / scaledata[i];
								}
								else if (sizevar[i] == 4) {
									datate[i] = (float)append32(buf, &ind)/ scaledata[i];
								}
								//std::cout << datate[i] << " " << i << " " << std::endl;
							}
						
						}
						Outputcurrent = datate[2];
						Inputcurrent = datate[3];
						yPos = datate[16];
						Motorspeed = datate[7];
						Idcurrent = datate[4];
						Iqcurrent = datate[5];
						break;
					}
					case COMM_SET_POS_SPD:// = 91, // Motor runs in position velocity loop mode
					{
						break;
					}
					case COMM_SET_POS_MULTI:// = 92, // Set the motor movement to single-turn mode
					{
						break;
					}
					case COMM_SET_POS_SINGLE:// = 93, // Set the motor motion to multi-turn mode, the range is �100 turns
					{
						break;
					}
					case COMM_SET_POS_UNLIMITED:// = 94, //Save
					{
						break;
					}
					case COMM_SET_POS_ORIGIN:// = 95, //Set the motor origin
					{
						break;
					}
					default:
					{
						break;
					}
				}
				//std::cout << COMM_SET_POS;
			
				upValState = 1;
				break;
			}//End of Case 0
			case 1:
			{

				return false;
				break;
			}//End
		}//End of Switch
	}//End of While
	return false;
}//End of Function



bool Mars::ParseIn() 
{
	 uint8_t len=0;
	uint8_t buff[255];
	bool Err = false, TimeOut = false;
	//
	const clock_t begin_time = clock();
	parseState = 0;
	while (!Err && !TimeOut) {
		//std::cout << "state " <<(int)parseState<< std::endl;
		if ((float(clock() - begin_time) ) > 1000) {
			TimeOut= true;
		}
		switch (parseState) {
			case 0:				//Initialization
			{
				if (this->connected()) {
					parseState = 1;
				}
				;
				break;
			}
			case 1:				//Ready
			{

				len = 0;
				parseState = 2;
				break;
			}
			case 2:
			{						//Wait x02 Header
				if (this->available()) {
					uint8_t in = this->read();
					//	std::cout << "in " << (int)in << " ";
					if (in == 2) {
						parseState = 3;
					}
				}
				else
					return 0;// Sleep(0.1);
			
			break;
			}
			case 3:					//Read Length data
			{
				const clock_t slowread = clock();
				uint8_t countin = 0;
				uint8_t in;
				while ((clock() - slowread) < 1000 && countin < 1)
				{
					if (this->available()) {
						len = this->read();
						countin++;
					}
				}
				//std::cout <<std::endl<<"length  "<<(int)len << " \n";
				if (len > 0) {
					parseState = 4;
				}
				break;
			}
			case 4:
			{						//Read Packet Payload
				const clock_t slowread = clock();
				uint8_t countin = 0;
				while ((clock() - slowread)  < 100 && countin<1)
				{
					if (this->available()>len+2) {
						//buff[countin] = 
						this->read(buff,len+3);
						countin++;
					}
					
				}
				parseState = 5;
				break;
			}
			case 5:					//Read 2 bytes CRC
			{
				const clock_t slowread = clock();
				uint8_t countin = 1;
				//while ((clock() - slowread) < 1000 && countin >=0)
				//{
				//	if (this->available()) {
						crc._8[1] = buff[len];
						crc._8[0] = buff[len+1];

				//		countin--;
				//	}

//				}
				//std::cout << "INPUT : ";
				//for (int i = 0; i < len; i++) {
				//	std::cout <<(unsigned int) buff[i] << " ";
				//}
/*				std::cout << std::endl;
				std::cout <<"crcfromard" << (std::bitset<8>)crc._8[1] << " ";
				std::cout << (std::bitset<8>)crc._8[0] << " ";
				std::cout << (std::bitset<8>)buff[len + 2];
				std::cout << std::endl;
*/				
				crcparsein._16 = crc16(buff, len);
/*				std::cout << "crcparsein" << (std::bitset<8>)(crcparsein._16 >> 8) << " ";
				std::cout << (std::bitset<8>)(uint8_t)(crcparsein._16 & 0xFF) << " ";
				std::cout << std::endl;
				std::cout << "crcparsein" << (std::bitset<8>)crcparsein._8[1] << " ";
				std::cout << (std::bitset<8>)crcparsein._8[0] << " ";
				std::cout << std::endl;*/
				if (crcparsein._16 == crc._16) {
					parseState = 6;
				}
				else {
					parseState = 9;
				}
			break;
			}
			case 6:
			{						//Read x03 Footer
				//uint8_t in = this->read();
				//std::cout << "EENNDD1  " << in << std::endl;
				if (buff[len + 2] = 3) {
					parseState = 7;
					
				}
				else{
					return false;
				}
				break;
			}
			case 7:					//update and confirmation
			{

				this->successco++;

				UpdateVal(buff, len);
				//std::cout << " SUCCESS " << this->available() << std::endl;
				parseState = 1;
				//for (int i = 0; i < len; i++) {
				//	this->write(buff[i]);
				//}
				return true;
				break;
			}
			case 8:					//fault com throw error
			{
				break;
			}
			case 9:					//fault crc throw error
			{
				this->faultco++;
				const clock_t slowread = clock();
				uint8_t countin = 0;
				uint8_t in;
			
				//std::cout << "EENNDD2  " << buff[len + 2] << std::endl;
				std::cout<<" CRC FAULT"<< std::endl;
				parseState = 1;
				return false;
				break;
			}
		}//EndSw
	}//End While
	return false;
}//End Func


bool Mars::ParseOut() 
{
	uint8_t len = 0;
	uint8_t buff[255];
	bool Err = false, TimeOut = false;
	//
	const clock_t begin_time = clock();

	while (!Err && !TimeOut) 
	{
		//std::cout << "parseout state " << (int)yparseState << std::endl;
		if ((float(clock() - begin_time) / CLOCKS_PER_SEC) > 1) {
		TimeOut = true;
		}

		switch (yparseState) {
			case 0:				//Initialization
			{
				if (this->connected()) {
					yparseState = 1;
				}
				//std::cout << "state 0" << std::endl;
				break;
			}
			case 1:				//Ready
			{
				yparseState = 2;
				break;
			}
			case 2:
			{						//Add x02 Header
				len++;
				buff[len - 1] =2;
				yparseState = 3;

				break;
			}
			case 3:					//Add Length data
			{
				switch (_mode)
				{
					case COMM_FW_VERSION: {              //0
						break;
					}
					case COMM_JUMP_TO_BOOTLOADER: {      //1
						break;
					}
					case COMM_ERASE_NEW_APP: {           //2
						break;
					}
					case COMM_WRITE_NEW_APP_DATA: {      //3
						break;
					}
					case COMM_GET_VALUES: {              //4
						len++;
						buff[len - 1] = (uint8_t)1;
						break;
					}
					case COMM_SET_DUTY: {                //5
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_CURRENT: {             //6
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_CURRENT_BRAKE: {       //7
						break;
					}
					case COMM_SET_RPM: {                 //8
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_POS: {                 //9
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_HANDBRAKE: {           //10
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_DETECT: {              //11
						break;
					}
					case COMM_ROTOR_POSITION: {          //22
						break;
					}
					case COMM_GET_VALUES_SETUP: {        //50
						len++;
						buff[len - 1] = 5;
						break;
					}
					case COMM_SET_POS_SPD: {             //91
						len++;
						buff[len - 1] = (uint8_t)13;
						break;
					}
					case COMM_SET_POS_MULTI: {           //92
						break;
					}
					case COMM_SET_POS_SINGLE: {          //93
						break;
					}
					case COMM_SET_POS_UNLIMITED: {       //94
						break;
					}
					case COMM_SET_POS_ORIGIN: {          //95
						len++;
						buff[len - 1] = (uint8_t)2;
						break;
					}
					
				}
				yparseState = 4;
				break;
			}
			case 4:
			{
				switch (_mode)
				{
					case COMM_FW_VERSION: {              //0
						break;
					}
					case COMM_JUMP_TO_BOOTLOADER: {      //1
						break;
					}
					case COMM_ERASE_NEW_APP: {           //2
						break;
					}
					case COMM_WRITE_NEW_APP_DATA: {      //3
						break;
					}
					case COMM_GET_VALUES: {              //4
						len++;
						buff[len - 1] = (uint8_t)4;
						yparseState = 5;
						break;
					}
					case COMM_SET_DUTY: {                //5
						len++;
						buff[len - 1] = (uint8_t)5;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)(xDuty*1000);
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
						break;
					}
					case COMM_SET_CURRENT: {             //6
						len++;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)xCurr;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
						break;
					}
					case COMM_SET_CURRENT_BRAKE: {       //7
						break;
					}
					case COMM_SET_RPM: {                 //8
						len++;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)xSpd ;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
					}
					case COMM_SET_POS: {                 //9
						len++;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)xPos*1000000;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
					}
					case COMM_SET_HANDBRAKE: {           //10
						len++;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)HandBrakeCurr*1000;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
					}
					
					case COMM_SET_DETECT: {              //11
						break;
					}
					case COMM_ROTOR_POSITION: {          //22
						break;
					}
					case COMM_GET_VALUES_SETUP: {        //50
						len++;
						buff[len - 1] = (uint8_t)_mode;
						getstream[0] = 0;//Mosfet temperature
						getstream[1] = 1;//Motor temperature
						getstream[2] = 0;//output current
						getstream[3] = 0;//input current
						getstream[4] = 0;//Id current
						getstream[5] = 0;//Iq current
						getstream[6] = 0;//Duty Cycle
						getstream[7] = 1;//Motor speed
						getstream[8] = 0;//Input voltage
						for (int i = 9; i < 15; i++) {
							getstream[i] = 0;
						}
						getstream[15] = 1;//Motor Error Flag
						getstream[16] = 1;//Motor position
						getstream[17] = 1;//Motor ID
						for (int i = 18; i < 32; i++) {
							getstream[i] = 0;
						}
						for (int i = 0; i < 32; i++) {
							if (getstream[i]) {

								conv32._u32 = conv32._u32 | uint32_t(1UL << i);

							}
							else

								conv32._u32 = ~(~conv32._u32 | uint32_t(1UL << i));
						
						}

						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						//std::cout << "Send DATA "<<conv32._u32 << std::endl;
						break;
					}
					case COMM_SET_POS_SPD: {             //91
					
						len++;
						buff[len - 1] = (uint8_t)_mode;
						conv32._32 = (int32_t)xPos * 1000;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						conv32._32 = (int32_t)xSpd ;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						conv32._32 = (int32_t)xAcc;
						len++;
						buff[len - 1] = conv32._8[3];
						len++;
						buff[len - 1] = conv32._8[2];
						len++;
						buff[len - 1] = conv32._8[1];
						len++;
						buff[len - 1] = conv32._8[0];
						break;
					}
					case COMM_SET_POS_MULTI: {           //92
						break;
					}
					case COMM_SET_POS_SINGLE: {          //93
						break;
					}
					case COMM_SET_POS_UNLIMITED: {       //94
						break;
					}
					case COMM_SET_POS_ORIGIN: {          //95
						len++;
						buff[len - 1] = (uint8_t)95;
						len++;
						buff[len - 1] = (uint8_t)1;
						break;
					}

				}
			
				yparseState = 5;
				//std::cout << (int)buff[0] << " " << (int)buff[1] << " " << (int)buff[2] << " " << (int)buff[3] << " " << (int)buff[4] << " " << (int)buff[5]<< " " << (int)buff[6] << " "<<(int)len<<" " << (int)xPos * conv32._32 << std::endl;
				//return false;
				break;
			}
			case 5: {
				uint8_t payload[255];
				for (int i = 0; i < len-2; i++) {
					payload[i] = buff[i+2];
					//std::cout << "  pl  " << (int)payload[i];
				}
				//std::cout << "crc " <<
				if (len - 2> 0) {
					crc._16 = crc16(payload, (unsigned int) len - 2);
					//std::cout << "crc "  << " " << (int)len-2<< " " << (int)crc._8[0] <<" "<< (int)crc._8[1]<<" "<<(int)payload[0] << " " << (int)payload[1] << " " << (int)payload[2] <<" "<<(int)payload[3] << " " << (int)payload[4] << std::endl;
				}else 	yparseState = 1;
	
				len++;
				buff[len - 1] = crc._8[1];

				len++;
				buff[len - 1] = crc._8[0];
				len++;
				buff[len - 1] = 3;
				yparseState = 6;
				break;
			}

			case 6: {
				//for (int i = 0; i < len; i++) {
					uint8_t* uint8_pointer = &buff[0];
					this->write((char*)uint8_pointer, len);
					//Sleep(1);
				//}
			
				yparseState = 1;
				return 0;
				//break;
			}
		}//EndSw
	}//EndWhile
	return false;
}//EndFunc

int counter = 0;
void Mars::getParam() {
	_mode = 4;
	this->ParseOut();
}
void Mars::setPos() {
	_mode = 9;
	
	this->ParseOut();
}
void Mars::setDuty() {
	_mode = 5;

	this->ParseOut();
}
void Mars::setCurr() {
	_mode = 6;

	this->ParseOut();
}
void Mars::setSpd() {
	_mode = 8;

	this->ParseOut();
}
void Mars::setPosSpd() {
	_mode = 91;

	this->ParseOut();
}
void Mars::setOrigin() {
	_mode = 95;

	this->ParseOut();
}
void Mars::getStream() {
	_mode = 50;
	this->ParseOut();
}
void Mars::setHandBrake() {
	_mode = 10;
	this->ParseOut();
}
bool Mars::Update() {
	//std::cout << "update" << std::endl;
	//this->ParseOut();
	this->ParseIn();
	return false;
}

const unsigned short crc16_tab[] = { 0x0000, 0x1021, 0x2042,
0x3063, 0x4084,
0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c,
0xd1ad,
0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294,
0x72f7,
0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff,
0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a,
0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653,
0x2672,
0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a,
0x9719,
0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886,
0x78a7,
0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af,
0x8948,
0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71,
0x0a50,
0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58,
0xbb3b,
0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60,
0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97,
0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f,
0xefbe,
0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9,
0xb1ca,
0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2,
0x20e3,
0x5004, 0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da,
0xc33d,
0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235,
0x5214,
0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f,
0xd52c,
0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424,
0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3,
0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c,
0xc96d,
0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865,
0x7806,
0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f,
0xfb1e,
0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16,
0x0af1,
0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa,
0xad8b,
0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83,
0x1ce0,
0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9,
0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};
unsigned short Mars::crc16(uint8_t buf[255], unsigned int len) {
	unsigned int i;
	uint16_t cksum = 0;
	//std::cout << "Hoooo"  <<len<< std::endl;
	for (i = 0; i < len	; i++) {
		//std::cout << "HAAAAAAAAAAAAAA" << (int)buf[i] <<" " << (int)cksum << std::endl;
		cksum = crc16_tab[(((cksum >> 8) ^ buf[i]) & 0xFF)] ^ (cksum << 8);
	}
	return cksum;
}

