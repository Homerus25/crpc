#!/bin/bash
set -e

cmake ..
cmake  --build . --target crpc-http-ws-server crpc-http-client crpc-ws-client crpc-mqtt-server crpc-mqtt-client

# test websocket
./crpc-http-ws-server &
PIDserver=$!

sleep 1

./crpc-ws-client
ECws=$?
kill $PIDserver

if [ "$ECws" != "0" ]; then
    echo "Test websocket failed!!!!!"
    echo "Exit with: $ECws\n"
    echo $ECws
    exit -1
fi



# test http
./crpc-http-ws-server &
PIDserver2=$!

sleep 1

./crpc-http-client
EChttp=$?
kill $PIDserver2

if [ "$EChttp" != "0" ]; then
    echo "Test http failed!!!!!"
    exit -1
fi





# test mqtt
./crpc-mqtt-server 1 &
PIDserver2=$!

sleep 1

./crpc-mqtt-client
EChttp=$?
kill $PIDserver2

if [ "$EChttp" != "0" ]; then
    echo "Test mqtt failed!!!!!"
    exit -1
fi


exit 0
