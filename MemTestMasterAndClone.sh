#!/bin/bash
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes src/RemoteVariableMaster/bin/Debug/RemoteVariableSupportTester 2> MasterDebug.msg & 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes src/RemoteVariableClone/bin/Debug/RemoteVariableTesterClient 2> ClientDebug.msg&
sleep 23
killall RemoteVariableSupportTester
killall RemoteVariableS

killall RemoteVariableT
killall RemoteVariableTesterClient
killall memcheck-amd64-

exit 0
