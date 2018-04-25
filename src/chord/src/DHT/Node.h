/*
Node.h

Copyright (c) 2007 Hong Kong University of Science and Technology ("HKUST")
This source code is an intellectual property owned by HKUST and funded by
Innovation and Technology Fund (Ref No. GHP/045/05)

Permission is hereby granted, to any person and party obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the
Software with the rights to use, copy, modify and merge, subject to the
following conditions:

1. Commercial use of this source code or any derivatives works from this source
 code in any form is not allowed

2. Redistribution of this source code or any derivatives works from this source
 code in any form is not allowed

3. Any of these conditions can be waived if you get permission from the
copyright holder

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**	@file Node.h
 *	This file contains the class for encapsulating node data used in the DHT.
 *	This class serves as an interface for communicating host information with application above the DHT.
 */
#ifndef _H_NODE_
#define _H_NODE_

#include "../Util/Common.h"
#include "../Util/InetAddress.h"
#include "../DHT/AbstractNodeValue.h"
#include "../DHT/DHTNetworkID.h"

using namespace std;

namespace DHT{
	/**	@class Node
	 *	This class encapsulates node in Chord.
	 */
	class Node : public DHTNetworkID{
	private:
		/**	@var address
		 * The internet address of the node at Chord level.
		 */
		Util::InetAddress address;

		/**	@var appValue
		 * The application value associated with the node.
		 */
		AbstractNodeValue* appValue;

	public:
		Node();

		Node(const DHTNetworkID& _nodeID);

		Node(const DHTNetworkID& _nodeID, const char* _IP, int _port);

		Node(const Node& _node);

		~Node();

		DHTNetworkID getNodeID() const;

		int setNodeID(const DHTNetworkID& _nodeID);

		string getNodeIP() const;

		int setNodeIP(const char* _IP);

		int getNodePort() const;

		int setNodePort(int _port);

		/** @fn int toBin(char*& _output) const
		 * Output the node to a binary representation.
		 * @param _output: output parameter, an array of char which allocated dynamically
		 * @return the length of the array
		 */
		int toBin(char*& _output) const;

		/** @fn int fromBin(const char* _input)
		 * Get the value of the node from binary message.
		 * @param _input: the message to be parsed
		 * @return 0 if the parising is successful
		 */
		int fromBin(const char* _input);

		/** @fn int toBinApp(char*& _output) const
		 * Output the application value to a binary representation.
		 * @param _output: output parameter, an array of char which allocated dynamically
		 * @return the length of the array
		 */
		int toBinApp(char*& _output) const;

		/** @fn int fromBinApp(const char* _input)
		 * Get the application value from binary message.
		 * @param _input: the message to be parsed
		 * @return 0 if the parising is successful
		 */
		int fromBinApp(const char* _input);

		/** @fn int toBinFull(char*& _output) const
		 * Output the node and the application value to a binary representation.
		 * @param _output: output parameter, an array of char which allocated dynamically
		 * @return the length of the array
		 */
		int toBinFull(char*& _output) const;

		/** @fn int fromBinFull(const char* _input)
		 * Get the node and the application value from binary message.
		 * @param _input: the message to be parsed
		 * @return 0 if the parising is successful
		 */
		int fromBinFull(const char* _input);

		/** @fn bool isIdentical(const Node& _node)
		 *	Check if the given node is equal to this, comparing ID and the address.
		 * @param _node: the node to be compare with.
		 * @return true if the ID and address are identical to the corresponding fields of the provided node
		 */
		bool isIdentical(const Node& _node);

		/** @fn bool operator==(const Node& _node) const
		 *	Check if the given node is equal to this, comparing ID only.
		 * @param _node: the node to be compare with.
		 * @return true if the ID is identical to the corresponding field of the provided node
		 */
		bool operator==(const Node& _node) const;

		/** @fn bool operator<(const Node& _node) const
		 *	Check if the given NodeID is less than this.
		 * @param _node: the NodeID to be compare with.
		 * @return true if the ID of the local node is smaller than the input
		 */
		bool operator<(const Node& _node) const;

		/** @fn Node& operator=(const Node& _node)
		 *	Assignment operator overloading.
		 * Make a deep copy of the input object.
		 * @param _node: the Node to be copied.
		 * @return a reference to the caller
		 */
		Node& operator=(const Node& _node);

		/** @fn int setNodeFrmStr(const string& _msg)
		 * Get the value of the node from a message in the format of ID@IP:Port.
		 * @param _msg: the message to be parsed
		 * @return 0 if the parising is successful
		 */
		int setNodeFrmStr(const string& _msg);

		/** @fn AbstractNodeValue* getAppValue() const
		 * @return the application value associated with the node
		 */
		AbstractNodeValue* getAppValue() const;

		/** @fn friend ostream& operator<<(ostream&, const Node& _node)
		 *	output stream operator for this class
		 * @param _out: the stream to output.
		 * @param _node: the node to be output.
		 */
		friend ostream& operator<<(ostream& _out, const Node& _node);
	};
}

#endif
