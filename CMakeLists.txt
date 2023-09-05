cmake_minimum_required(VERSION 3.5)
project(krakenite)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files to the project
set(SOURCES
    src/client.cpp
    src/main.cpp
    src/server.cpp
    src/test.cpp
# src/file.cpp
)

# Add header files to the project
set(HEADERS
    include/client.h
    include/filestream.h
    include/history.h
    include/logpub.h
    include/netio.h
    include/order.h
    include/orderbook.h
    include/proto.h
    include/server.h
    include/test.h
    include/tob.h
    include/types.h
    include/user.h
    include/util.h
)

# Set include directories
include_directories(include)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

