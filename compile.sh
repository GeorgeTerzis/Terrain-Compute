#!/bin/bash

O_LVL="0"
G_LVL="3"

g++ main.cpp -std=c++23 -O$O_LVL -g$G_LVL -lglfw -lGLEW -lGL -pthread
