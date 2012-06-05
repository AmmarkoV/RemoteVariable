#!/bin/bash
echo "RemoteVariables  / Provides libRemoteVariableSupport.a  / Author : Ammar Qammaz"    


 
if [ -e libRemoteVariableSupport.a ]
then
  rm libRemoteVariableSupport.a
fi 

cd src
  
Optimizations="-O3 -fexpensive-optimizations -s"
CPU="-mfpmath=sse -mtune=core2 -msse -msse2 -msse3"
 
gcc  $Optimizations $CPU -c HashFunctions.c -o HashFunctions.o 
gcc  $Optimizations $CPU -c InternalTester.c -o InternalTester.o 
gcc  $Optimizations $CPU -c MessageTables.c -o MessageTables.o 
gcc  $Optimizations $CPU -c NetworkFramework.c -o NetworkFramework.o  
gcc  $Optimizations $CPU -c Peers.c -o Peers.o  
gcc  $Optimizations $CPU -c RemoteVariableSupport.c -o RemoteVariableSupport.o  
gcc  $Optimizations $CPU -c SocketAdapterToMessageTables.c -o SocketAdapterToMessageTables.o  
gcc  $Optimizations $CPU -c VariableDatabase.c -o VariableDatabase.o  
gcc  $Optimizations $CPU -c helper.c -o helper.o  

FILESTOLINK="HashFunctions.o InternalTester.o helper.o MessageTables.o  NetworkFramework.o Peers.o   RemoteVariableSupport.o SocketAdapterToMessageTables.o  VariableDatabase.o"

ar  rcs libRemoteVariableSupport.a $FILESTOLINK
rm $FILESTOLINK

cd RemoteVariableClone
./make
cd ..

cd RemoteVariableMaster
./make
cd ..

cd GamesTester
./make
cd ..

cd ..

cp src/libRemoteVariableSupport.a libRemoteVariableSupport.a

if [ -e libRemoteVariableSupport.a ]
then
  echo "Success.."
else
  echo "Failure.."
fi

exit 0
