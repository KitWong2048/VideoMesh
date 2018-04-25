#include "../Common/Packet.h"

_VMESH_NAMESPACE_HEADER

Packet::Packet(unsigned int packetID, char* data, int length):packetID(packetID), data(data), length(length){
	checksum = calculateAdler32Checksum(data, length);
	return;
}

Packet::Packet(unsigned int packetID, char* data, int length, unsigned int checksum):packetID(packetID), data(data), length(length), checksum(checksum){
}


Packet::Packet(const Packet& p):packetID(p.packetID), length(p.length){
	data = new char[length];
	//for (int i=0; i<length; i++)
	//	data[i] = p.data[i];
	if ( data && p.data )
		memcpy(data, p.data, length);

	checksum = calculateAdler32Checksum(data, length);

	return;
}

Packet::Packet(const char* serialData, int len, bool calculateChecksum){
	int sizeOfInt = sizeof(int);
	length = len - sizeOfInt - sizeOfInt;
	
	int i;
	packetID = 0;
	
	//for (i=0; i<sizeof(int); i++){
//		packetID |= ((int(serialData[i]) & 0xff) << 8*i);	
//	}

	for ( i = sizeOfInt - 1; i >= 0; i-- )
	{
		packetID <<= 8;
		packetID |= ((int(serialData[i]) & 0xff));
	}
	data = new char[length];
	
	//for (i=0; i<length; i++){
	//	data[i] = serialData[i+sizeof(int)];
	//}
	
	memcpy(data, serialData + sizeof(int), length);

	if ( calculateChecksum )
		checksum = calculateAdler32Checksum(data, length);
	else
	{
		checksum = 0;
		const char* dataPtr = &serialData[len - 1];
		for ( i = 0; i < 4; i++ )
		{
			checksum <<= 8;
			checksum |= ((int)(*dataPtr) & 0xff);
			dataPtr--;
		}
	}

	return;	
}

Packet::~Packet(){
	if (data){
		delete[] data;
		data = 0;
	}
	return;
}

void Packet::serialize(char*& serialData, int& len){
	int sizeOfInt = sizeof(int);
	len = sizeOfInt + length + sizeOfInt;	// packet id + packet data + checksum
	
	serialData = new char[len];
	
	int i;
	unsigned int pid = packetID;
	// serial[0 ... sizeof(int)-1] = packetID;
//	for (i=0; i<sizeOfInt; i++){
//		serialData[i] = (char)((packetID >> 8*i) & 0xff);
//	}

	for ( i = 0; i < sizeOfInt; i++)
	{
		serialData[i] = (char)(pid & 0xff);
		pid >>= 8;
	}

	memcpy(serialData+sizeOfInt, data, length);

	unsigned int cksum = checksum;
	
	for ( i = sizeOfInt + length; i < len; i++)
	{
		serialData[i] = (char)(cksum & 0xff);
		cksum >>= 8;
	}

	return;	
}

Packet& Packet::operator=(const Packet& p){
	if (this != &p){
		packetID = p.packetID;
		length = p.length;
		
		if (data){
			delete[] data;
			data = 0;
		}
		data = new char[length];
		
		//for (int i=0; i<length; i++){
		//		data[i] = p.data[i];
		//	}

		memcpy( data, p.data, length );

		checksum = p.checksum;
	}
	
	return *this;
}

char* Packet::getData() const{
    return data;
}

int Packet::getLength() const{
	return length;
}

unsigned int Packet::getPacketID() const{
	return packetID;
}


unsigned int Packet::getCheckSum() const{
	return checksum;
}

bool Packet::validateSerializedData(char* data, int len)
{
	int sizeOfInt = sizeof(int);
	int dataLength = len - sizeOfInt - sizeOfInt;

	int i;

	unsigned int checksumInData = 0;
	unsigned int checksumCalculated;

	const char* dataPtr = &data[len - 1];

	for ( i = 0; i < 4; i++ )
	{
		checksumInData <<= 8;
		checksumInData |= ((int)(*dataPtr) & 0xff);
		dataPtr--;
	}

	checksumCalculated = calculateAdler32Checksum(data + sizeOfInt, dataLength);

	return ( checksumInData == checksumCalculated );
}

bool Packet::validatePacket()
{
	unsigned int checksumCalculated;

	checksumCalculated = calculateAdler32Checksum(data , length);
	
	return ( checksum == checksumCalculated );
}

unsigned int Packet::calculateAdler32Checksum(char* data, int len)
{
	if ( data == NULL || len < 1 )
		return 0;

	unsigned int a = 1, b = 0;

	while (len) 
	{
		int tlen = len > 5550 ? 5550 : len;
		len -= tlen;
		do 
		{
			a += *data++;
			b += a;
		} while (--tlen);

		a %= MOD_ADLER;
		b %= MOD_ADLER;
	}

	return (b << 16) | a;
}

_VMESH_NAMESPACE_FOOTER
