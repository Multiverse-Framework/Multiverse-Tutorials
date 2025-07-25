#!/bin/bash

HOUSE_NUMBER=$1

# Check if the user input is a number
if ! [[ ${HOUSE_NUMBER} =~ ^[0-9]+$ ]]; then
  echo "Please enter a valid house number"
  exit 1
fi

echo "Get house ${HOUSE_NUMBER}"

cd $(dirname "$0") || exit

OUTPUT_DIR=$PWD/../output

# Get the house from the Procthor

python3 get_house.py --house="${HOUSE_NUMBER}"

# # Convert the house into USD

python3 procthor_to_scene.py --house="${HOUSE_NUMBER}" --output_dir="${OUTPUT_DIR}"