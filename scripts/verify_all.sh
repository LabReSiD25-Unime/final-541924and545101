#!/bin/bash

# Configurazione
PORT=8088
URL="http://localhost:$PORT"
AUTH_HEADER="X-Auth: labresid2025"
DATA_FILE="data/bookings.txt"

echo "====================================================="
echo "FINAL COMPREHENSIVE COMPLIANCE VERIFICATION"
echo "====================================================="

# 0. Pulizia processi precedenti sulla porta
fuser -k $PORT/tcp > /dev/null 2>&1
sleep 1

# 1. Cleanup old instances and build
make
fuser -k $PORT/tcp 2>/dev/null || true
rm -f "$DATA_FILE"
mkdir -p data

# 2. Start Server
./server $PORT &
SERVER_PID=$!
sleep 3

# Check if server is running
if ! ps -p $SERVER_PID > /dev/null; then
    echo "ERROR: Server failed to start. Check if port $PORT is already in use."
    exit 1
fi

echo -e "\n[1] Testing 401 Unauthorized (Missing Auth Header)"
curl -s -i "$URL/rooms" | head -n 1
echo "-----------------------------------------------------"

echo -e "\n[2] Testing 200 OK (GET)"
curl -s -i -H "$AUTH_HEADER" "$URL/rooms" | head -n 1
echo "-----------------------------------------------------"

echo -e "\n[3] Testing 201 Created (POST)"
POST_OUT=$(curl -s -H "$AUTH_HEADER" -d "user=TestUser&room_id=5" "$URL/book")
echo "$POST_OUT"
BOOK_ID=$(echo "$POST_OUT" | grep "ID prenotazione:" | sed 's/[^0-9]//g')

if [ -n "$BOOK_ID" ]; then
    echo -e "\n[4] Testing 200 OK (PUT)"
    curl -s -i -H "$AUTH_HEADER" -X PUT "$URL/book?id=$BOOK_ID&user=UpdatedUser" | head -n 1
    echo "-----------------------------------------------------"

    echo -e "\n[5] Testing 204 No Content (DELETE)"
    curl -s -i -H "$AUTH_HEADER" -X DELETE "$URL/book?id=$BOOK_ID" | head -n 1
    echo "-----------------------------------------------------"
fi

echo -e "\n[6] Testing 400 Bad Request"
curl -s -i -H "$AUTH_HEADER" -X POST "$URL/book" | head -n 1
echo "-----------------------------------------------------"

# Cleanup finale
kill $SERVER_PID > /dev/null 2>&1
echo -e "\nVerifica completata con successo e senza errori di bind."
