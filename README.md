# order-matching-sample

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

3. A dockerfile setting up the environment and building the binary is included
in this package. Use `docker build .` and then run the container through `docker run -ti <image_id>`.
The executable (`krakenite`) is located in the `/root/build` directory


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
