#!/bin/bash

if ! command -v clang &>/dev/null; then
	gcc client.c -o client -Wall -g
else
	clang client.c -o client -Wall -g
fi
