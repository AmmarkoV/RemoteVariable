#!/bin/bash
./KillMasterAndClone.sh 
src/GamesTester/bin/Release/GamesTester  2> MasterDebug.msg &  
src/GamesTester/bin/Release/GamesTester 2> ClientDebug.msg&

exit 0
