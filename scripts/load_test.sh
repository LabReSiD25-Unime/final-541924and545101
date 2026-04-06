#!/bin/bash

# Get the script's directory and root directory
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$ROOT_DIR" || exit 1

# Configuration
PORT=${1:-8080}
URL="http://localhost:$PORT"
AUTH_HEADER="X-Auth: labresid2025"
REQUESTS=20

echo "Starting Load Test with $REQUESTS concurrent requests on port $PORT..."

# Function to perform a request
perform_request() {
    local i=$1
    # Alternating between GET rooms and POST book
    if [ $((i % 2)) -eq 0 ]; then
        curl -s -o /dev/null -w "Request #$i: POST /book -> HTTP %{http_code}\n" \
             -H "$AUTH_HEADER" -d "user=LoadTest_$i&room_id=$(( (i % 5) + 1 ))" "$URL/book"
    else
        curl -s -o /dev/null -w "Request #$i: GET /rooms -> HTTP %{http_code}\n" \
             -H "$AUTH_HEADER" "$URL/rooms"
    fi
}

# Run requests in parallel
for ((i=1; i<=REQUESTS; i++)); do
    perform_request $i &
done

# Wait for all background processes to finish
wait

echo "Load Test completed."
