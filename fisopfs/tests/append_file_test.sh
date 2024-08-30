#!/bin/bash
DIRECTORY=$1
if [  $#  != 1 ]; then
  echo "Parámatero invalido de entrada en bash test."
fi

echo -e "\n"
echo "Test comenzando..."
echo "-------------------------------------------------------------"
echo "------------------------<<_cat test_>>-----------------------"
echo "Cambiando a directorio de pruebas..."
echo "Ejecutando => cd 'directorio de pruebas'"
cd $DIRECTORY
if [ $? -eq 1 ]; then
  echo "No se pudo cambiar de directorio"
  exit 1
fi
echo "Ejecutando => echo hola mundo! >> new_file_A"
echo "------------------------------"
echo "Resultado:"
echo "------------------------------"
echo hola mundo! >> new_file_A
echo "Ejecutando => echo | cat new_file_A"
[ -s new_file_A ] && echo "new_file_A contiene información" || echo "new_file_A está vacío"
echo "-------------------------------------------------------------"
echo "Test end"