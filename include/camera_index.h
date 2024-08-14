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
    <button id="startButton" onclick="startStream()">Start Stream</button>
    <button id="stopButton" onclick="stopStream()">Stop Stream</button>
    <br><br>
    <img id="stream" src="" style="display:none;">
    <script>
        const streamElement = document.getElementById('stream');

        function startStream() {
            streamElement.src = "/stream";
            streamElement.style.display = "block";
        }

        function stopStream() {
            streamElement.src = "";
            streamElement.style.display = "none";
        }

        // Optionally add a way to auto-refresh the image if it's not working
        streamElement.onerror = function() {
            console.error('Stream error encountered. Trying to reconnect...');
            setTimeout(() => {
                startStream();
            }, 5000); // Try to reconnect stream every 5 seconds
        };
    </script>
</body>
</html>
)rawliteral";

#endif // CAMERA_INDEX_H_
