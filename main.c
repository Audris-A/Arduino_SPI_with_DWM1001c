/*
    Things to be done:
      1) Add timestamp to messages;
      2) Add SD Card logging;
      
    Output example (distance values are in mm):
      {
        "nodeId": 192C, 
        "distances": 
           [
             {
               "anchorId": D68B, 
               "distance": 956 
             }, 
             {
               "anchorId": C8B4, 
               "distance": 2313 
             }, 
             {  
               "anchorId": 921F, 
               "distance": 4038 
             }, 
             {
               "anchorId": 489B, 
               "distance": 4933 
             }
           ]
      } 
      {
        "nodeId": 192C, 
        "position":
          {
            "x":4.198,
            "y":2.284,
            "z":0.763,
            "quality":68
          }
      }^B01^arduinoTest^23^room
*/

#include <SPI.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

SPISettings settingsA(8000000, MSBFIRST, SPI_MODE0); 

//A replacement for timestamp (not a good one, just so it is possible to differentiate messages)
//  could be replaced with millis()
int msgCounter = 0;

void setup() {
  pinMode(10,OUTPUT);
  Serial.begin(115200);
  SPI.begin();
}

void loop() {
  char locationBuffer[500];
  char distanceBuffer[300];
  char anchorDistanceInfo[70];
  
  char testName[12] = "arduinoTest\0";
  char arduinoId[4] = "B01\0";
  char zoneId[5] = "room\0";

  // For debugging purposes
  uint8_t wholeData[1500]; // Contains the whole received message
  uint32_t iterator = 0;
  
  uint8_t data[10][50]; // User data message array
  uint8_t rxSize = 0x00; // Transfer size (0 - 255)
  uint8_t rxNum = 0x00; // Transfer count (0 - n)
  uint8_t dummy = 0x00;
  uint16_t anchorDistance = 0x0000;
  
  int32_t xCoordinate = 0x00000000;
  int32_t yCoordinate = 0x00000000;
  int32_t zCoordinate = 0x00000000;
  
  uint8_t userMessageCount = 0; // How many user messages are in one transfer
  uint8_t userDataLength = 29; // Length of user data to work with
  uint8_t userDataLengthIt = 29;
  uint8_t checkByteCount = 0x05; // Count of check bytes (sign of user data)
  uint8_t checkByte = 0x99; // The byte to check by.
  uint8_t dataCheckByteOccurance = 0x00; // Contains counter for check bytes (0x00 - 0x05)
  
  SPI.beginTransaction(settingsA);
  digitalWrite(10, LOW);
  delay(1);
  
  dummy = SPI.transfer(0x37);
  dummy = SPI.transfer(0x02);
  dummy = SPI.transfer(0x01);
  dummy = SPI.transfer(0x00);
  
  digitalWrite(10,HIGH);
  delay(1);

  // Get the transfer size and the number of transfers to be held
  while (rxSize == 0x00) {
    digitalWrite(10,LOW);
    delay(1);
    rxSize = SPI.transfer(0xFF);
    rxNum = SPI.transfer(0xFF);
    digitalWrite(10,HIGH);
    delay(1);
  }

  // For debugging purposes
  /*wholeData[iterator] = rxSize;
  iterator++;
  wholeData[iterator] = rxNum;
  iterator++;*/
  
  // Make rxNum transfers with rxSize byte count. 
  //   Format for working with transfers, but
  //   only the first transfer contains tag data. Transfers go from 6E + rxNum.
  //   For defined request it is 6E + 0x05, which means that there
  //   will be 5 transfers with msg types from 6E to 72.
  //   More info in the Decawave documentation "DWM1001 Firmware API Guide"
  for (byte j = 0; j < rxNum; j++) {
    digitalWrite(10,LOW);
    delay(1);
    for (byte i = 0; i < rxSize; i++) {
      wholeData[iterator] = SPI.transfer(0xFF);
      
      if (dataCheckByteOccurance == checkByteCount && userDataLengthIt != 0) {
        // read 34 bytes of user data
        data[userMessageCount][userDataLength - userDataLengthIt] = wholeData[iterator];
        userDataLengthIt--;
      } else if (wholeData[iterator] == checkByte && dataCheckByteOccurance != checkByteCount && userDataLengthIt == userDataLength) {
        dataCheckByteOccurance++;
      } else if (dataCheckByteOccurance != checkByteCount && wholeData[iterator] != checkByte){
        dataCheckByteOccurance = 0x00;
        userDataLengthIt = userDataLength;
      }

      if (userDataLengthIt == 0x00){
        userMessageCount++;
        userDataLengthIt = userDataLength;
        dataCheckByteOccurance = 0x00;
      }
      
      iterator++;
    }
    digitalWrite(10,HIGH);
    delay(1);
  }
  
  delay(100);
  SPI.endTransaction();

  // The message should always have the checked rxSize and rxNum
  //   but it's checked just in case. The only transfer that is
  //   being logged is the first one, which holds the tag data. Other transfer
  //   data is being ignored, because for now it is only 0x00.
  if(rxSize == 0xFF && rxNum == 0x05){
    // For debugging
    /*Serial.print("\nWhole data start....\n"); 
    for (int i = 0; i < iterator; i++) {
      Serial.print(wholeData[i],HEX);
      Serial.print(" ");
      if (i % 12 == 0) {
        Serial.print("\n");
      }
    }
    Serial.print("\nWhole data end....\n");*/
    
    //Serial.print("\n");
    /*Serial.print("Start...."); 
    Serial.print("\n");*/
    
    for (int it_msg = 0; it_msg < userMessageCount; it_msg++) {
      /*Serial.print("Node user start....\n"); 
      Serial.print("Node id: ");
      Serial.print(data[it_msg][1], HEX);
      Serial.print(data[it_msg][0], HEX);
      Serial.print("\n");
      Serial.print("Anchor count: ");
      Serial.print(data[it_msg][2], HEX);
      Serial.print("\n");*/
      
      sprintf(distanceBuffer, "{\"nodeId\": %02X%02X, \"distances\": [", data[it_msg][1], data[it_msg][0]);

      uint8_t anchorIt = 4;
      uint8_t distanceIt = 5;
      // Loop through anchor count
      for (uint8_t i = 0; i < data[it_msg][2]; i++){
          /*Serial.print("Anchor id: ");
          Serial.print(data[it_msg][anchorIt], HEX);
          Serial.print(data[it_msg][anchorIt-1], HEX);
          Serial.print("\n");*/
          
          /*Serial.print("Distance to anchor: ");
          Serial.print(data[it_msg][distanceIt], HEX);*/
          anchorDistance = data[it_msg][distanceIt+1] << 8;
          //Serial.print(data[it_msg][distanceIt+1], HEX);
          anchorDistance += data[it_msg][distanceIt];
          //Serial.print("\n");

          sprintf(anchorDistanceInfo, "{\"anchorId\": %02X%02X, \"distance\": %d }", data[it_msg][anchorIt], data[it_msg][anchorIt-1],
                                                                                     (int) anchorDistance);
          strcat(distanceBuffer, anchorDistanceInfo);
          strcat(distanceBuffer, ", ");

          //delay(100);
          /*Serial.print(anchorDistance, DEC);
          Serial.print("\n");*/
          anchorIt += 4;
          distanceIt += 4;
          anchorDistance = 0x0000;
      }

      Serial.print("\n");
      distanceBuffer[strlen(distanceBuffer) - 2] = ']';
      distanceBuffer[strlen(distanceBuffer) - 1] = '}';
      
      // Print tag anchor distances
      //   This should be logged in an sd card
      Serial.print(distanceBuffer);
      //Serial.print("\n");
      
      /*Serial.print("x coordinate: ");
      Serial.print(data[it_msg][19], HEX);
      Serial.print(data[it_msg][20], HEX);
      Serial.print(data[it_msg][21], HEX);
      Serial.print("\n");*/
      
      xCoordinate = (int16_t) data[it_msg][21] << 16;
      xCoordinate += (int16_t) data[it_msg][20] << 8;
      xCoordinate += (int16_t) data[it_msg][19];
      //Serial.print(xCoordinate, DEC);
      //Serial.print("\n");

      /*Serial.print("y coordinate: ");
      Serial.print(data[it_msg][22], HEX);
      Serial.print(data[it_msg][23], HEX);
      Serial.print(data[it_msg][24], HEX);
      Serial.print("\n");*/
      
      yCoordinate = (int16_t) data[it_msg][24] << 16;
      yCoordinate += (int16_t) data[it_msg][23] << 8;
      yCoordinate += (int16_t) data[it_msg][22];
      //Serial.print(yCoordinate, DEC);
      
      //Serial.print("\n");
      /*Serial.print("z coordinate: ");
      Serial.print(data[it_msg][25], HEX);
      Serial.print(data[it_msg][26], HEX);
      Serial.print(data[it_msg][27], HEX);
      Serial.print("\n");*/

      zCoordinate = (int16_t) data[it_msg][27] << 16;
      zCoordinate += (int16_t) data[it_msg][26] << 8;
      zCoordinate += (int16_t) data[it_msg][25];
      //Serial.print(zCoordinate, DEC);
      
      Serial.print("\n");
      
      /*Serial.print("Quality factor: ");
      Serial.print(data[it_msg][userDataLength-1], DEC);
      Serial.print("\n");*/

      // Workaround for printing floating point numbers
      uint16_t decimalValue = abs(xCoordinate) % 1000;
      uint16_t metres = abs(xCoordinate)/1000;
      char xReal[20];
      if (xCoordinate < 0){
        sprintf(xReal, "-%" PRIu16 ".%" PRIu16, metres, decimalValue);
      } else {
        sprintf(xReal, "%" PRIu16 ".%" PRIu16, metres, decimalValue);
      }

      decimalValue = abs(yCoordinate) % 1000;
      metres = abs(yCoordinate)/1000;
      char yReal[20];
      if (yCoordinate < 0){
        sprintf(yReal, "-%" PRIu16 ".%" PRIu16, metres, decimalValue);
      } else {
        sprintf(yReal, "%" PRIu16 ".%" PRIu16, metres, decimalValue);
      }
  
      decimalValue = abs(zCoordinate) % 1000;
      metres = abs(zCoordinate)/1000;
      char zReal[20];
      if (zCoordinate < 0){
        sprintf(zReal, "-%" PRIu16 ".%" PRIu16, metres, decimalValue);
      } else {
        sprintf(zReal, "%" PRIu16 ".%" PRIu16, metres, decimalValue);
      }

      sprintf(locationBuffer, "{\"nodeId\": %02X%02X, \"position\":{\"x\":%s,\"y\":%s,\"z\":%s,\"quality\":%d}}^%s^%s^%d^%s", 
                             data[it_msg][1],
                             data[it_msg][0],
                             xReal,
                             yReal,
                             zReal, 
                             (int) data[it_msg][userDataLength-1], 
                             arduinoId, testName, msgCounter, zoneId);
      
      msgCounter++;

      // Print tag location information
      //   This should be logged in an sd card
      Serial.print(locationBuffer);
      /*Serial.print("\n");
      for (int it = 0; it < userDataLength; it++) {
          Serial.print(data[it_msg][it],HEX);
          Serial.print(" ");
          if (it % 10 == 0 && it != 0) {
              Serial.print("\n");  
          }
      }*/
      //Serial.print("Node user end....\n"); 
    }
    /*Serial.print("\n");
    Serial.print("End...."); 
    Serial.print("\n");*/
  }
}