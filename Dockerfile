FROM ubuntu:23.10
RUN apt-get update
RUN apt-get install -y build-essential cmake libboost-dev python3
WORKDIR /root
COPY . .
RUN rm -rf build && mkdir -p build && cd build && cmake .. && make
RUN pwd
CMD /bin/bash