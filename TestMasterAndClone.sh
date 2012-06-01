#!/bin/bash
./KillMasterAndClone.sh 
src/RemoteVariableMaster/bin/Debug/RemoteVariableSupportTester 2> MasterDebug.msg & 
sleep 1
src/RemoteVariableClone/bin/Debug/RemoteVariableTesterClient 2> ClientDebug.msg&

sleep 23
./KillMasterAndClone.sh 

exit 0
