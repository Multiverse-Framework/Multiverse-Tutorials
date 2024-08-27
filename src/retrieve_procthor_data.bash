#!/bin/bash

cd $(dirname $0)

if [ ! -d "ai2thor" ]; then
    git clone git@github.com:allenai/ai2thor.git --depth 1
fi

python get_meshes.py

python3.11 extract_objects.py