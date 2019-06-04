#include "ArduinoAudioFile.h"



/* Public functions */
ArduinoAudioFile::ArduinoAudioFile(){

}

ArduinoAudioFile::~ArduinoAudioFile(){

}

bool ArduinoAudioFile::load (File myfile)
{
    file = myfile;
    return decodeWaveFile(file);
}

//=============================================================


/* =============== Private functions ===============*/

bool ArduinoAudioFile::decodeWaveFile (File file)
{

  // -----------------------------------------------------------
  // try and find the start points of key chunks
  int indexOfDataChunk = getIndexOfString (file, String("dat"));
  int indexOfFormatChunk = getIndexOfString (file, String("fmt"));

  // if we can't find the data or format chunks, or the IDs/formats don't seem to be as expected
  // then it is unlikely we'll able to read this file, so abort
  if (indexOfDataChunk == -1 || indexOfFormatChunk == -1)
  {
    blueToothSerial.print("\r\nERROR: this doesn't seem to be a valid .WAV file\r\n");
    return false;
  }

    // -----------------------------------------------------------
    // FORMAT CHUNK
    int f = indexOfFormatChunk;
    int16_t audioFormat = twoBytesToInt (file, f + 8);
    int16_t numChannels = twoBytesToInt (file, f + 10);
    sampleRate = (uint32_t) fourBytesToInt (file, f + 12);
    int32_t numBytesPerSecond = fourBytesToInt (file, f + 16);
    int16_t numBytesPerBlock = twoBytesToInt (file, f + 20);
    bitDepth = (int) twoBytesToInt (file, f + 22);

    int numBytesPerSample = bitDepth / 8;

    // check that the audio format is PCM
    if (audioFormat != 1)
    {
        blueToothSerial.print("ERROR: this is a compressed .WAV file and this library does not support decoding them at present");
        return false;
    }

    // check the number of channels is mono or stereo
    if (numChannels < 1 ||numChannels > 2)
    {
        blueToothSerial.print("ERROR: this WAV file seems to be neither mono nor stereo (perhaps multi-track, or corrupted?)");
        return false;
    }

    // check header data is consistent
    if ((numBytesPerSecond != (numChannels * sampleRate * bitDepth) / 8) || (numBytesPerBlock != (numChannels * numBytesPerSample)))
    {
        blueToothSerial.print("ERROR: the header data in this WAV file seems to be inconsistent");
        return false;
    }

    // check bit depth is either 8, 16 or 24 bit
    if (bitDepth != 8 && bitDepth != 16 && bitDepth != 24)
    {
        blueToothSerial.print("ERROR: this file has a bit depth that is not 8, 16 or 24 bits");
        return false;
    }

    // -----------------------------------------------------------
    // DATA CHUNK
    int d = indexOfDataChunk;
    int32_t dataChunkSize = fourBytesToInt (file, d + 4);

    numSamples = dataChunkSize / (numChannels * bitDepth / 8);
    samplesStartIndex = indexOfDataChunk + 8;

    double real[numSamples];
    double imag[numSamples];

    for (int i = 0; i < sampleRate; i++){
      int16_t sampleAsInt;
      if(i == 0){
        // first byte, so need to locate it in the file
        sampleAsInt = twoBytesToInt (file, samplesStartIndex);
      }else{
        // otherwise just read the byte continuously
        sampleAsInt = twoBytesToIntCon (file);
      }
      // two channels
      double sample = sixteenBitIntToSample (sampleAsInt);
      real[i] = sample;
      imag[i] = 0;
      twoBytesToIntCon(file);
    }
    file.close();
    performFFT(real, imag);
}


//template <class double>
void ArduinoAudioFile::performFFT(double *real, double *imag){
  // arduinoFFT requires samples number to be the power of 2
  int power = log(numSamples) / log(2);
  const uint16_t samples = pow(2, power);

  arduinoFFT FFT = arduinoFFT(real, imag, samples, (double)sampleRate);

  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data*/
  FFT.Compute(FFT_FORWARD); /* Do FFT */
  FFT.ComplexToMagnitude(); /* Get magnitude of the data */

  fftToLED();
}

double* ArduinoAudioFile::fftToLED(){
  // find the corresponding txt file
  String filename = String(file.name());
  filename.replace("WAV", "txt");
  File myFile = SD.open(filename);

  /* numSamples: Total number of samples
     Sample rate: how many samples in one second
     So numSamples / sampleRate = the length of the audio file in second */
  for(int num = 0 ; num < (int)numSamples / sampleRate; i++){
    /* e.g. we have 96 samples in second -> sampleRate is 96 samples / second
            32 columns in LED matrix
            so every 3 samples are add toghter and get the main value -> 32 numbers then
            each number corresponding to each column
    */
    for(int j = 0; j < XRES; j++){
      double temp = 0;
      for(int i = 0; i < sampleRate / XRES; i++){
        temp += fft[num*sampleRate + i];
      }
      temp = temp / (sampleRate / XRES);
      myFile.println((int)temp); // write the data to the file
    }
  }

}

//=============================================================
//template <class double>
int ArduinoAudioFile::getIndexOfString (File file, String stringToSearchFor)
{
  int index = -1;
  int stringLength = (int)stringToSearchFor.length();
  char section[stringLength];

  for (int i = 0; i < file.size() - stringLength; i++){
    file.seek(i);
    file.read(section, stringLength);

    if (String(section).equals(stringToSearchFor))
      {
          index = i;
          break;
      }
  }

  return index;
}

//=============================================================
//template <class double>
int16_t ArduinoAudioFile::twoBytesToInt (File file, int startIndex)
{
  file.seek(startIndex);
  char temp[2];
  file.read(temp, 2);
  int16_t result = (temp[1] << 8) | temp[0];
  return result;
}

//=============================================================
//template <class double>
int16_t ArduinoAudioFile::twoBytesToIntCon(File file){

  int16_t result;
  short temp[2];
  int a = file.read(temp, sizeof(temp));
  result = (temp[1] << 8) | temp[0];
  return result;
}


//=============================================================
//template <class double>
int32_t ArduinoAudioFile::fourBytesToInt (File file, int startIndex)
{
    int32_t result;
    file.seek(startIndex);
    char * temp = new char[4];
    file.read(temp, 4);
    result = (temp[3] << 24) | (temp[2] << 16) | (temp[1] << 8) | temp[0];
    delete(temp);
    return result;
}

//=============================================================
// TODO
//template <class double>
void ArduinoAudioFile::clearAudioBuffer()
{
  // delete(samples);
}

//=============================================================
//template <class double>
double ArduinoAudioFile::singleByteToSample (uint8_t sample)
{
    return double((sample - 128) / (128));
}

//=============================================================
//template <class double>
double ArduinoAudioFile::sixteenBitIntToSample (int16_t sample)
{
    return double( (sample) / (32768));
}
