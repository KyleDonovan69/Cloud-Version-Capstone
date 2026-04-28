# Use Ubuntu as base image
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsfml-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY include/ /app/include/
COPY src/Network/ /app/src/Network/
COPY src/ServerMain.cpp /app/src/

# Create build directory
RUN mkdir -p build

# Create a simple CMakeLists.txt for the server
RUN echo 'cmake_minimum_required(VERSION 3.10)\n\
project(LandsOfPixelServer)\n\
\n\
set(CMAKE_CXX_STANDARD 20)\n\
set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\
\n\
find_package(SFML 2.5 COMPONENTS system network REQUIRED)\n\
\n\
include_directories(include)\n\
\n\
add_executable(server\n\
    src/ServerMain.cpp\n\
    src/Network/NetworkProtocol.cpp\n\
    src/Network/GameServer.cpp\n\
)\n\
\n\
target_link_libraries(server sfml-system sfml-network)\n' > /app/CMakeLists.txt

# Build the server
WORKDIR /app/build
RUN cmake .. && cmake --build . --config Release

# Expose the game server port
EXPOSE 53001/udp

# Run the server
CMD ["./server", "53001"]
