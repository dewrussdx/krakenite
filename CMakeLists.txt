cmake_minimum_required(VERSION 3.5)
project(krakenite)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files to the project
set(SOURCES
    client.cpp
    main.cpp
    server.cpp
    test.cpp
# src/file.cpp
)

# Add header files to the project
set(HEADERS
    client.h
    filestream.h
    history.h
    logpub.h
    netio.h
    order.h
    orderbook.h
    proto.h
    server.h
    test.h
    tob.h
    types.h
    user.h
    util.h
)

# Set include directories
# include_directories(include)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

