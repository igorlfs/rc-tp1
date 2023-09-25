#!/bin/bash

if ! command -v clang &>/dev/null; then
	gcc client.c common.c -o client -Wall -g
else
	clang client.c common.c -o client -Wall -g
fi
