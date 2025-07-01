#!/bin/bash

# python3.10  -m venv venv

source venv/bin/activate
 
# Step 1: Install tools
conan install conanfile_tools.txt --output-folder=build/tools --build=missing --profile:build=$1 --profile:host=$1

# Step 2: Activate environment
source build/tools/conanbuild.sh

# Step 3: Verify tools
echo "Using CMake: $(which cmake)"
cmake --version

# Step 4: Install main package
conan install . --output-folder=build/depend --build=missing --profile:build=$1 --profile:host=$1

# Step 5: Activate environment
source build/depend/conanbuild.sh

# Step 5: Build (optional)
conan build . --output-folder=build/Debug --profile:build=$1 --profile:host=$1

# Step 6: Test (optional)
#conan test . SimpleExpected/1.0@ --output-folder=build/Debug --profile:build=$1 --profile:host=$1 
ctest --test-dir build/Debug/

