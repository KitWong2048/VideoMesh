# VideoMesh

## Project Description

VideoMesh is software for P2P live streaming.
It includes 3 major components："VMesh Server", "VMesh Player" and “centralLookupServer”.
### VMesh Server is a video publisher server.

Its functions include
Movie management (publish/delete)
Start & Restart Central Lookup Server
Like other peers, except:
Store all segments
Has a “root” ID
Store and serve all advertisement clips upon request

### VMesh Player is a P2P live streaming client.

Its functions include
Use web page to manage movie list
Multiple movie management
Removed redundancy code and features
Simplified playback control code in old version and added various new features

### centralLookupServer is a P2P tracker server, which acts as the Bitorrent trackers.
