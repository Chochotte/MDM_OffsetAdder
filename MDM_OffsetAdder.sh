#! /bin/sh

while [ $# -gt 1 ]; 
do
	echo "Execution du programme pour : $1 sur le fichier $2"  
	mv $2 $2_BAK
	./main.exe $1 $2
	shift
	shift
done
