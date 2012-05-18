#!/bin/bash
src/RemoteVariableMaster/bin/Debug/RemoteVariableSupportTester 2> MasterDebug.msg & 
src/RemoteVariableClone/bin/Debug/RemoteVariableTesterClient 2> ClientDebug.msg&
sleep 3
killall RemoteVariableSupportTester
killall RemoteVariableTesterClient
exit 0
