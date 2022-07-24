FROM ubuntu:20.04 as  baseenv

RUN apt-get update \
    && apt-get install -y vim automake build-essential git wget libssl-dev libtool python3 python3-pip >/dev/null \
    && apt-get clean

#install cmake
RUN mkdir -p /thirdparty/cmake-src \
    && cd /thirdparty/cmake-src \
    && wget https://github.com/Kitware/CMake/releases/download/v3.22.2/cmake-3.22.2.tar.gz >/dev/null \ 
    && tar -zxvf cmake-3.22.2.tar.gz >/dev/null \
    && cd cmake-3.22.2 \
    && ./bootstrap >/dev/null \
    && make >/dev/null \
    && make install 

FROM baseenv as builder

COPY ./ /xsce/
RUN cd /xsce && ls && ./build.py libote xsce clean && ./build.py install=/opt/xsce_install

FROM baseenv as machine

COPY --from=builder /opt/xsce_install/ /usr/local
