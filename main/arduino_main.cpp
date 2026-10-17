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

TwoWire I2C_0 = TwoWire(0);
APDS9960 apds = APDS9960(I2C_0, APDS9960_INT);

extern ControllerPtr myControllers[BP32_MAX_GAMEPADS]; // BP32 library allows for up to 4 concurrent controller connections, but we only need 1

void dumpGamepad(ControllerPtr ctl) {
    Console.printf(
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

void setup() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys(); 
    esp_log_level_set("gpio", ESP_LOG_ERROR); // Suppress info log spam from gpio_isr_service
    uni_bt_allowlist_set_enabled(true);
    //sets up I2C protocol
            I2C_0.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

            //sets up color sensor
            apds.setInterruptPin(APDS9960_INT);
            apds.begin();
            Serial.begin(115200);
          
}



 void loop() {
    int r, g, b, a;
            while (!apds.colorAvailable()) { delay(5); } // Wait until color is read from the sensor 
            apds.readColor(r, g, b, a);
            Serial.printf("RED: %d GREEN: %d BLUE: %d AMBIENT: %d\n", r, g, b, a);
            delay(100); // Ensures WDT does not get triggered when no controller is connected
            BP32.update(); 
         for (auto myController : myControllers) { // Only execute code when controller is connected
          if (myController && myController->isConnected() && myController->hasData()) {        
          

            


            dumpGamepad(myController); // Prints the gamepad state, delete or comment if don't need
       }
    }
}