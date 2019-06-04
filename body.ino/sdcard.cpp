#include "sdcard.h"

/* Public Functions */
sdcard::sdcard(void){
  counter = 0;
}

sdcard::~sdcard(void){
}

bool sdcard::sdIni(void){
  // SD module initialization
  if(!SD.begin(CS)){
    return false;
  }
  File root = SD.open("/");
  // get the file list
  retrFiles(root);
  return true;
}

void sdcard::printFileList(void){
  for(int i = 1; i < counter+1; i++){
    blueToothSerial.print("\r\n");
    blueToothSerial.print(i, HEX);
    blueToothSerial.print(". ");
    blueToothSerial.print(*(file_list+i-1));
    blueToothSerial.print("\r\n");
  }
}

int sdcard::fileNum(void){ return counter; }

String sdcard::getSongName(int num){ return *(file_list + num); }


/* Private Functions */
// recursive function
void sdcard::retrFiles(File dir){
  counter = 0;
  while (true) {
    File entry =  dir.openNextFile();
    // no more files
    if (!entry)
      break;

    if (entry.isDirectory()) {
      retrFiles(entry);
    } else {
      String temp = String(entry.name());
      // only get the WAV file
      if(temp.endsWith(String("WAV"))){
        file_list[counter] = entry.name();
        counter++;
      }
    }
    entry.close();
  }
}
