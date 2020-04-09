#!/bin/bash

for i in {1..990};
do
    if [ ! -e "./src/ai/training_data/${1}_${i}_observation.zip" ]; then
        echo "./rocksndiamonds_headless -levelset $1 -loadlevel $i -controller two_level -loglevel 4" >> train_commands.txt
    fi
done