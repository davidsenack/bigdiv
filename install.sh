#!/bin/bash

# Compile the program
make

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful."

    # Copy the executable to /usr/local/bin
    sudo cp bin/program /usr/local/bin

    # Check if copy was successful
    if [ $? -eq 0 ]; then
        echo "Installation successful. The program is now available system-wide."
    else
        echo "Error: Failed to copy the executable to /usr/local/bin. Installation failed."
        exit 1
    fi
else
    echo "Error: Compilation failed. Installation aborted."
    exit 1
fi

