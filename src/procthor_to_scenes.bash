#!/bin/bash

HOUSE_NUMBER=$1

# Check if the user input is a number
if ! [[ ${HOUSE_NUMBER} =~ ^[0-9]+$ ]]; then
  echo "Please enter a valid house number"
  exit 1
fi

cd $(dirname $0)

RESOURCES_DIR=$PWD/../resources

HOUSE_DIR=$RESOURCES_DIR/house_${HOUSE_NUMBER}

# Get the house from the Procthor

python get_house.py --house=${HOUSE_NUMBER}

# # Convert the house into USD

python procthor_to_scene.py --house=${HOUSE_NUMBER} --output_dir=${RESOURCES_DIR}