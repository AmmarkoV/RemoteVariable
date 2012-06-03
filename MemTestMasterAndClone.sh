#!/bin/bash
./KillMasterAndClone.sh 

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes  --track-origins=yes src/RemoteVariableMaster/bin/Debug/RemoteVariableSupportTester 2> MasterDebug.msg & 
sleep 1
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes  --track-origins=yes src/RemoteVariableClone/bin/Debug/RemoteVariableTesterClient 2> ClientDebug.msg&
sleep 23
./KillMasterAndClone.sh 


exit 0
