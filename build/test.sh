#!/bin/bash

cmake --build . --target http-ws-server rpc-http-client rpc-ws-client crpc-server-mqtt crpc-client-mqtt

# test websocket
./http-ws-server &
PIDserver=$!

sleep 1

./rpc-ws-client
ECws=$?
kill $PIDserver

if [ "$ECws" != "0" ]; then
    echo "Test websocket failed!!!!!"
    echo "Exit with: $ECws\n"
    echo $ECws
    exit -1
fi



# test http
./http-ws-server &
PIDserver2=$!

sleep 1

./rpc-http-client
EChttp=$?
kill $PIDserver2

if [ "$EChttp" != "0" ]; then
    echo "Test http failed!!!!!"
    exit -1
fi





# test mqtt
./crpc-server-mqtt 1 &
PIDserver2=$!

sleep 1

./crpc-client-mqtt
EChttp=$?
kill $PIDserver2

if [ "$EChttp" != "0" ]; then
    echo "Test mqtt failed!!!!!"
    exit -1
fi


exit 0
