#!/bin/bash

DIR_MOUNT="prueba"
RED='\e[31m'
GREEN='\e[32m'
NC='\e[0m'

chmod +x tests/*.sh

mkdir $DIR_MOUNT
chmod 755 "$DIR_MOUNT"
make > /dev/null
./fisopfs $DIR_MOUNT/

mount | grep fisopfs > /dev/null
if [ $? -eq 0 ]; then
  echo "fisopfs montado"
else
  echo "fisopfs no montado"
  exit 1
fi

test_scripts=(
    "tests/mkdir_test.sh"
    "tests/touch_test.sh"
    "tests/ls_test.sh"
    "tests/stat_test.sh"
    "tests/append_file_test.sh"
    "tests/cat_test.sh"
    "tests/overwrite_file_test.sh"
    "tests/rm_test.sh"
    "tests/rmdir_test.sh"
)

results=()

tests_fallados=0

for script in "${test_scripts[@]}"; do
    echo "Ejecutando $script..."

    script_name=$(basename "$script" .sh)

    expected_file="tests/${script_name}.txt"

    output=$("$script" "$DIR_MOUNT") > /dev/null

    if [ -f "$expected_file" ]; then
        expected_output=$(cat "$expected_file")
    else
        echo -e "${RED}Error:${NC} Archivo esperado ${expected_file} no encontrado"
        continue
    fi

    if [ "$output" == "$expected_output" ]; then
        result="${GREEN} PASSED ${NC}"
        echo -e "$result"
    else
        result="${RED} FAILED ${NC}"
        tests_fallados=1
        echo -e "$result"
        results+=("Resultado del $script: $result")
        results+=("Output del $script:")
        results+=("$output")
        results+=("-----------------------------------")
        results+=("Esperado:")
        results+=("$expected_output")
        results+=("-------------------------------------------------------------")
    fi


done

echo -e "\n"

if [ $tests_fallados -eq 0 ]; then
    echo "Todos los tests pasaron"
else
  echo -e "\nResultados de las pruebas:"
  for result in "${results[@]}"; do
      echo -e "$result"
  done
fi

umount $DIR_MOUNT
rm -r $DIR_MOUNT

if [ $tests_fallados -eq 1 ]; then
    exit 1
else
    exit 0
fi
