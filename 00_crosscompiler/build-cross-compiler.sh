#!/bin/bash
set +h
set -o nounset
set -o errexit

TARGET=aarch64-elf
PREFIX=$HOME/aarch64-elf-linux-x86_64
PARALLEL_JOBS=$(cat /proc/cpuinfo | grep cores | wc -l)
export PATH=$PREFIX/bin:$PATH

wget -c https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
wget -c https://ftp.gnu.org/gnu/gmp/gmp-6.2.0.tar.xz
wget -c https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.18.tar.bz2
tar xvJf binutils-2.34.tar.xz
tar xvJf gmp-6.2.0.tar.xz
tar xvjf isl-0.18.tar.bz2
mv -v gmp-6.2.0 binutils-2.34/gmp
mv -v isl-0.18 binutils-2.34/isl
( cd binutils-2.34 && \
./configure \
--prefix=$PREFIX \
--target=$TARGET \
--enable-shared \
--enable-threads=posix \
--enable-libmpx \
--with-system-zlib \
--with-isl \
--enable-__cxa_atexit \
--disable-libunwind-exceptions \
--enable-clocale=gnu \
--disable-libstdcxx-pch \
--disable-libssp \
--enable-plugin \
--disable-linker-build-id \
--enable-lto \
--enable-install-libiberty \
--with-linker-hash-style=gnu \
--with-gnu-ld \
--enable-gnu-indirect-function \
--disable-multilib \
--disable-werror \
--enable-checking=release \
--enable-default-pie \
--enable-default-ssp \
--enable-gnu-unique-object )
make -j$PARALLEL_JOBS -C binutils-2.34
make -j$PARALLEL_JOBS install -C binutils-2.34
rm -rf binutils-2.34

wget -c https://ftp.gnu.org/gnu/gcc/gcc-9.3.0/gcc-9.3.0.tar.xz
wget -c https://ftp.gnu.org/gnu/mpfr/mpfr-4.0.2.tar.xz
wget -c https://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz
wget -c https://gcc.gnu.org/pub/gcc/infrastructure/cloog-0.18.1.tar.gz
tar xvJf gcc-9.3.0.tar.xz
tar xvJf mpfr-4.0.2.tar.xz
tar xvJf gmp-6.2.0.tar.xz
tar xvzf mpc-1.1.0.tar.gz
tar xvjf isl-0.18.tar.bz2
tar xvzf cloog-0.18.1.tar.gz
mv -v mpfr-4.0.2 gcc-9.3.0/mpfr
mv -v gmp-6.2.0 gcc-9.3.0/gmp
mv -v mpc-1.1.0 gcc-9.3.0/mpc
mv -v isl-0.18 gcc-9.3.0/isl
mv -v cloog-0.18.1 gcc-9.3.0/cloog
( cd gcc-9.3.0 && \
./configure \
--prefix=$PREFIX \
--target=$TARGET \
--enable-languages=c \
--enable-shared \
--enable-threads=posix \
--enable-libmpx \
--with-system-zlib \
--with-isl \
--enable-__cxa_atexit \
--disable-libunwind-exceptions \
--enable-clocale=gnu \
--disable-libstdcxx-pch \
--disable-libssp \
--enable-plugin \
--disable-linker-build-id \
--enable-lto \
--enable-install-libiberty \
--with-linker-hash-style=gnu \
--with-gnu-ld \
--enable-gnu-indirect-function \
--disable-multilib \
--disable-werror \
--enable-checking=release \
--enable-default-pie \
--enable-default-ssp \
--enable-gnu-unique-object )
make -j$PARALLEL_JOBS all-gcc -C gcc-9.3.0
make -j$PARALLEL_JOBS install-gcc -C gcc-9.3.0
rm -rf gcc-9.3.0
