#!/bin/bash

if ! command -v clang &>/dev/null; then
	gcc server.c -o server -Wall -g
else
	clang server.c -o server -Wall -g
fi
