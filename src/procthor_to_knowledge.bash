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

IN_USD=${HOUSE_DIR}/house_${HOUSE_NUMBER}.usda

TBOX_USD=$PWD/ontology/SOMA_DFL_module.usda
TBOX_OWL=$PWD/ontology/SOMA_DFL_module.owl
USD_SEMANTIC_REPORTED=${HOUSE_DIR}/house_${HOUSE_NUMBER}_semantic_reported.usda
USD_SEMANTIC_TAGGED=${HOUSE_DIR}/house_${HOUSE_NUMBER}_semantic_tagged.usda
USD_SEMANTIC_TAGGED_FLATTEN=${HOUSE_DIR}/house_${HOUSE_NUMBER}_semantic_tagged_flatten.usda

OUT_OWL=${HOUSE_DIR}/house_${HOUSE_NUMBER}.owl

# Semantic reporting on the USD (Optional)

semantic_reporting --in_usd=${IN_USD} --in_TBox_usd=${TBOX_USD} --out_usd=${USD_SEMANTIC_REPORTED}

# Semantic tagging on the USD

python semantic_tagging.py --in_usd=${IN_USD} --in_TBox_usd=${USD_SEMANTIC_REPORTED} --out_ABox_usd=${USD_SEMANTIC_TAGGED}

# # Flatten the USD

usdcat ${USD_SEMANTIC_TAGGED} -o ${USD_SEMANTIC_TAGGED_FLATTEN} --flatten

# # Clean up the flatten USD

python clean_up_usd.py --in_usd=${USD_SEMANTIC_TAGGED_FLATTEN} --out_usd=${USD_SEMANTIC_TAGGED_FLATTEN}

# # Conver the USD into OWL

usd_to_ABox --in_usd=${USD_SEMANTIC_TAGGED} --in_owl=${TBOX_OWL} --out_owl=${OUT_OWL}