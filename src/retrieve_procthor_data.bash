#!/bin/bash

cd $(dirname "$0") || exit

if [ ! -d "ai2thor" ]; then
    git clone git@github.com:allenai/ai2thor.git --depth 1
fi

python3 get_meshes.py

python3.11 extract_objects.py