#include "Status.h"
SoftDMD dmd(1,1);
DMD_TextBox box(dmd, 0, 2);

// Constructor
Status::Status(void)
{
  song = currentSong();
  card = sdcard();
  delay(3000);
}

Status::~Status(void){

}


/* Public Functions*/
void Status::sdIni(void){
  bool is_first_attemp = true;

  // check if SD card successfully initialized
  while(!card.sdIni()){
    // just send false message once to user
    if(is_first_attemp){
      blueToothSerial.print("\r\nSD card initialization failed\r\n");
      blueToothSerial.print("\r\nCheck your connection\r\n");
    }
    is_first_attemp = false;
  }
    blueToothSerial.print("\r\nSD card initialization successed");
}

// print a string in the LED matrix
void Status::printLed(String str){
  dmd.clearScreen();
  box.print(str);
  delay(500);
}

// print command list
void Status::printCommand(void){
  blueToothSerial.print("\r\nPlease choose a command in following list by input its number\r\n");
  for (int i = 1; i < 8; i++){
    blueToothSerial.print("\r\n");
    blueToothSerial.print(i, HEX);
    blueToothSerial.print(". ");
    blueToothSerial.print(*(command_list+i - 1));
    blueToothSerial.print("\r\n");
  }

}

void Status::printFileList(){
  blueToothSerial.print("\r\n Please choose a song by input its number from the following list");
  card.printFileList();
}

bool Status::playSong(int recvNum){
  dmd.setBrightness(355);
  dmd.selectFont(Arial14);
  dmd.begin();
  /* because we have a "play next song" command, and in the "body.ino" we just simply increased
     the recvNum, hence it will requires the program to get the remainder in here */
  recvNum = recvNum % (card.fileNum());
  String filename = card.getSongName(recvNum);
  // find the corresponding txt file
  filename.replace("WAV", "txt");
  Serial.println(filename);
  File file = SD.open(filename, FILE_READ);
  // check if the file exists
  if (!file)
  {
      blueToothSerial.print("\r\nERROR: File doesn't exist, please load the song first");
      blueToothSerial.print(file);
      blueToothSerial.print("\r\n");
      return false;
  }
  // find the size of the file
  fftSize = file.size();
  int temp[5600];

  // read the data
  for(int i = 0; i < 5600; i++){
    temp[i] = file.parseInt();
  }

  // pass the data to the global variable
  data = temp;
  file.close();
  blueToothSerial.println(" loaded");

  // print the filename
  for(int i = 0; i < filename.length()-4; i++){
    box.print(filename.charAt(i));
    delay(200);
  }
  delay(500);
  dmd.clearScreen();
  box.print("begin");
  delay(1000);
  return true;
}

void Status::ledDisplay(int start){
  /*Assigning an LED height to each data value.*/
  int freq_array[50] = {0};
  for(int i = start*Xres; i < Xres*(start+1); i++){
    int index = i % Xres;
    if (data[i] < 2000 && data[i] > 300){freq_array[index] = 16;}
    else if(data[i] <= 300 && data[i] > 200){freq_array[index] = 15;}
    else if (data[i] <= 200 && data[i] > 180){freq_array[index] = 14;}
    else if (data[i] <= 180 && data[i] > 160){freq_array[index] = 13;}
    else if (data[i] <= 160 && data[i] > 140){freq_array[index] = 12;}
    else if (data[i] <= 140 && data[i] > 120){freq_array[index] = 11;}
    else if (data[i] <= 120 && data[i] > 100){freq_array[index] = 10;}
    else if (data[i] <= 100 && data[i] > 80){freq_array[index] = 9;}
    else if (data[i] <= 80 && data[i] > 60){freq_array[index] = 8;}
    else if (data[i] <= 60 && data[i] > 50){freq_array[index] = 7;}
    else if (data[i] <= 50 && data[i] > 40){freq_array[index] = 6;}
    else if (data[i] <= 40 && data[i] > 30){freq_array[index] = 5;}
    else if (data[i] <= 30 && data[i] > 20){freq_array[index] = 4;}
    else if (data[i] <= 20 && data[i] > 15){freq_array[index] = 3;}
    else if (data[i] <= 15 && data[i] > 10){freq_array[index] = 2;}
    else if (data[i] <= 10 && data[i] > 3){freq_array[index] = 1;}
    else if (data[i] <= 3 && data[i] > 0){freq_array[index] = 0;}

  }
  dmd.clearScreen();
  for(int i = 0; i < Xres; i++){
    dmd.drawLine(0+i, Yres, 0+i, (Yres - freq_array[i]));
  }

  delay(500);
}

void Status::ledCircle(int start){
  int vmax = -1;
  for(int i = start*Xres; i < Xres*(start+1); i++){
    int temp = 0;
    if (data[i] < 2000 && data[i] > 140){temp = 4;}
    else if (data[i] <= 140 && data[i] > 80){temp = 3;}
    else if (data[i] <= 80 && data[i] > 20){temp = 2;}
    else if (data[i] <= 20 && data[i] > 0){temp = 1;}
    // find the maximum value
    vmax = vmax>temp?vmax:temp;
  }
  dmd.clearScreen();
  dmd.drawCircle(16, 8, 2*vmax-1);
  delay(250);
  dmd.drawCircle(16, 8, 2*vmax);
  delay(250);
}

bool Status::loadSong(int recvNum){
  if(recvNum > card.fileNum()){
    blueToothSerial.println("Out of range");
    return false;
  }
  String song_name = String(card.getSongName(recvNum));

  File file = SD.open(song_name, FILE_READ);
  // check if the file exists
  if (!file)
  {
      blueToothSerial.print("\r\nERROR: File doesn't exist or otherwise can't load file ");
      blueToothSerial.print(file);
      blueToothSerial.print("\r\n");
      return false;
  }
  // if music has been loaded successfully
  if(music.load(file)){
    blueToothSerial.print("\r\n");
    blueToothSerial.print(song_name);
    blueToothSerial.print(" loaded!\r\n");
    delay(50);
  }
    blueToothSerial.print("\r\nStart reading...give me some minutes..\r\n");
    delay(50);
  return true;
}
