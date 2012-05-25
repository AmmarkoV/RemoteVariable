
TEST STEP 1
Variable Changed !
Job_SingalLocalVariableChanged broadcasting to all peers ( 1 ) 
Job_SingalLocalVariableChanged broadcasting to peer number 0 
Is this correct ? 
Added JOB 1 , our variable 0 must be shared with peer 0 -  SIGNALCHANGED  @ 12
1 variables changed and 0 variables refreshed
...Execution of Singal Changed to peer : 3 of variable SHARED_VAR with var id 0 
Trying to send `SIG=SHARED_VAR` to peer socket 3 
Trying RecvRAWFrom
Received `OK` from peer socket 3  
Successfull Signal change handshaking..!
.........Execution of Singal Changed to peer : 3 of variable SHARED_VAR with var id 0 
Trying to send `SIG=SHARED_VAR` to peer socket 3 
.ProtocolServeResponse peeked on a Get Share request 
Trying RecvRAWFrom
Received `GET=SHARED_VAR` from peer socket 3  
Received GET=SHARED_VAR request for variable
Peer Signaled that it wants a variable SHARED_VAR changed 
TODO Add check for wether it is in security/policy to send him the variable requested
Trying to send `OK` to peer socket 3 
Trying RecvRAWFrom
Sending GeneralPacket
Sending Payload
SendVariableTo , Great Success :) 
Received `GET=SHARED_VAR` from peer socket 3  
ERROR MESSAGE : Error at signal change handshaking : 1
Could not signal change
........ProtocolServeResponse peeked on a Get Share request 
Trying RecvRAWFrom
Received `GET=SHARED_VAR` from peer socket 3  
Received GET=SHARED_VAR request for variable
Peer Signaled that it wants a variable SHARED_VAR changed 
TODO Add check for wether it is in security/policy to send him the variable requested
Trying to send `OK` to peer socket 3 
Sending GeneralPacket
Sending Payload
SendVariableTo , Great Success :) 
.Execution of Singal Changed to peer : 3 of variable SHARED_VAR with var id 0 
Trying to send `SIG=SHARED_VAR` to peer socket 3 
Trying RecvRAWFrom
Received `GET=SHARED_VAR` from peer socket 3  
ERROR MESSAGE : Error at signal change handshaking : 1
Could not signal change
.........Execution of Singal Changed to peer : 3 of variable SHARED_VAR with var id 0 