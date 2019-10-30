#!/bin/bash

set -euo pipefail

SUBMISSION_ZIP="${1}"

echo "Checking submission file: $SUBMISSION_ZIP"

if [ ! -f "$SUBMISSION_ZIP" ]; then
  echo FAIL
  exit
fi

echo -ne "Checking file name...\t\t"
if [ "$(basename $SUBMISSION_ZIP)" == "project1.zip" ]; then
  echo OK
else
  echo FAIL
  exit
fi

echo -ne "Checking file type...\t\t"
if [ "$(file -bi $SUBMISSION_ZIP)" == "application/zip; charset=binary" ]; then
  echo OK
else
  echo FAIL
  exit
fi

echo "Checking submission contents."

WORK_DIR="$(mktemp -d)"

function cleanup {
  echo "Cleaning up."
  rm -rf "$WORK_DIR"
  killall chat-client 2>/dev/null || true
  killall chat-server 2>/dev/null || true
}
trap cleanup EXIT

cp "$SUBMISSION_ZIP" "$WORK_DIR"
cd "$WORK_DIR"
unzip project1.zip

echo -ne "Checking for Makefile...\t"
if [ -f "Makefile" ]; then
  echo "OK"
else
  echo "FAIL"
  exit
fi

echo "Building project."

make

echo -ne "Checking for chat-client...\t"
if [ -x "chat-client" ]; then
  echo "OK"
else
  echo "FAIL"
  exit
fi

echo -ne "Checking for chat-server...\t"
if [ -x "chat-server" ]; then
  echo "OK"
else
  echo "FAIL"
  exit
fi

echo "Running project."

./chat-server 1234 >/dev/null 2>/dev/null &
SERVER_PID=$!

(sleep .5) | ./chat-client localhost 1234 >chat-client.out 2>/dev/null &
RCLIENT_PID=$!
sleep .1
RCLIENT_IPPORT=$(netstat -np 2>/dev/null | awk "\$7 == \"$RCLIENT_PID/./chat-client\" {print \$4}")

(echo Test; sleep .3) | ./chat-client localhost 1234 >/dev/null 2>/dev/null &
SCLIENT_PID=$!
sleep .1
SCLIENT_IPPORT=$(netstat -np 2>/dev/null | awk "\$7 == \"$SCLIENT_PID/./chat-client\" {print \$4}")

wait $SCLIENT_PID || true
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
wait $RCLIENT_PID || true

echo -ne "Checking output format...\t"
if diff chat-client.out - <<EOF
$RCLIENT_IPPORT joined.
$SCLIENT_IPPORT joined.
$SCLIENT_IPPORT Test
$SCLIENT_IPPORT left.
EOF
then
  echo OK
else
  echo FAIL
  exit
fi

echo "All basic checks passed."
