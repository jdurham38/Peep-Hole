#ifndef CAMERA_INDEX_H_
#define CAMERA_INDEX_H_

#include <pgmspace.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM Stream</title>
    <style>
        #stream {
            width: 640px;
            height: 480px;
        }
    </style>
</head>
<body>
    <h1>ESP32-CAM Video Stream</h1>
    <img src="http://10.0.0.157/stream" id="stream" width="640" height="480">
<script>
    const stream = document.getElementById('stream');
    let eventSource;
    let reconnectAttempts = 0;

    function updateStream() {
        stream.src = 'http://10.0.0.157/stream?' + new Date().getTime();
    }

    function tryReconnectEventSource() {
        console.log('Attempting to reconnect EventSource...');
        setupEventSource();
    }

    function setupEventSource() {
        if (eventSource) {
            eventSource.close();
        }

        eventSource = new EventSource('/events');
        eventSource.onmessage = function(event) {
            if (event.data === 'Stream started') {
                updateStream();
            }
        };

        eventSource.onerror = function(event) {
            console.error('EventSource failed:', event);
            eventSource.close();
            reconnectAttempts++;
            setTimeout(tryReconnectEventSource, Math.min(30000, 5000 * reconnectAttempts)); // Incremental backoff
        };
    }

    setupEventSource();

    // Optionally add a way to auto-refresh the image if it's not working
    stream.onerror = function() {
        console.error('Stream error encountered. Trying to reconnect...');
        setTimeout(updateStream, 5000); // Try to reconnect stream every 5 seconds
    };
</script>

</body>
</html>
)rawliteral";

#endif // CAMERA_INDEX_H_
