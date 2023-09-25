#!/bin/bash

if ! command -v clang &>/dev/null; then
	gcc server.c common.c -o server -Wall -g
else
	clang server.c common.c -o server -Wall -g
fi
