#!/bin/bash
export SERVER_ADDRESS="0.0.0.0"
export SERVER_PORT="8001"
make
./SetEnv 8001 0.0.0.0
