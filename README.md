# krakenite

This is Dale Russell's submission to the take-home programming test

### How to build and run?

1. Create and build project files for your platform, e.g.
   ```
   mkdir build
   cd build
   cmake ..
   ```
   Note: You can build in build type to ```cmake```, i.e. ```-DCMAKE_BUILD_TYPE="Release"```.

   After creating the project file, run ```make``` which will create the executable.

2. Run the built executable via ```./krakenite```
   By default it will run the unit tests, and the execution mode can be set from command line:

    - ```./krakenite --test```: Run unit tests
	- ```./krakenite --server```: Run the UDP stream server
	- ```./krakenite --client```: Run the UDP stream client

### What's in the package?

Following files are in the package (in alphabetical order):

C++ Header Files
```
./include
	./client.h - UDP stream client 
	./filestream.h - File streamer for .csv files
	./history.h - Order history tracker
	./logpub.h - Log publisher
	./netio.h - Home made protocol buffers
	./order.h - Order type, sorting, finding
	./orderbook.h - Orderbook, and OrderBookManager 
	./proto.h - Protocol parser
	./server.h - UDP stream server
	./test.h - Unit tests
	./tob.h - Top of book tracker
	./types.h - Custom types abstraction
	./user.h - User type, sorting, finding
	./util.h - Utility functions (Misc stuff)
```

C++ Source Files
```
./src
	./client.cpp - UDP stream client
	./main.cpp - Program entry point/dispatcher
	./server.cpp - UDP stream server
	./test.cpp - Unit tests
```

Misc Data Files
```
./etc
	./input.csv - The exercise input file (no functional change)
	./output.csv - The excercise outputs (with supplement of the even numbered scenario outputs)
```

### Functionality in this implementation

- *UDP stream client/server*: I was not sure if my implementation counts as codec, 
  for the bonus - the server loads the input file (csv), converts it to protocol buffers (see ```netio.h```),	
  and then sends it over the wire as binary format. I didn't use gprotobufs.

- *Orderbook processing*: All bullet points in the challenge are implemented

- *Publish in separate thread*: Implemented, please see ```logpub.h```.
  This functionality also helped with the unit tests, so it could capture	
  the output from the Orderbook and cross validate with the input

- *Bonus-Create unit tests*: Implemented, please see ```test.cpp```
  Created in memory representation of the inputs and a full complement of the 
  expected outputs for a total of 16 scenarios and 177 steps

- *Bonus-Create e2e performance tests*: _NOT_ implemented
  Wasn't able to think about how to effectively measure this, 
  and the time ran out!

### If I had more time, I'd done...
- Revisit my orderbook implementation. I'm not happy with the top-of-book handling.
- Enhance stability (and performance) of the custom UDP protocols. 
  For example, buffer batching (rather than sending them individually), and minimize sendto().
  Package numbering, retry facilities and other strategies for *Reliable UDP*
