#!/bin/bash

# Script to create Bitcoin Classic 0.13.4 release packages for both Windows and Linux
# Run this from the classic2/src directory

set -e  # Exit on any error

# Set variables
VERSION="0.13.4"
WINDOWS_ARCH="windows-x86_64"
LINUX_ARCH="linux-x86_64"

# Function to create Windows release
create_windows_release() {
    echo "=== Creating Windows Release ==="
    
    local RELEASE_NAME="bitcoin-classic-${VERSION}-${WINDOWS_ARCH}"
    local ZIP_FILE="${RELEASE_NAME}.zip"
    
    # Check if Windows executables exist
    if [[ ! -f "bitcoind.exe" ]]; then
        echo "ERROR: Windows executables not found!"
        echo "Please build Windows binaries first with:"
        echo "cd depends && make HOST=x86_64-w64-mingw32 -j\$(nproc)"
        echo "cd .. && ./configure --prefix=\$(pwd)/depends/x86_64-w64-mingw32 && make -j\$(nproc)"
        return 1
    fi
    
    # Create temporary directory for the release
    mkdir -p "${RELEASE_NAME}"
    
    # Copy Windows executables to release directory
    echo "Copying Windows executables..."
    cp bitcoind.exe "${RELEASE_NAME}/"
    cp bitcoin-cli.exe "${RELEASE_NAME}/"
    cp bitcoin-tx.exe "${RELEASE_NAME}/"
    cp qt/bitcoin-qt.exe "${RELEASE_NAME}/"
    
    # Verify all files are present
    echo "Verifying Windows files..."
    if [[ -f "${RELEASE_NAME}/bitcoind.exe" && \
          -f "${RELEASE_NAME}/bitcoin-cli.exe" && \
          -f "${RELEASE_NAME}/bitcoin-tx.exe" && \
          -f "${RELEASE_NAME}/bitcoin-qt.exe" ]]; then
        echo "All Windows executables found!"
    else
        echo "ERROR: Missing Windows executables!"
        rm -rf "${RELEASE_NAME}"
        return 1
    fi
    
    # Create the zip file
    echo "Creating ${ZIP_FILE}..."
    zip -r "${ZIP_FILE}" "${RELEASE_NAME}/"
    
    # Show file sizes
    echo "Windows file sizes:"
    ls -lh "${RELEASE_NAME}/"*.exe
    
    # Show zip info
    echo "Windows archive created:"
    ls -lh "${ZIP_FILE}"
    
    # Clean up temporary directory
    rm -rf "${RELEASE_NAME}"
    
    echo "Windows release package ${ZIP_FILE} created successfully!"
    echo ""
}

# Function to create Linux release
create_linux_release() {
    echo "=== Creating Linux Release ==="
    
    local RELEASE_NAME="bitcoin-classic-${VERSION}-${LINUX_ARCH}"
    local TAR_FILE="${RELEASE_NAME}.tar.gz"
    
    # Check if Linux executables exist
    if [[ ! -f "bitcoind" ]]; then
        echo "ERROR: Linux executables not found!"
        echo "Please build Linux binaries first with:"
        echo "cd depends && make HOST=x86_64-pc-linux-gnu -j\$(nproc)"
        echo "cd .. && ./configure --prefix=\$(pwd)/depends/x86_64-pc-linux-gnu && make -j\$(nproc)"
        return 1
    fi
    
    # Create temporary directory for the release
    mkdir -p "${RELEASE_NAME}"
    
    # Copy Linux executables to release directory
    echo "Copying Linux executables..."
    cp bitcoind "${RELEASE_NAME}/"
    cp bitcoin-cli "${RELEASE_NAME}/"
    cp bitcoin-tx "${RELEASE_NAME}/"
    
    # Check if GUI version exists
    if [[ -f "qt/bitcoin-qt" ]]; then
        cp qt/bitcoin-qt "${RELEASE_NAME}/"
        echo "GUI version included"
    else
        echo "Warning: GUI version (bitcoin-qt) not found, skipping..."
    fi
    
    # Make executables executable
    chmod +x "${RELEASE_NAME}/"*
    
    # Verify core files are present
    echo "Verifying Linux files..."
    if [[ -f "${RELEASE_NAME}/bitcoind" && \
          -f "${RELEASE_NAME}/bitcoin-cli" && \
          -f "${RELEASE_NAME}/bitcoin-tx" ]]; then
        echo "All core Linux executables found!"
    else
        echo "ERROR: Missing core Linux executables!"
        rm -rf "${RELEASE_NAME}"
        return 1
    fi
    
    # Create the tar.gz file
    echo "Creating ${TAR_FILE}..."
    tar -czf "${TAR_FILE}" "${RELEASE_NAME}/"
    
    # Show file sizes
    echo "Linux file sizes:"
    ls -lh "${RELEASE_NAME}/"*
    
    # Show tar info
    echo "Linux archive created:"
    ls -lh "${TAR_FILE}"
    
    # Show tar contents
    echo "Archive contents:"
    tar -tzf "${TAR_FILE}"
    
    # Clean up temporary directory
    rm -rf "${RELEASE_NAME}"
    
    echo "Linux release package ${TAR_FILE} created successfully!"
    echo ""
}

# Function to show summary
show_summary() {
    echo "=== Release Summary ==="
    echo "Created release packages:"
    
    local WINDOWS_ZIP="bitcoin-classic-${VERSION}-${WINDOWS_ARCH}.zip"
    local LINUX_TAR="bitcoin-classic-${VERSION}-${LINUX_ARCH}.tar.gz"
    
    if [[ -f "${WINDOWS_ZIP}" ]]; then
        echo "✓ Windows: ${WINDOWS_ZIP} ($(du -h "${WINDOWS_ZIP}" | cut -f1))"
    fi
    
    if [[ -f "${LINUX_TAR}" ]]; then
        echo "✓ Linux: ${LINUX_TAR} ($(du -h "${LINUX_TAR}" | cut -f1))"
    fi
    
    echo ""
    echo "All release packages created successfully!"
}

# Main execution
echo "Bitcoin Classic ${VERSION} Release Builder"
echo "=========================================="
echo ""

# Check if we're in the right directory
if [[ ! -f "Makefile" ]] || [[ ! -d "qt" ]]; then
    echo "ERROR: Please run this script from the classic2/src directory"
    exit 1
fi

# Create releases
create_windows_release || echo "Windows release failed"
create_linux_release || echo "Linux release failed"

# Show summary
show_summary
