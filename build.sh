#!/bin/bash
# Simple build script for Ally Plugin System

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Building Ally Plugin System ===${NC}\n"

# Create build directory
if [ -d "build" ]; then
    echo -e "${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf build
fi

echo -e "${GREEN}Creating build directory...${NC}"
mkdir build
cd build

# Configure
echo -e "\n${GREEN}Configuring with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "\n${GREEN}Building...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "\n${GREEN}=== Build Complete! ===${NC}\n"

# Show what was built
echo -e "${GREEN}Built artifacts:${NC}"
ls -lh calculator_plugin* string_processor_plugin* c_host cpp_host 2>/dev/null || true

echo -e "\n${GREEN}To run examples:${NC}"
echo -e "  ${YELLOW}./c_host ./calculator_plugin${NC}"
echo -e "  ${YELLOW}./cpp_host ./string_processor_plugin${NC}\n"
