#!/bin/bash
DIRECTORY=$1
if [  $#  != 1 ]; then
  echo "Parámatero invalido de entrada en bash test."
fi

echo -e "\n"
echo "Test comenzando..."
echo "-------------------------------------------------------------"
echo "------------------------<<_ls test_>>------------------------"
echo "Cambiando a directorio de pruebas..."
echo "Ejecutando => cd 'directorio de pruebas'"
cd $DIRECTORY
if [ $? -eq 1 ]; then
  echo "No se pudo cambiar de directorio"
  exit 1
fi
echo "Ejecutando => ls"
echo "------------------------------"
echo "Resultado:"
echo "------------------------------"
ls
echo "-------------------------------------------------------------"
echo "Test end"