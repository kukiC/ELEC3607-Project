#ifndef STATUS_H
#define STATUS_H

#include "Arduino.h"
#include "sdcard.h"
#include "ArduinoAudioFile.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <DMD2.h>
#include <SPI.h>
#include <fonts/Arial14.h>

#define Xres 32
#define Yres 16
#define blueToothSerial Serial2




class Status
{
public:
  /* Constructor */
  Status(void);

  /* Deconstructor */
  ~Status(void);

  /* Functions */

  // initialize SD card module
  void sdIni();

  // Decode command sent by user
  int decodeCommand(int *recvInt);
  bool loadSong(int recvNum);
  void printFileList();
  bool playSong(int recvNum);
  void ledDisplay(int start);
  void ledCircle(int start);

  // Print command list
  void printCommand();
  void printLed(String str);


private:
  /* Variables */
  char const *command_list[7] = {"choose a song to play!", "next song", "random playing mode",  "pause or play", "reset", "load a song", "print command list"};
  sdcard card;
  double *real;
  ArduinoAudioFile music;
  int *data;
  uint16_t fftSize;

  void initialize();


};

#endif
