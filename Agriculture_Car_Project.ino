#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

#define dir1PinL  12
#define speedPinL 25
#define dir1PinR  27
#define speedPinR 23
#define BUZZ_PIN  21
#define SERVO_PIN 33
#define PUMP_PIN 16
#define LIGHT_PIN 17
Servo head;

enum Direction {STOPPED, FORWARD, BACKWARD, LEFT, RIGHT, LEFT_BACK, RIGHT_BACK};
Direction currentDir = STOPPED;
Direction targetDir = STOPPED;

int targetSpeedL = 0, targetSpeedR = 0;
int currentSpeedL = 1, currentSpeedR = 1;

int servoPos = 90;
unsigned long lastMotorUpdate = 0;
const unsigned long motorInterval = 10;

AsyncWebServer server(80);

void updateMotorSpeeds() {
  if (millis() - lastMotorUpdate >= motorInterval) {
    bool changed = false;

  

    // Direction change: wait for stop, then switch direction
    if ((currentSpeedL == 0 && currentSpeedR == 0) && currentDir != targetDir && targetDir != STOPPED) {
      if (targetDir == FORWARD) {
        digitalWrite(dir1PinL, HIGH);
        digitalWrite(dir1PinR, HIGH);
        targetSpeedL = 255;
        targetSpeedR = 255;
      } else if (targetDir == BACKWARD) {
        digitalWrite(dir1PinL, LOW);
        digitalWrite(dir1PinR, LOW);
        targetSpeedL = 255;
        targetSpeedR = 255;
      } else if (targetDir == LEFT) {
        digitalWrite(dir1PinL, HIGH);
        digitalWrite(dir1PinR, HIGH);
        targetSpeedL = 127;
        targetSpeedR = 255;
      } else if (targetDir == RIGHT) {
        digitalWrite(dir1PinL, HIGH);
        digitalWrite(dir1PinR, HIGH);
        targetSpeedL = 255;
        targetSpeedR = 127;
      } else if (targetDir == LEFT_BACK) {
        digitalWrite(dir1PinL, LOW);
        digitalWrite(dir1PinR, LOW);
        targetSpeedL = 127;
        targetSpeedR = 255;
      } else if (targetDir == RIGHT_BACK) {
        digitalWrite(dir1PinL, LOW);
        digitalWrite(dir1PinR, LOW);
        targetSpeedL = 255;
        targetSpeedR = 127;
      }
      currentDir = targetDir;
    }

    if (currentSpeedL != targetSpeedL) {
      currentSpeedL += (targetSpeedL > currentSpeedL) ? 1 : -1;
      analogWrite(speedPinL, currentSpeedL);
      changed = true;
    }

    if (currentSpeedR != targetSpeedR) {
      currentSpeedR += (targetSpeedR > currentSpeedR) ? 1 : -1;
      analogWrite(speedPinR, currentSpeedR);
      changed = true;
    }

    if (changed) {
      Serial.printf("Speed L: %d, Speed R: %d\n", currentSpeedL, currentSpeedR);
    }

    lastMotorUpdate = millis();
  }
}

void go_Advance() {
  if (currentDir == STOPPED || currentDir == LEFT || currentDir == RIGHT ) {
    digitalWrite(dir1PinL, HIGH);
    digitalWrite(dir1PinR, HIGH);
    currentDir = FORWARD;
    targetSpeedL = 255;
    targetSpeedR = 255;
  } else if (currentDir != FORWARD) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = FORWARD;
  }
}

void go_Back() {
  if (currentDir == STOPPED) {
    digitalWrite(dir1PinL, LOW);
    digitalWrite(dir1PinR, LOW);
    currentDir = BACKWARD;
    targetSpeedL = 255;
    targetSpeedR = 255;
    
  } else if (currentDir == LEFT_BACK || currentDir == RIGHT_BACK) {
    currentDir = BACKWARD;
    targetSpeedL = 255;
    targetSpeedR = 255;
  } else if (currentDir != BACKWARD) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = BACKWARD;
  }
}

void go_Left() {
  if (currentDir == STOPPED) {
    digitalWrite(dir1PinL, HIGH);
    digitalWrite(dir1PinR, HIGH);
    currentDir = LEFT;
    targetSpeedL = 127;
    targetSpeedR = 255;
  } 
  else if (currentDir == FORWARD || currentDir == RIGHT) {
    currentDir = LEFT;
    targetSpeedL = 127;
    targetSpeedR = 255;
  } else if (currentDir != LEFT) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = LEFT;
  }
}

void go_Right() {
  if (currentDir == STOPPED) {
    digitalWrite(dir1PinL, HIGH);
    digitalWrite(dir1PinR, HIGH);
    currentDir = RIGHT;
    targetSpeedL = 255;
    targetSpeedR = 127;
  } 
  else if (currentDir == FORWARD || currentDir == LEFT) {
    currentDir = RIGHT;
    targetSpeedL = 255;
    targetSpeedR = 127;
   } else if (currentDir != RIGHT) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = RIGHT;
  }
}
void go_LeftBack() {
  if (currentDir == STOPPED) {
    digitalWrite(dir1PinL, LOW);
    digitalWrite(dir1PinR, LOW);
    currentDir = LEFT_BACK;
    targetSpeedL = 127;
    targetSpeedR = 255;
  } else if (currentDir == BACKWARD || currentDir == RIGHT_BACK) {
    currentDir = LEFT_BACK;
    targetSpeedL = 127;
    targetSpeedR = 255;
  } else if (currentDir != LEFT_BACK) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = LEFT_BACK;
  }
}

void go_RightBack() {
  if (currentDir == STOPPED) {
    digitalWrite(dir1PinL, LOW);
    digitalWrite(dir1PinR, LOW);
    currentDir = RIGHT_BACK;
    targetSpeedL = 255;
    targetSpeedR = 127;
  } else if (currentDir == BACKWARD || currentDir == LEFT_BACK) {
    currentDir = RIGHT_BACK;
    targetSpeedL = 255;
    targetSpeedR = 127;
  } else if (currentDir != RIGHT_BACK) {
    targetSpeedL = 0;
    targetSpeedR = 0;
    targetDir = RIGHT_BACK;
  }
}

void stop_Stop() {
  targetSpeedL = 0;
  targetSpeedR = 0;
  targetDir = STOPPED;
  currentDir = STOPPED;
}
void pump_ON() {
  digitalWrite(PUMP_PIN, HIGH);
}
void pump_OFF() {
  digitalWrite(PUMP_PIN, LOW);
}
void light_ON() {
  digitalWrite(LIGHT_PIN, HIGH);
}
void light_OFF() {
  digitalWrite(LIGHT_PIN, LOW);
}
void buzz_ON() {
  digitalWrite(BUZZ_PIN, HIGH);
}

void buzz_OFF() {
  digitalWrite(BUZZ_PIN, LOW);
}

void handleServoCommand(String command) {
  if (command == "servo_left") {
    servoPos = max(servoPos - 10, 0);
    head.write(servoPos);
  } else if (command == "servo_right") {
    servoPos = min(servoPos + 10, 180);
    head.write(servoPos);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(dir1PinL, OUTPUT);
  pinMode(speedPinL, OUTPUT);
  pinMode(dir1PinR, OUTPUT);
  pinMode(speedPinR, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  //analogWrite(speedPinL, 0);
  //analogWrite(speedPinR, 0);
  buzz_OFF();
  stop_Stop();
  head.attach(SERVO_PIN);
  head.write(90);

  WiFi.softAP("ESP32_Car_AP", "12345678");
  Serial.println("Access Point Created");

  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_Advance();
    request->send(200, "text/plain", "Moving Forward");
  });

  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_Back();
    request->send(200, "text/plain", "Moving Backward");
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_Left();
    request->send(200, "text/plain", "Turning Left");
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_Right();
    request->send(200, "text/plain", "Turning Right");
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    stop_Stop();
    request->send(200, "text/plain", "Stopped");
  });

  server.on("/servo_left", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleServoCommand("servo_left");
    request->send(200, "text/plain", "Servo Left");
  });

  server.on("/servo_right", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleServoCommand("servo_right");
    request->send(200, "text/plain", "Servo Right");
  });
  server.on("/servo_down", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleServoCommand("servo_right");
    request->send(200, "text/plain", "Servo Right");
  });
  server.on("/servo_up", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleServoCommand("servo_right");
    request->send(200, "text/plain", "Servo Right");
  });

  server.on("/buzzer_on", HTTP_GET, [](AsyncWebServerRequest *request) {
    buzz_ON();
    request->send(200, "text/plain", "Buzzer On");
  });

  server.on("/buzzer_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    buzz_OFF();
    request->send(200, "text/plain", "Buzzer Off");
  });
  server.on("/left_back", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_LeftBack();
    request->send(200, "text/plain", "Turning Left Backward");
  });

  server.on("/right_back", HTTP_GET, [](AsyncWebServerRequest *request) {
    go_RightBack();
    request->send(200, "text/plain", "Turning Right Backward");
  });
  server.on("/light_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("LIGHT OFF!");
    light_OFF();
    request->send(200, "text/plain", "Turning Right Backward");
  });
  server.on("/light_on", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("LIGHT ON!");
    light_ON();
    request->send(200, "text/plain", "Turning Right Backward");
  });
  server.on("/pump_on", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("PUMP ON!");
    pump_ON();
    request->send(200, "text/plain", "Turning Right Backward");
  });
  server.on("/pump_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("PUMP OFF!");
    pump_OFF();
    request->send(200, "text/plain", "Turning Right Backward");
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  updateMotorSpeeds();
}
