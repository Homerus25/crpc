#!/bin/bash

build() {
  cmake ..
  #cmake  --build . --target clean 
  cmake  --build . --target crpc-http-ws-server crpc-http-client crpc-ws-client crpc-mqtt-server crpc-mqtt-client crpc-http2-server crpc-http2-client
}

check() {
  local exitCode=$1
  local test=$2
  if [ "$exitCode" != "0" ]; then
        echo "Test $test failed!!!"
        exit 1
    fi
}

test-websocket() {
  ./crpc-http-ws-server &
  local PIDserver=$!
  sleep 1

  ./crpc-ws-client
  local exitCode=$?

  kill $PIDserver

  check $exitCode "websocket"
}

test-http() {
  ./crpc-http-ws-server &
  local PIDserver=$!
  sleep 1

  ./crpc-http-client
  local exitCode=$?

  kill $PIDserver

  check $exitCode "http"
}

test-http2() {
  ./crpc-http2-server &
  local PIDserver=$!
  sleep 1

  ./crpc-http2-client
  local exitCode=$?

  kill $PIDserver

  check $exitCode "http"
}

test-mqtt() {
  ./crpc-mqtt-server 1 &
  local PIDserver=$!
  sleep 1

  ./crpc-mqtt-client
  local exitCode=$?

  kill $PIDserver

  check $exitCode "mqtt"
}

build

echo "test websocket"
test-websocket

echo "test http"
test-http

echo "test http2"
test-http

echo "test mqtt"
test-mqtt

exit 0
