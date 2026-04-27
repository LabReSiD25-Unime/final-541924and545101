#!/bin/bash
echo "--- Test 1: GET iniziale ---"
curl -u admin:password http://localhost:8080/reservations
echo -e "\n\n--- Test 2: POST (Creazione) ---"
ID_JSON=$(curl -s -u admin:password -X POST -d "room=AulaA&student=Massimo&time=10-12" http://localhost:8080/reservations)
echo "Risposta: $ID_JSON"
echo -e "\n--- Test 3: GET finale ---"
curl -u admin:password http://localhost:8080/reservations
echo -e "\n"
