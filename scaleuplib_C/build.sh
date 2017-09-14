#!/bin/bash
gcc -o scaleupapi scaleupapi.c jsmn.c -lconfig -lzmq -lm
