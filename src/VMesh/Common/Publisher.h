#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>

#include "../Common/VMeshCommon.h"
#include "../Storage/BootstrapStorage.h"
#include "../Common/MediaInfo.h"
#include "../Util/Logger.h"
//#include "../../Packetizer/RTPPacketizer.h"
extern "C" {
#include "../../libasf/libasf.h"
}

/**	@file Publisher.h
 *	This file contains the definition of class Publisher;
 *	This file must also include a desired packetizer;
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@class Publisher
 *	This class provides static methods for publishing and packetizing a movie
 */
class Publisher {
public:
	/**	@fn static bool packetizeMovie(std::string filename)
	 *	Packetize the movie with filename;
	 *	@param filename the filename of the movie file to be packetized
	 *	@return true if packetize success, false otherwise
	 */
	static bool packetizeMovie(std::string filename);

	/**	@fn static bool publish(BootstrapStorage*& b, MediaInfo*& m, 
	 *  std::string filename, int mediaType, int interval, int numIntervalsinSegment)
	 *	Publish the movie with packetized file;
	 *	@param b the pointer to the BootstrapStorage to be constructed
	 *	@param m the pointer to the MediaInfo to be constructed
	 *	@param filename the filename of the packetized file to be published
	 *	@param mediaType type of media (no use)
 	 *	@param interval 1000 (no use)
	 *	@param numIntervalsinSegment number of seconds in a segment
	 *	@return true if publish success, false otherwise
	 */
	static bool publish(BootstrapStorage*& b, MediaInfo*& m, std::string filename, int mediaType, int interval, int numIntervalsinSegment);	
};
_VMESH_NAMESPACE_FOOTER
#endif

