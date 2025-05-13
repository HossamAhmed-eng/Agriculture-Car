#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>

#define LIGHT_PIN 4

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 2;
const int PWMLightChannel = 3;

//Camera related constants
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid     = "ESP32_Car_AP";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
uint32_t cameraClientId = 0;

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
    .arrows {
      font-size:40px;
      color:red;
    }
    td.button {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 15px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }
     .control-panel {
            display: grid;
            grid-template-areas:
                ". forward ."
                "left stop right"
                ". backward .";
            gap: 10px;
            margin-bottom: 20px;
        }
        .control-button {
            padding: 10px 25px;
            font-size: 18px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            background-color: #007bff;
            color: white;
            transition: background-color 0.3s;
        }
        .control-button:active {
            background-color: #0056b3;
        }
        .control-button#forward { grid-area: forward; }
        .control-button#backward { grid-area: backward; }
        .control-button#left { grid-area: left; }
        .control-button#right { grid-area: right; }
        .control-button#stop { grid-area: stop; background-color: #dc3545; }
        .control-button#stop:active { background-color: #a71d2a; }
        .servo-control {
            margin-bottom: 20px;
        }
        .servo-control button {
            padding: 10px 20px;
            font-size: 16px;
            border-radius: 5px;
            margin: 5px;
        }
        .status-button {
            padding: 10px 20px;
            font-size: 16px;
            border-radius: 5px;
            margin: 5px;
            cursor: pointer;
        }
        .status-button.on {
            background-color: #28a745;
            color: white;
        }
        .status-button.off {
            background-color: #dc3545;
            color: white;
        }
        .status-button:active {
            opacity: 0.8;
        }
    </style>
   <script>
        // Toggle status for pump, buzzer, and light
        function toggleStatus(device) {
            const button = document.getElementById(device);
            let command = '';

            if (button.classList.contains('off')) {
                command = device + '_on';
                button.classList.remove('off');
                button.classList.add('on');
                button.innerText = device.charAt(0).toUpperCase() + device.slice(1) + ' ON';
            } else {
                command = device + '_off';
                button.classList.remove('on');
                button.classList.add('off');
                button.innerText = device.charAt(0).toUpperCase() + device.slice(1) + ' OFF';
            }

            }

    </script>

  </head>
  <body class="noselect" align="center" style="background-color:white">
     
    <!--h2 style="color: teal;text-align:center;">Wi-Fi Camera &#128663; Control</h2-->
    
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <img id="cameraImage" src="" style="width:400px;height:250px"></td>
      </tr> 
      <tr>
        <td></td>
      
  
    <div class="control-panel">
            <button class="control-button" id="forward" onclick="sendCommand('/forward')">Forward</button>
            <button class="control-button" id="backward" onclick="sendCommand('/backward')">Backward</button>
            <button class="control-button" id="left" onclick="sendCommand('/left')">Left</button>
            <button class="control-button" id="right" onclick="sendCommand('/right')">Right</button>
            <button class="control-button" id="backwardleft" onclick="sendCommand('/left_back')">Back Left</button>
            <button class="control-button" id="backwardright" onclick="sendCommand('/right_back')">Back Right</button>
            <button class="control-button" id="stop" onclick="sendCommand('/stop')">Stop</button>
            </div>
<div class="servo-control">
            <button class="control-button" onclick="sendCommand('/servo_left')">Pan Left</button>
            <button class="control-button" onclick="sendCommand('/servo_up')">Tilt Up</button>
            <button class="control-button" onclick="sendCommand('/servo_right')">Pan Right</button>
            <button class="control-button" onclick="sendCommand('/servo_down')">Tilt Down</button>
        </div>
<div>
            <button class="status-button off" id="pump" onclick=handleClick(1)>Pump OFF</button>
            <button class="status-button off" id="buzzer" onclick=handleClick(2)>Buzzer OFF</button>
            <button class="status-button off" id="light" onclick=handleClick(3)>Light OFF</button>

<script>
  let clickCounts = { 1: 0, 2: 0, 3: 0 };

  // Button 1 functions
  function button1FirstClick() {
    console.log('Button 1 - First Click Action');
    sendCommand('/pump_on')
      toggleStatus('pump')
  }
  function button1SecondClick() {
    sendCommand('/pump_off')
      toggleStatus('pump')
  }

  // Button 2 functions
  function button2FirstClick() {
    sendCommand('/buzzer_on')
      toggleStatus('buzzer')
         setTimeout(() => {
        sendCommand('/buzzer_off')
        toggleStatus('buzzer');
    }, 500);
  }

  // Button 3 functions
  function button3FirstClick() {
    sendCommand('/light_on')
      toggleStatus('light')
  }
  function button3SecondClick() {
   sendCommand('/light_off')
      toggleStatus('light')
  }

  // Handle clicks
  function handleClick(buttonNumber) {
    clickCounts[buttonNumber]++;

    if (clickCounts[buttonNumber] === 1) {
      if (buttonNumber === 1) button1FirstClick();
      else if (buttonNumber === 2) button2FirstClick();
      else if (buttonNumber === 3) button3FirstClick();
    } else if (clickCounts[buttonNumber] === 2) {
      if (buttonNumber === 1) button1SecondClick();
      else if (buttonNumber === 2) button2FirstClick();
      else if (buttonNumber === 3) button3SecondClick();
      clickCounts[buttonNumber] = 0;
    }
  }
</script>

        </div>
    <script>
      var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";
      var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";      
      var websocketCamera;
      var websocketCarInput;
      function sendCommand(command) {
            // Replace '192.168.4.1' with the IP address of your ESP32 web server
            fetch(`http://192.168.4.1${command}`)
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                })
                .catch(error => {
                    console.error('Error:', error);
                });
        }
      function initCameraWebSocket() 
      {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen    = function(event){};
        websocketCamera.onclose   = function(event){setTimeout(initCameraWebSocket, 2000);};
        websocketCamera.onmessage = function(event)
        {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }
      
      function initCarInputWebSocket() 
      {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen    = function(event)
        {
          var speedButton = document.getElementById("Speed");
          sendButtonInput("Speed", speedButton.value);
          var lightButton = document.getElementById("Light");
          sendButtonInput("Light", lightButton.value);
        };
        websocketCarInput.onclose   = function(event){setTimeout(initCarInputWebSocket, 2000);};
        websocketCarInput.onmessage = function(event){};        
      }
      
      function initWebSocket() 
      {
        initCameraWebSocket ();
        initCarInputWebSocket();
      }
    
      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";




void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCameraWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0;
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setupCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  config.frame_size = FRAMESIZE_240X240;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }  

  if (psramFound())
  {
    heap_caps_malloc_extmem_enable(20000);  
    Serial.printf("PSRAM initialized. malloc to take memory from psram above this size");    
  }  
}

void sendCameraPicture()
{
  if (cameraClientId == 0)
  {
    return;
  }
  unsigned long  startTime1 = millis();
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) 
  {
      Serial.println("Frame buffer could not be acquired");
      return;
  }

  unsigned long  startTime2 = millis();
  wsCamera.binary(cameraClientId, fb->buf, fb->len);
  esp_camera_fb_return(fb);
    
  //Wait for message to be delivered
  while (true)
  {
    AsyncWebSocketClient * clientPointer = wsCamera.client(cameraClientId);
    if (!clientPointer || !(clientPointer->queueIsFull()))
    {
      break;
    }
    delay(1);
  }
  
  unsigned long  startTime3 = millis();  
  Serial.printf("Time taken Total: %d|%d|%d\n",startTime3 - startTime1, startTime2 - startTime1, startTime3-startTime2 );
}



void setup(void) 
{

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
      
  wsCamera.onEvent(onCameraWebSocketEvent);
  server.addHandler(&wsCamera);

  server.begin();
  Serial.println("HTTP server started");

  setupCamera();
}


void loop() 
{
  wsCamera.cleanupClients(); 
  sendCameraPicture(); 
  delay(100);
  //Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
}