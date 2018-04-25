#ifndef PACKET_H
#define PACKET_H

#include "../Common/VMeshCommon.h"
#include "../Util/Logger.h"

/**	@file Packet.h
 *	This file contains the definition of class Packet
 *  Packet is the unit of network transmission
 *	A Segment contains a number of Packets
 *  
 *	@author Kelvin Chan, Philip Yang
 */

_VMESH_NAMESPACE_HEADER

/**	@class Packet
 *	This class provides the packet of a movie, the basic components of a movie
 */
#define MOD_ADLER 65521
class Packet{
public:
	/**	@fn Packet(unsigned int packetID, char* data, int length)
	 *	Constructor providing the Packet data and the Packet ID;
	 *	@param packetID the ID of the Packet
	 *	@param data the Packet Data, shallow copy
	 *	@param length length of the array in data
	 */
	Packet(unsigned int packetID, char* data, int length);

	Packet(unsigned int packetID, char* data, int length, unsigned int checksum);

	/**	@fn Packet(const Packet& p)
	 *	Copy constructor;
	 *	@param p Packet to be copied
	 */
	Packet(const Packet& p);

	/**	@fn Packet(const char* serialData, int len)
	 *	Constructor using serialized char array;
	 *	@param serialData serialized char array
	 *	@param len length of the array
	 */
	Packet(const char* serialData, int len, bool calculateChecksum = true);
	
	/**	@fn ~Packet()
	 *	Destructor;
	 */
	~Packet();
	
	/**	@fn serialize(char*& serialData, int& len)
	 *	Creates a serialized char array for this object;
	 *	@param serialData serialized char array (packetID|data)
	 *	@param len length of the array
	 */
	void serialize(char*& serialData, int& len);
	
	/**	@fn operator=(const Packet& p)
	 *	Deep copy of the Packet;
	 */
	Packet& operator=(const Packet& p);
		
	/**	@fn getData() const
	 *	Return the packet data;
	 *	@return the packet data (pointer is returned directly without deep copy)
	 */
	char* getData() const;
	
	/**	@fn getLength() const
	 *	Return the length of the packet data;
	 *	@return the length of the packet data
	 */
	int getLength() const;

	/**	@fn getPacketID() const
	 *	Return the packet ID;
	 *	@return the packet ID
	 */
	unsigned int getPacketID() const;

	/**	@fn char getCheckSum() const
	*	Return the checksum value for this packet;
	*	@return the checksum value
	*/
	unsigned int getCheckSum() const;

	static bool validateSerializedData(char* data, int len);

	bool validatePacket();

private:
	unsigned int packetID;
	char* data;
	int length;
	unsigned int checksum;

	static unsigned int calculateAdler32Checksum(char* data, int len);
};
_VMESH_NAMESPACE_FOOTER
#endif
