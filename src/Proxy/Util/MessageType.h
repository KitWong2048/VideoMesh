#ifndef _MESSAGE_TYPE_H
#define _MESSAGE_TYPE_H

/**	@file MessageType.h
 *	This file contains the definition of enum MessageType
 *	@author Kelvin Chan
 */

_VMESH_NAMESPACE_HEADER

/**	@enum MessageType
 *	An enum to classifies different types of message
 */
 enum MessageType{
	SUPPLY_LOCALNODELIST = 0,	/**< supply with VMeshLocalNodeList */

	PING = 10,					/**< ping */
	RPY_PING,					/**< ping acknowledgement */

	REQ_PACKET = 20,			/**< request for packet */
	RPY_PACKET_OK,				/**< request reply ok */
	RPY_PACKET_FAIL,			/**< request reply fails */
	
	REQ_LIST = 30,				/**< request for VMeshRemoteNodeList of a segment */
	RPY_LIST_OK,				/**< request reply ok */
	RPY_LIST_FAIL,				/**< request reply fails */

	REQ_MEDIAINFO = 40,			/**< request for supplying MediaInfo */
	RPY_MEDIAINFO_OK,			/**< request reply ok */
	RPY_MEDIAINFO_FAIL,			/**< request reply fails */

	REQ_LOCALNODELIST = 50,		/**< request for the VMeshLocalNodeList */
	RPY_LOCALNODELIST_OK,		/**< request reply ok */

	REQ_PARENTLIST = 60,		/**< request for a VMeshRemoteNodeList of parents of a segment */
	RPY_PARENTLIST_OK			/**< request reply ok */
};

_VMESH_NAMESPACE_FOOTER
#endif

