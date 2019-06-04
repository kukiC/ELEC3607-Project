#ifndef ARDUINOAUDIOFILE_H
#define RDUINOAUDIOFILE_H
#include "Arduino.h"
#include <SD.h>
#include <math.h>
#include <arduinoFFT.h>

#define blueToothSerial Serial2
#define XRES 32
#define YRES 16


class ArduinoAudioFile
{
public:
  /* Constructor */
  ArduinoAudioFile();
  /* Deconstructor */
  ~ArduinoAudioFile();

  bool load (File myfile);



private:
  bool decodeWaveFile (File file);
  void performFFT(double *real, double *imag);
  double* fftToLED();

  int getIndexOfString (File file, String stringToSearchFor);

  int16_t twoBytesToInt (File file, int startIndex);
  int16_t twoBytesToIntCon(File file);

  int32_t fourBytesToInt (File file, int startIndex);
  void clearAudioBuffer();
  double singleByteToSample (uint8_t sample);
  double sixteenBitIntToSample (int16_t sample);

  /* Variables */
  uint32_t sampleRate;
  int bitDepth;
  uint32_t numSamples;
  uint32_t samplesStartIndex;
  File file;
  double* fft;
  String filename;
  
};



#endif /* AudioFile_h */
