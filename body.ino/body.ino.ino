#include "status.h"
#define blueToothSerial Serial2

/* Initialize control function */
Status control = Status();

/* This function is used to read incoming bytes, for parseInt() will cause unexpected delay*/
int readData(){
  int recvInt = -1;
  bool is_first = true; // if we are reading the first character
  while(blueToothSerial.available() > 0){
    byte recvChar = blueToothSerial.read();
    if(is_first){ // if this is the first character, transform it into int
      recvInt = recvChar - '0';
      is_first = false;
      continue;
    }
  }

  if(recvInt > 0) // if the program has read something, return the number otherwise return -1
    return recvInt;
  return -1;
}

void setup() {
  Serial.begin(9600);
  setupBlueToothConnection();
  // Initialize SD card function
  control.sdIni();
  // Print command list to user
  control.printCommand(); //print commands list to user
  delay(2000);
}

void loop() {
  int last_command = -1;
  int recvInt = -1;
  int start = 0;
  int songNum = 0;
  bool circle = false;
  bool is_song_playing = false;


  while(1){
    recvInt = readData();
    // if there's new incoming command
    if(recvInt != -1)
      blueToothSerial.println(recvInt);

    // if we have finished executing last command and there's new command
    if(last_command == -1 && recvInt != -1){
      // either "choose a song to play" or "load song" mode
      if(recvInt == 1 || recvInt == 6){
        control.printFileList(); // print song list to user
        last_command = recvInt;
        start = 0;
      }
      // next song
      else if(recvInt == 2){
        songNum++;
        if(control.playSong(songNum)){
          blueToothSerial.println("start playing!");
          is_song_playing = true;
          last_command = -1; // we have finished executing this command
          start = 0;
        }
      }
      // "circle" mode
      else if(recvInt == 3){
        circle = !circle;
      }
      // "pause" or "resume" mode
      else if(recvInt == 4){
        is_song_playing = !is_song_playing;
        if(!is_song_playing)
          control.printLed("pause");
        else
          control.printLed("play");
      }
      // "reset" mode -> break the while loop -> reset all parameters
      else if(recvInt == 5){
          control.printCommand(); //print commands list to user
          control.printLed("reset");
          break;
      }
      else if(recvInt == 7){
          control.printCommand(); //print commands list to user
      }
      recvInt = -1;
    }


    // command 1 & command 6 requires another number to be inputted by user
    if(last_command == 1){
      // read again until there's input from user
      do{
        delay(50);
        recvInt = readData();
      }while(recvInt == -1);

      songNum = recvInt - 1;
      recvInt = -1;
      if(control.playSong(songNum)){
        blueToothSerial.println("start playing!");
        is_song_playing = true;
        last_command = -1;
      }
    }else if(last_command == 6){
      control.loadSong(songNum);
    }

    // display data is divied n sets, and each of it contains 16 doubles
    // Every loop will perform 1 set
    if(is_song_playing){
      if(circle){ control.ledCircle(start);}
      else {control.ledDisplay(start);}
      start++;
      Serial.println(start);
    }



   }// end of while loop



}


void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=Xin\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STPIN=2333\r\n");
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The Xin bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}
