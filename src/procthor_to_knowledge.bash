#!/bin/bash

HOUSE_NUMBER=$1

# Check if the user input is a number
if ! [[ ${HOUSE_NUMBER} =~ ^[0-9]+$ ]]; then
  echo "Please enter a valid house number"
  exit 1
fi

echo "Translate house ${HOUSE_NUMBER} to knowledge"

cd $(dirname "$0") || exit

OUTPUT_DIR=$PWD/../output

HOUSE_DIR=$OUTPUT_DIR/house_${HOUSE_NUMBER}

IN_USD=${HOUSE_DIR}/house_${HOUSE_NUMBER}.usda

TBOX_USD=$PWD/ontology/SOMA_DFL_module.usda
TBOX_OWL=$PWD/ontology/SOMA_DFL_module.owl
USD_SEMANTIC_TAGGED=${HOUSE_DIR}/house_${HOUSE_NUMBER}_semantic_tagged.usda
USD_SEMANTIC_TAGGED_FLATTEN=${HOUSE_DIR}/house_${HOUSE_NUMBER}_semantic_tagged_flatten.usda

OUT_OWL=${HOUSE_DIR}/house_${HOUSE_NUMBER}.owl

# Semantic tagging on the USD
echo "Running semantic tagging on ${IN_USD}"
python3 semantic_tagging.py --in_usd="${IN_USD}" --in_TBox_usd="${TBOX_USD}" --out_ABox_usd="${USD_SEMANTIC_TAGGED}"

# Flatten the USD
echo "Flatten the semantic tagged USD to ${USD_SEMANTIC_TAGGED_FLATTEN}"
usdcat "${USD_SEMANTIC_TAGGED}" -o "${USD_SEMANTIC_TAGGED_FLATTEN}" --flatten

# Clean up the flatten USD
echo "Clean up the flatten semantic tagged USD to ${USD_SEMANTIC_TAGGED_FLATTEN}"
python3 clean_up_usd.py --in_usd="${USD_SEMANTIC_TAGGED_FLATTEN}" --out_usd="${USD_SEMANTIC_TAGGED_FLATTEN}"

# Conver the USD into OWL
echo "Convert the cleaned flatten semantic tagged USD to OWL ${OUT_OWL}"
python3 usd_to_ABox.py --in_usd="${USD_SEMANTIC_TAGGED}" --in_owl="${TBOX_OWL}" --out_owl="${OUT_OWL}"