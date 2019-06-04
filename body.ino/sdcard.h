#ifndef SDCARD_H
#define SDCARD_H

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#define CS 53
#define blueToothSerial Serial2

class sdcard
{
public:
  sdcard(void);
  ~sdcard(void);
  File getEntry(void);
  void printFileList(void);
  int fileNum(void);
  bool sdIni(void);
  String getSongName(int num);
private:
  bool ini(void);
  void retrFiles(File root);
  String file_list[20];
  int counter;
};

#endif
