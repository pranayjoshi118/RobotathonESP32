// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 Ricardo Quesada
// http://retro.moe/unijoysticle2


#include "sdkconfig.h"
#include <Arduino.h>
#include <Bluepad32.h>
#include <uni.h>
#include "controller_callbacks.h"
#include <Wire.h>
#include <Arduino_APDS9960.h>
#include <bits/stdc++.h>
#define APDS9960_INT 0
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 100000
#include <QTRSensors.h>
uint16_t sensors[2];
QTRSensors qtr;

TwoWire I2C_0 = TwoWire(0);
APDS9960 apds = APDS9960(I2C_0, APDS9960_INT);

#define IN1  16  // Motor 1 pin 1
#define IN2  17  // Motor 1 pin 2
#define IN3  19  // Motor 2 pin 1
#define IN4  18  // Motor 2 pin 2

extern ControllerPtr myControllers[BP32_MAX_GAMEPADS]; // BP32 library allows for up to 4 concurrent controller connections, but we only need 1

void moveForward();
void moveBackward();
/* void getDistance();
void getColor();
void turnLeft();
void turnRight();
*/
void stopMotors();


void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "DPAD: %d A: %d B: %d X: %d Y: %d LX: %d LY: %d RX: %d RY: %d L1: %d R1: %d L2: %d R2: %d\n",
        ctl->dpad(),        // D-pad
        ctl->a(),           // Letter buttons 
        ctl->b(),
        ctl->x(),
        ctl->y(),
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->l1(),           // Bumpers
        ctl->r1(),
        ctl->l2(),
        ctl->r2()
    );
}

const int deadzone = 40; //controller deadzone, prevents stick drift   

void runController(ControllerPtr ctl) {
    int y = ctl->axisY();
    
    // Process controller inputs and execute corresponding actions
    if (y < -deadzone) { //joystick pushed forward
        moveForward();
    } else if (y > deadzone) { //joystick pushed backward
        moveBackward();
    } else {
        stopMotors();
    }
}

void setup() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys(); 
    esp_log_level_set("gpio", ESP_LOG_ERROR); // Suppress info log spam from gpio_isr_service
    uni_bt_allowlist_set_enabled(true);

    pinMode(IN1, OUTPUT); //defining motor pins as outputs
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);   
    pinMode(IN4, OUTPUT);

    //sets up I2C protocol
            I2C_0.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

            //sets up color sensor
            apds.setInterruptPin(APDS9960_INT);
            apds.begin();
            Serial.begin(115200);
            // set up Serial Communication and line sensor pins
            qtr.setTypeAnalog(); // or setTypeAnalog()
            qtr.setSensorPins((const uint8_t[]) {32, 33}, 2); // pin numbers go in the curly brackets {}, and number of sensors in use goes after

            // line sensor calibration sequence
            for (uint8_t i = 0; i < 250; i++) { 
            Serial.printf("calibrating %d/250\n", i); // 250 is the number of calibrations recommended by manufacturer
            qtr.calibrate(); 
            delay(20);
        
            }
        
        }


 void loop() {
    int r, g, b, a;
        while (!apds.colorAvailable()) { delay(5); } // Wait until color is read from the sensor 
            apds.readColor(r, g, b, a);
          //  Serial.printf("RED: %d GREEN: %d BLUE: %d AMBIENT: %d\n", r, g, b, a);
            delay(100); // Ensures WDT does not get triggered when no controller is connected
            BP32.update(); 

            qtr.readLineBlack(sensors); // Get calibrated sensor values returned into sensors[]
            Serial.printf("S1: %d S2: %d\n", sensors[0], sensors[1]);
            delay(250);


         for (auto myController : myControllers) { // Only execute code when controller is connected
           if (myController && myController->isConnected() && myController->hasData()) {        
           
            runController(myController); // process and executes controller inputs when connected
            Serial.printf("OVER HERE\n");

            dumpGamepad(myController); // Prints the gamepad state, delete or comment if don't need
        
       }

      
    }

   // stopMotors(); // Stops motors when controller is disconnected
    vTaskDelay(10); // Ensures WDT does not get triggered when no controller is connected
}

void moveForward() {
    analogWrite(IN1, 255);
    digitalWrite(IN2, LOW);
    analogWrite(IN3, 255);
    digitalWrite(IN4, LOW);
}

void moveBackward() {
    analogWrite(IN1, 255);
    digitalWrite(IN2, HIGH);
    analogWrite(IN3, 255);
    digitalWrite(IN4, HIGH);
}


void stopMotors() {
    analogWrite(IN1, 0);
    digitalWrite(IN1, LOW);
    analogWrite(IN3, 0);
    digitalWrite(IN4, LOW);
}
/*
void turnLeft() {
    digitalWrite(IN1, LOW); //left motor backward
    analogWrite(IN2, 200);
    analogWrite(IN3, 200); //right motor forward
    digitalWrite(IN4, 255);  
}

void turnRight() {
    analogWrite(IN1, 200); //left motor forward
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); //right motor backward
    analogWrite(IN4, 200);
}
*/