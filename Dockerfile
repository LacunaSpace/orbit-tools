FROM ubuntu:focal
RUN apt update && apt install -y gcc make gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

RUN mkdir -p /buildroot/src
COPY Makefile /buildroot
COPY generate-cities.pl /buildroot
COPY generate-countries.pl /buildroot
COPY src/* /buildroot/src/

ARG VERSION
ARG ARCHITECTURE

WORKDIR /buildroot
RUN make clean
RUN if [ "$ARCHITECTURE" = "arm64" ] ; then make CC="aarch64-linux-gnu-gcc -static" ; else make CC=gcc ; fi

RUN mkdir -p package/usr/bin
RUN cp -r bin package/usr
RUN mkdir -p package/DEBIAN

RUN echo "Package: orbit-tools"                                 >> package/DEBIAN/control
RUN echo "Version: ${VERSION}"                                  >> package/DEBIAN/control
RUN echo "Maintainer: Jeroen Koops <jeroen@lacuna-space.com>"   >> package/DEBIAN/control
RUN echo "Depends:"                                             >> package/DEBIAN/control
RUN echo "Architecture: ${ARCHITECTURE}"                        >> package/DEBIAN/control
RUN echo "Homepage: https://github.com/LacunaSpace/orbit-tools" >> package/DEBIAN/control
RUN echo "Description: Tools for working with orbits"           >> package/DEBIAN/control

RUN mkdir /output
VOLUME /output

ENTRYPOINT dpkg --build package /output