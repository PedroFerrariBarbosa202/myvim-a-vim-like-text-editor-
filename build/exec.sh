#!/bin/bash

FILE=$1
DIR=$2

SOURCE="$DIR/$FILE"
EXEC="./program"
OUTPUT="output.txt"
INPUT="input.txt"

# Clear output file
: >"$OUTPUT"

# Check source exists
if [ ! -f "$SOURCE" ]; then
  echo "File not found: $SOURCE" >"$OUTPUT"
  exit 1
fi

# Compile
g++ "$SOURCE" -o "$EXEC" 2>>"$OUTPUT"

if [ $? -ne 0 ]; then
  echo "Compilation failed." >>"$OUTPUT"
  exit 1
fi

# Execute
"$EXEC" <"$INPUT" >>"$OUTPUT" 2>&1
STATUS=$?

if [ $STATUS -ne 0 ]; then
  echo "" >>"$OUTPUT"
  echo "Program exited with status $STATUS" >>"$OUTPUT"
fi

echo "Finished. Output written to $OUTPUT"
