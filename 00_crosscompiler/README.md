AArch64 Cross Compiler
======================

튜토리얼을 시작하기 전에 몇 가지 도구가 필요합니다. AArch64 아키텍처 용으로 컴파일 되는 컴파일러와 함께 제공되는 유틸리티입니다.

준비 사항
------------

우리는 'Ubuntu 20.04 LTS'에서 작업할 것입니다. GCC, G++, Make가 필요합니다.

```sh
sudo apt install gcc g++ make
```

다운로드와 소스 압축 풀기
---------------------------

우선 Binutils 및 GCC 소스를 다운로드 합니다. 이 예제에서는 필자가 최신 버전을 사용했습니다.

```sh
wget https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-9.3.0/gcc-9.3.0.tar.xz
wget https://ftp.gnu.org/gnu/mpfr/mpfr-4.0.2.tar.xz
wget https://ftp.gnu.org/gnu/gmp/gmp-6.2.0.tar.xz
wget https://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz
wget https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.18.tar.bz2
wget https://gcc.gnu.org/pub/gcc/infrastructure/cloog-0.18.1.tar.gz
```

그런 다음 아래의 명령으로 tarball의 압축을 풉니다:

```sh
for i in *.tar.gz; do tar -xzf $i; done
for i in *.tar.bz2; do tar -xjf $i; done
for i in *.tar.xz; do tar -xJf $i; done
```

파일들을 제거합니다. 우리는 더 이상 이 파일들이 필요 없습니다:

```sh
rm -f *.tar.*
```

컴파일을 시작하기 전에 심볼릭 링크가 필요하므로 링크를 만들어 보겠습니다:

```sh
cd binutils-*
ln -s ../isl-* isl
cd ..
cd gcc-*
ln -s ../isl-* isl
ln -s ../mpfr-* mpfr
ln -s ../gmp-* gmp
ln -s ../mpc-* mpc
ln -s ../cloog-* cloog
cd ..
```

소스 컴파일하기
---------------------

이제 두 개의 패키지를 만들어야합니다. 하나는 *Binutils*이며 링커, 어셈블러 및 기타 유용한 명령을 포함합니다.

```sh
mkdir aarch64-binutils
cd aarch64-binutils
../binutils-*/configure --prefix=/usr/local/cross-compiler --target=aarch64-elf \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld \
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4
make install
cd ..
```

첫 번째 인수는 configure 스크립트가 `/usr/local/cross-compiler`에 설치하도록 지시합니다. 두 번째는 툴을 컴파일 할 대상 아키텍처를 지정합니다. 다른 인수들은 특정 옵션을 켜고 끄는 옵션입니다. 임베디드 시스템 컴파일러에 맞게 적절히 조정되어 있다는 것을 아는 것으로 충분합니다.

그리고 두 번째 패키지는 *GCC 컴파일러*가 필요합니다.

```sh
mkdir aarch64-gcc
cd aarch64-gcc
../gcc-*/configure --prefix=/usr/local/cross-compiler --target=aarch64-elf --enable-languages=c \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld \
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4 all-gcc
make install-gcc
cd ..
```

여기에서 이전과 동일한 디렉토리 및 아키텍처를 지정합니다. GCC는 우리가 필요로 하지 않는 많은 언어를 지원하므로 C 컴파일러만 컴파일하도록 지시합니다. 컴파일 시간이 크게 줄어듭니다. 나머지 인수는 Binutils와 동일합니다.

이제 `/usr/local/cross-compiler` 디렉토리에서 `bin` 폴더를 확인합니다. 거기에는 많은 실행 파일들이 있습니다. 이 디렉토리를 `PATH` 환경 변수에 추가합니다.

```sh
$ ls /usr/local/cross-compiler/bin
aarch64-elf-addr2line  aarch64-elf-elfedit    aarch64-elf-gcc-ranlib  aarch64-elf-ld       aarch64-elf-ranlib
aarch64-elf-ar         aarch64-elf-gcc        aarch64-elf-gcov        aarch64-elf-ld.bfd   aarch64-elf-readelf
aarch64-elf-as         aarch64-elf-gcc-7.2.0  aarch64-elf-gcov-dump   aarch64-elf-nm       aarch64-elf-size
aarch64-elf-c++filt    aarch64-elf-gcc-ar     aarch64-elf-gcov-tool   aarch64-elf-objcopy  aarch64-elf-strings
aarch64-elf-cpp        aarch64-elf-gcc-nm     aarch64-elf-gprof       aarch64-elf-objdump  aarch64-elf-strip
```

우리에게 관심 있는 실행 파일은 아래와 같습니다:
 - `aarch64-elf-as` - 어셈블러
 - `aarch64-elf-gcc` - C 컴파일러
 - `aarch64-elf-ld` - 링커
 - `aarch64-elf-objcopy` - ELF 실행 파일을 IMG 형식으로 변환할때 사
 - `aarch64-elf-objdump` - 실행 파일을 분해하는 유틸리티 (디버깅 용)
 - `aarch64-elf-readelf` - 실행 파일에서 섹션 및 세그먼트를 덤프 하는 유용한 유틸리티 (디버깅 용)

위의 6가지 실행 파일이 모두 있고, 오류 메시지 없이 실행할 수 있다면, 이제 튜토리얼을 진행할 수 있습니다!
