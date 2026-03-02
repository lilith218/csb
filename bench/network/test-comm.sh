#!/bin/bash
# Copyright (C) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# SPDX-License-Identifier: MIT

PACKET_SPEC="2r1023-1w32"
NUM_CLIENTS=3
PORT=5000

CLIENT_PIDS=()
# launch clients in the background and save their PIDs
for i in $(seq 1 "$NUM_CLIENTS"); do
  echo "Launching client#$i on port $PORT"
  # -O the clients will just try to call readwrite once
  # -R the clients will keep retrying until they manage to connect to the server
  ./client -h localhost -p $PORT -P $PACKET_SPEC -O -R &
  CLIENT_PIDS+=($!)
done

echo "Launching server on port $PORT"
./server -p $PORT -P $PACKET_SPEC &
SERVER_PID=$!

A_CLIENT_FAILED=0
# wait for each client to exit
for pid in "${CLIENT_PIDS[@]}"; do
  wait "$pid"
  ret=$?
  echo "Client#$i exited with $ret"
  if [ $ret -ne 0 ]; then
    A_CLIENT_FAILED=1
  fi
done
echo "Killing server"
kill $SERVER_PID
# exit will be one if one of the clients fail
exit $A_CLIENT_FAILED

