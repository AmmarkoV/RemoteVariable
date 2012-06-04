#!/bin/bash
lsof -i :12345

killall GamesTester
killall RemoteVariableSupportTester
killall RemoteVariableS

killall RemoteVariableT
killall RemoteVariableTesterClient
killall memcheck-amd64-

lsof -i :12345
exit 0
