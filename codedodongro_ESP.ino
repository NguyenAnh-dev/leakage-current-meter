/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6f6v1G55T"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "qWEAftoOcGrRrjjHp7b_laWXeKQ0AcZ-"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
unsigned long lasttime = 0;
#include <FirebaseESP8266.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager

WiFiManager wifiManager;
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

SoftwareSerial mySerial(D6, D5);


int32_t lastSendData = 0;
String data[6];
float alert_state = 0;
// khai bao casc tham số truy cập tới firebase database
String canhbao;
#define FIREBASE_HOST "https://thietbigiamsat-2023-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "ID5SwuwloVA8AnWOrKFUlxu3rnQFHJEp9ZilEqSV"

FirebaseData firebaseData;
String fireStatus = "";  // bien chua trang thai cuar den led
int led = 2;

float h, t, current, test, V_battery;

String path = "MAY2/";  // duog dan toi truong led trong cau truc du lieu tren firebase

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";

BlynkTimer timer;




// This function is called every time the Virtual Pin 0 state changes


// This function is called every time the device is connected to the Blynk.Cloud


// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent() {
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V3, millis() / 1000);
}

void setup() {
  // Debug console
  mySerial.begin(9600);
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  wifiManager.autoConnect("AutoConnectAP_May2");
  Serial.println("connected...yeey :)");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString(firebaseData, path + "DATARECEIVED", "OFF");  // gui trang thai khoi tao cua led toi firebase
}

void loop() {
  Blynk.run();
  timer.run();
  dataReceived();
  if (millis() - lastSendData > 5000) {
    update_firebase();
    Serial.println("upload done ");
    lastSendData = millis();
  }
}


void dataReceived() {
  if (mySerial.available()) {                              // Kiểm tra xem có dữ liệu đang được gửi đến không
    String receivedData = mySerial.readStringUntil('\n');  // Đọc chuỗi dữ liệu cho đến khi gặp ký tự '\n'

    // Tách dữ liệu bằng dấu phẩy

    int startIndex = 0;
    int commaIndex;

    for (int i = 0; i < 6; i++) {
      commaIndex = receivedData.indexOf(',', startIndex);
      if (commaIndex != -1) {
        data[i] = receivedData.substring(startIndex, commaIndex);
        startIndex = commaIndex + 1;
      } else {
        data[i] = receivedData.substring(startIndex);
        break;
      }
    }

    // Xử lý dữ liệu nhận được
    for (int i = 0; i < 6; i++) {
      Serial.print("Data " + String(i + 1) + ": ");
      Serial.println(data[i]);
    }
  }


  // Serial.println();
  h = data[0].toFloat();
  t = data[1].toFloat();
  test = data[2].toFloat();
  current = data[3].toFloat();
  V_battery = data[4].toFloat();
 alert_state = data[5].toFloat();

if(alert_state==1) {
canhbao = "1-Bình Thường";
} else if(alert_state==2) {
canhbao = "2-Tiền Cảnh Báo";
}else if(alert_state==3) {
  canhbao = "3-Cảnh Báo";
}

else if(alert_state==4) {
  canhbao = "4-Tiền Nguy Hiểm";
}
else if (alert_state==5){
  canhbao = "5-Nguy Hiểm";
}
else if (alert_state==6){
  canhbao = "6-Cực Nguy Hiểm";
}





}
void update_firebase() {
  if ((unsigned long)(millis() - lasttime) > 5000) {
    // You can inject your own code or combine it with other sketches.
    // Check other examples on how to communicate with Blynk. Remember
    // to avoid delay() function!
    // put your main code here, to run repeatedly:
    Firebase.getString(firebaseData, path + "DATARECEIVED");  // lay trang thai tu co so du lieu

    fireStatus = firebaseData.stringData();
    if (fireStatus == "ON") {

    } else if (fireStatus == "OFF") {
    } else {
    }
    //float temperature = 0.00002; //random(20, 30); // Giả sử giá trị nhiệt độ là ngẫu nhiên từ 20 đến 30

    // Đẩy giá trị nhiệt độ lên Firebase
    Firebase.setInt(firebaseData, path + "CURRENT", current);
    Firebase.setInt(firebaseData, path + "TEMP", t);
    Firebase.setInt(firebaseData, path + "HUM", h);
    Firebase.setInt(firebaseData, path + "VOLT", V_battery);
    Firebase.setString(firebaseData, path + "ALERT", canhbao);
    lasttime = millis();
    //Serial.println(val);
    Blynk.virtualWrite(V8, t);
    Blynk.virtualWrite(V9, h);
    Blynk.virtualWrite(V10, current);
   
  }
}



