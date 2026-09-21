#!/bin/bash
set -e

export PREFIX="$HOME/opt/cross"
export TARGET="x86_64-elf"
export PATH="$PREFIX/bin:$PATH"

# Versions (You can update these to the latest stable releases if desired)
BINUTILS_VERSION="2.43"
GCC_VERSION="14.2.0"

CORES=$(nproc)

echo "==========================================================="
echo " Building x86_64-elf Cross-Compiler"
echo " Target: $TARGET"
echo " Prefix: $PREFIX"
echo " Cores:  $CORES"
echo "==========================================================="

mkdir -p "$HOME/src"
cd "$HOME/src"

echo "[1/4] Downloading sources..."
if [ ! -f "binutils-$BINUTILS_VERSION.tar.gz" ]; then
    wget "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"
fi

if [ ! -f "gcc-$GCC_VERSION.tar.gz" ]; then
    wget "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"
fi

echo "[2/4] Extracting sources..."
tar -xf "binutils-$BINUTILS_VERSION.tar.gz"
tar -xf "gcc-$GCC_VERSION.tar.gz"

echo "[3/4] Building Binutils ($BINUTILS_VERSION)..."
mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$CORES
make install
cd ..

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
echo "[4/4] Building GCC ($GCC_VERSION)..."
mkdir -p build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$CORES
make all-target-libgcc -j$CORES
make install-gcc
make install-target-libgcc
cd ..

echo "==========================================================="
echo " Build Complete!"
echo " Your cross-compiler is located at: $PREFIX/bin"
echo " "
echo " *** IMPORTANT ***"
echo " Add the following line to your ~/.bashrc or ~/.zshrc:"
echo " export PATH=\"\$HOME/opt/cross/bin:\$PATH\""
echo "==========================================================="
