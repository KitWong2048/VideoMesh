# VideoMesh

## Project Description

VideoMesh is software for P2P live streaming.
It includes 3 major components："VMesh", "VMesh Server", "VMesh Player" and “centralLookupServer”.

### VMesh is the core of the system
It discover neghbor peers, matains and construct P2P overlay, manages vedio segment caching at peers.

### VMesh Server is a video publisher server.
Its functions include
Movie management (publish/delete)
Starting & Restarting Central Lookup Server
Storing and serving all advertisement clips upon request

### VMesh Player is a P2P live streaming client.
Its functions include
Movie list management and playback control

### centralLookupServer is a P2P tracker server
which acts as the Bitorrent trackers.

## Requirement
Boost library 1.46.1
MFC

## Build
Open the the solution VMesh.sln with Visual C++ 2009 (Visual studio 2008) and build projects according to follwing  order
libNode, chord, libasf, VMesh, VMeshController, CentralizedLookupServer, VMeshAPI, VMeshPlayer, Vmeshsever

##How to
1) Run the CentralizedLookupServer
2) Run the VMesh Server and publish some videos (support only .ASF format)
3) Run VMesh Players and subscribe the videos list of VMesh Server.

