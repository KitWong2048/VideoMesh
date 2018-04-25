INCLUDES += -I ../..
CPPFLAGS += -g $(INCLUDES)
LDFLAGS  += -lpthread
LDLIBS	+= -lpthread
CC	= g++
CXX = $(CC)
OUTPUT_DIR = ../build

OBJS = DHTSocket/SessionReference.o DHTSocket/Socket.o DHTSocket/PoolSocket.o DHTSocket/SocketSelector.o DHTSocket/SocketClient.o DHTSocket/VirtualPoolSocket.o DHTSocket/ServerHandle.o DHTSocket/PoolSocketMgr.o DHTSocket/SocketServer.o DHTSocket/MessageBoxSet.o Util/Mutex.o Util/SeqGenerator.o Util/ConditionVariable.o Util/InetAddress.o Util/Thread.o DHT/Logger.o DHT/Node.o DHT/LocalNodeCollection.o DHT/DHTMessageFactory.o DHT/LocalNodeHandle.o DHT/DHTNetworkID.o DHT/FingerTable.o DHT/DHTRequest.o DHT/AbstractNodeValue.o DHT/AppValue.o DHT/Stabilizer.o DHT/DHTService.o DHT/Chord.o DHT/NodeBucket.o App/ChordApp.o App/ChordAttr.o App/AsyncHandle.o App/FarTimeShift.o
TEST_OBJS = Test/FarTimeShfitTest.o main.o
#TEST_OBJS = Test/DistributedTest.o  Test/FarTimeShfitTest.o  Test/PoolTest.o  Test/Test.o
CHORD_LIB = chord
CHORD_TEST = ChordEXE
LDFLAGS  += -l$(CHORD_LIB)


all: $(CHORD_LIB)

test: $(CHORD_TEST)	
	mv $? $(OUTPUT_DIR)/

$(CHORD_LIB): $(OBJS)
	$(AR) rcs lib$(CHORD_LIB).a $? 
	mv lib$@.a $(OUTPUT_DIR)
	
$(CHORD_TEST): $(TEST_OBJS) $(CHORD_LIB)
	$(CXX) -g $(TEST_OBJS) -o $@ -L $(OUTPUT_DIR) $(LDFLAGS)

clean:
	-rm $(OBJS) $(TEST_OBJS)

distclean:
	-rm -f $(OBJS) $(TEST_OBJS)
	-rm -f $(OUTPUT_DIR)/lib$(CHORD_LIB).a $(OUTPUT_DIR)/CHORD_TEST
