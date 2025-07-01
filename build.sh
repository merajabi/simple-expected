#!/bin/bash

# python3.10  -m venv venv

source venv/bin/activate
 
# Step 1: Install tools
conan install conanfile_tools.txt --output-folder=build/conan-tools --build=missing --profile:build=$1 --profile:host=$1

# Step 2: Activate environment
source build/conan-tools/conanbuild.sh

# Step 3: Verify tools
echo "Using CMake: $(which cmake)"
cmake --version

# Step 4: Install main package
conan install . --output-folder=build/conan-libs --build=missing --profile:build=$1 --profile:host=$1

# Step 5: Activate environment
source build/conan-libs/conanbuild.sh

# Step 5: Build (optional)
conan build . --output-folder=build --profile:build=$1 --profile:host=$1

# Step 6: Test (optional)
#conan test . SimpleExpected/1.0@ --profile:build=$1 --profile:host=$1 
ctest --test-dir build/build/Debug/

conan create . -s build_type=Debug --profile:build=$1 --profile:host=$1 

