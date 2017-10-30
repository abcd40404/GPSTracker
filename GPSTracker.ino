#include <SoftwareSerial.h>
SoftwareSerial GPSModule(2, 3); // RX, TX
SoftwareSerial sSerial(10,11); //(RX,TX)

int updates;
int failedUpdates;
int pos;
int stringplace = 0;

boolean ServerConnected;

String timeUp;
String nmea[15];
String labels[12] {"Time: ", "Status: ", "Latitude: ", "Hemisphere: ", "Longitude: ", "Hemisphere: ", "Speed: ", "Track Angle: ", "Date: "};
String gpsString="";

void setup() {
  Serial.println("SoftSerial to ESP8266 AT commands test ...");
  ServerConnected = false;
  sSerial.begin(9600);  //軟體序列埠速率 (與硬體同步調整)
  Serial.begin(9600);    //硬體序列埠速率 (與軟體同步調整)
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  connectWifi();
  connectServer();
  GPSModule.begin(9600);
}

void loop() {
//  Serial.flush();
//  GPSModule.flush();
  sSerial.listen();
  while (sSerial.available()) {   //若軟體序列埠 Rx 收到資料 (來自 ESP8266)
    char c = sSerial.read();
    Serial.write(c);  //讀取後寫入硬體序列埠 Tx (PC)
  }
  while (Serial.available()) {  //若硬體序列埠 Rx 收到資料 (來自 PC)
//      sSerial.write(Serial.read());  //讀取後寫入軟體序列埠 Tx (ESP8266)
    char c = Serial.read();
//      Serial.println(c);
    sSerial.write(c);
  } 
  
  delay(5000);
  if(ServerConnected){
//    String msg = "abc";
//    int len = msg.length();
//    String cmd = "AT+CIPSEND=" + String(len+2);
//    Serial.println(cmd);
//    sSerial.println(cmd);
//    delay(2000);
//    Serial.println(msg);
//    sSerial.println(msg);
    GPSModule.listen();
    while (GPSModule.available() > 0)
    {
      GPSModule.read();
    }
    if (GPSModule.find("$GPRMC,")) {
      String tempMsg = GPSModule.readStringUntil('\n');
      for (int i = 0; i < tempMsg.length(); i++) {
        if (tempMsg.substring(i, i + 1) == ",") {
          nmea[pos] = tempMsg.substring(stringplace, i);
          stringplace = i + 1;
          pos++;
        }
        if (i == tempMsg.length() - 1) {
          nmea[pos] = tempMsg.substring(stringplace, i);
        }
      }
      updates++;
      nmea[2] = ConvertLat();
      nmea[4] = ConvertLng();
      String msg = nmea[2] + "," + nmea[4];
      int len = msg.length();
      String cmd = "AT+CIPSEND=" + String(len+2);
      Serial.println(cmd);
      sSerial.println(cmd);
      delay(2000);
      Serial.println(msg);
      sSerial.println(msg);
  //    for (int i = 0; i < 9; i++) {
  //      Serial.print(labels[i]);
  //      Serial.print(nmea[i]);
  //      Serial.println("");
  //    }
  
    }
    else {
      failedUpdates++;
    }
    stringplace = 0;
    pos = 0;
  }
}
void connectWifi(){
  boolean ok = false;
  while(1){
    sSerial.println("AT+CWJAP=\"HI\",\"sky61003\"");
    while(sSerial.available()){
      if(sSerial.find('OK'))
        ok = true;
    }
    delay(1000);
    if(ok)
      break;
  }
  if(ok)
    Serial.println("Wifi OK");
  else
    Serial.println("Error");
}

void connectServer(){
  boolean ok = false;
  while(1){
    sSerial.println("AT+CIPSTART=\"TCP\",\"172.104.77.168\",10000");
    while(sSerial.available()){
      if(sSerial.find("OK"))
        ok = true;
    }
    delay(1000);
    if(ok)
      break;
  }
  if(ok){
    Serial.println("Server OK");
    ServerConnected = true;
  }
  else
    Serial.println("Error");
}

String get_ESP8266_response() {  //取得 ESP8266 的回應字串
  String str="";  //儲存接收到的回應字串
  char c;  //儲存接收到的回應字元
  while (sSerial.available()) {  //若軟體序列埠接收緩衝器還有資料
    c=sSerial.read();  //必須放入宣告為 char 之變數 (才會轉成字元)
    str.concat(c);  //串接回應字元 
    delay(10);  //務必要延遲, 否則太快
    }
  str.trim();  //去除頭尾空白字元
  return str;
} 

String ConvertLat() {
  String posneg = "";
  if (nmea[3] == "S") {
    posneg = "-";
  }
  String latfirst;
  float latsecond;
  for (int i = 0; i < nmea[2].length(); i++) {
    if (nmea[2].substring(i, i + 1) == ".") {
      latfirst = nmea[2].substring(0, i - 2);
      latsecond = nmea[2].substring(i - 2).toFloat();
    }
  }
  latsecond = latsecond / 60;
  String CalcLat = "";

  char charVal[9];
  dtostrf(latsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLat += charVal[i];
  }
  latfirst += CalcLat.substring(1);
  latfirst = posneg += latfirst;
  return latfirst;
}

String ConvertLng() {
  String posneg = "";
  if (nmea[5] == "W") {
    posneg = "-";
  }

  String lngfirst;
  float lngsecond;
  for (int i = 0; i < nmea[4].length(); i++) {
    if (nmea[4].substring(i, i + 1) == ".") {
      lngfirst = nmea[4].substring(0, i - 2);
      //Serial.println(lngfirst);
      lngsecond = nmea[4].substring(i - 2).toFloat();
      //Serial.println(lngsecond);

    }
  }
  lngsecond = lngsecond / 60;
  String CalcLng = "";
  char charVal[9];
  dtostrf(lngsecond, 4, 6, charVal);
  for (int i = 0; i < sizeof(charVal); i++)
  {
    CalcLng += charVal[i];
  }
  lngfirst += CalcLng.substring(1);
  lngfirst = posneg += lngfirst;
  return lngfirst;
}
