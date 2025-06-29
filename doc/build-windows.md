WINDOWS BUILD NOTES
====================

Some notes on how to build Bitcoin Classic for Windows.

Most developers use cross-compilation from Ubuntu to build executables for
Windows. This is also used to build the release binaries.

Building on Windows itself is possible (for example using msys / mingw-w64),
but no one documented the steps to do this. If you are doing this, please contribute them.

Recommended Build Environment (DevContainer)
---------------------

For reproducible and portable Windows builds, Bitcoin Classic uses a DevContainer environment. This ensures consistent compilation and produces portable executables that work across different Windows versions.

### Using DevContainer (Recommended)

1. **Prerequisites:**
   - Visual Studio Code with Remote-Containers extension
   - Docker installed and running

2. **Setup:**
   ```bash
   git clone https://github.com/Bitcoinclassicxbt/classic2.git
   cd classic2
   code .  # Open in VS Code
   # When prompted, click "Reopen in Container"
   ```

3. **Build Windows Executables in DevContainer:**
   ```bash
   # Build dependencies for Windows x86_64
   cd depends
   make HOST=x86_64-w64-mingw32 -j$(nproc)
   
   # Build Bitcoin Classic for Windows
   cd ..
   ./autogen.sh
   ./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32
   
   make -j$(nproc)
   ```

   The DevContainer provides:
   - Pre-configured MinGW-w64 cross-compilation toolchain
   - All required dependencies
   - Optimized build flags for Windows compatibility

Cross-compilation (Local Development)
-------------------

If not using DevContainer, these steps can be performed on, for example, an Ubuntu VM. The depends system
will also work on other Linux distributions, however the commands for
installing the toolchain will be different.

Make sure you install the build requirements mentioned in
[build-unix.md](/doc/build-unix.md).

Then, install the toolchains and curl:

    sudo apt-get install g++-mingw-w64-i686 mingw-w64-i686-dev g++-mingw-w64-x86-64 mingw-w64-x86-64-dev curl

### Configure MinGW alternatives (recommended):

    sudo update-alternatives --install /usr/bin/x86_64-w64-mingw32-gcc x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix 100
    sudo update-alternatives --install /usr/bin/x86_64-w64-mingw32-g++ x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix 100

### To build executables for Windows 32-bit:

    cd depends
    make HOST=i686-w64-mingw32 -j$(nproc)
    cd ..
    ./autogen.sh
    ./configure --prefix=$(pwd)/depends/i686-w64-mingw32
    make -j$(nproc)

### To build executables for Windows 64-bit (Recommended):

    cd depends
    make HOST=x86_64-w64-mingw32 -j$(nproc)
    cd ..
    ./autogen.sh
    ./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32
    make -j$(nproc)

Bitcoin Classic Specific Notes
---------------------

### Windows Compatibility
Bitcoin Classic Windows builds include:
- Support for Windows 7 and later
- Static linking for better portability
- Optimized compiler flags for performance
- All dependencies bundled (no external DLL requirements)

### Build Outputs
After successful compilation, you'll find these executables in the `src/` directory:
- `bitcoind.exe` - Bitcoin Classic daemon (command line)
- `bitcoin-cli.exe` - RPC client for daemon interaction
- `bitcoin-tx.exe` - Transaction manipulation utility
- `qt/bitcoin-qt.exe` - Bitcoin Classic GUI wallet

### Creating Release Packages

To create a distributable Windows package:

```bash
cd src

# Create Windows release directory
mkdir -p bitcoin-classic-0.13.4-windows-x86_64

# Copy executables
cp bitcoind.exe bitcoin-classic-0.13.4-windows-x86_64/
cp bitcoin-cli.exe bitcoin-classic-0.13.4-windows-x86_64/
cp bitcoin-tx.exe bitcoin-classic-0.13.4-windows-x86_64/
cp qt/bitcoin-qt.exe bitcoin-classic-0.13.4-windows-x86_64/

# Create zip package
zip -r bitcoin-classic-0.13.4-windows-x86_64.zip bitcoin-classic-0.13.4-windows-x86_64/

# Clean up
rm -rf bitcoin-classic-0.13.4-windows-x86_64/
```

Troubleshooting
---------------

### Common Issues

1. **MinGW Toolchain Issues:**
   - Ensure the posix version of MinGW is being used (configure alternatives above)
   - For Ubuntu 18.04+, the default MinGW may have threading issues

2. **Missing Dependencies:**
   - DevContainer pre-installs all dependencies
   - For local builds, ensure all packages from build-unix.md are installed

3. **Build Failures:**
   - Try building with `-j1` instead of `-j$(nproc)` if running out of memory
   - Ensure Docker has sufficient resources (4GB+ RAM) when using DevContainer

4. **Runtime Issues:**
   - Windows executables built with cross-compilation should run on Windows 7+
   - If DLL errors occur, the build may not be properly static-linked

### Windows-Specific Configure Options

For specialized Windows builds, additional configure options are available:

```bash
# Build without GUI (smaller executable)
./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32 --without-gui

# Build without wallet functionality
./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32 --disable-wallet

# Build with debug symbols
./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32 --enable-debug
```

### Testing Windows Builds on Linux

You can test Windows executables on Linux using Wine:

```bash
# Install Wine
sudo apt-get install wine

# Test the executable
wine src/bitcoind.exe --version
wine src/qt/bitcoin-qt.exe  # For GUI testing
```

DevContainer vs Local Builds
----------------------------

**DevContainer Advantages:**
- Consistent, reproducible builds
- Pre-configured cross-compilation environment
- No host system dependency conflicts
- Optimized for Bitcoin Classic specifically
- Works on Windows, macOS, and Linux hosts

**Local Build Advantages:**
- Faster iteration for development
- Can customize toolchain versions
- No Docker dependency

For release builds and distribution, always use the DevContainer environment to ensure maximum compatibility and reproducibility.

For further documentation on the depends system see [README.md](../depends/README.md) in the depends directory.

Additional Resources
-------------------

- **Bitcoin Classic Website:** https://www.classicxbt.com
- **GitHub Repository:** https://github.com/Bitcoinclassicxbt/classic2
- **Build Issues:** https://github.com/Bitcoinclassicxbt/classic2/issues
- **Cross-compilation Guide:** [build-unix.md](build-unix.md)
