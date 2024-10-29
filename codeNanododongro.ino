#include <LiquidCrystal_I2C.h>

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <DHT.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(6, 7);
// Địa chỉ I2C của module ADS1115
//#define ADS1115_ADDRESS 0x48

// Khởi tạo đối tượng ADS1115
const int batteryPin = A1;
const int DHT_pin = 3;
const int DHT_type = DHT11;
DHT dht(DHT_pin, DHT_type);
int alert_state = 0;
float currentRMS = 0;
int test = 1;
Adafruit_ADS1115 ads;
const float FACTOR = 4 / 3;  //20A/1V from teh CT
const float multiplier = 0.0000078125;
float scale;
int arr[90];
byte degree[8] = {
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};
byte arrow[8] = {
  0B00000,
  0B00000,
  0B00010,
  0B11111,
  0B11111,
  0B00010,
  0B00000,
  0B00000
};

float h, t, current, voltage, V_battery;
int16_t adcValue;
float adc = 0;
void setup(void) {
  lcd.init();
  lcd.backlight();
  ads.setGain(GAIN_SIXTEEN);  // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
    // Khởi tạo module ADS1115
  ads.begin();
  dht.begin();
  lcd.createChar(1, degree);
  lcd.createChar(2, arrow);
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("RCD Current Measurement with ETCR025K, INA128, and ADS1115");
  lcd.setCursor(1, 0);
  lcd.print("Dang khoi dong");
  delay(1000);
  lcd.setCursor(0, 1);
  for(int i=0 ;i<16;i++){
    lcd.write(2);
    delay(300);
  }
  
    pinMode(batteryPin, INPUT);
}

void loop(void) {
 
  int count=0;
  int sum = 0 ;
for (int i=0;i<100;i++){
  sum += analogRead(batteryPin);
  count++;
}

  V_battery = 0.0442635877 * sum/count;  //((R1 + R2) / (R2 * 1023)) * analogRead(battery) //0.0148356




  currentRMS = getcurrent() * 1000;

  h = dht.readHumidity();
  t = dht.readTemperature();

  Serial.print("Hum (%): ");
  Serial.print(h);
  Serial.print("\t Temp(°C): ");
  Serial.print(t);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DongRo: ");
  lcd.print(currentRMS, 2);
  lcd.print("mA");
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(round(h));
  lcd.print("%");
  lcd.setCursor(8, 1);
  lcd.print("T: ");
  lcd.print(round(t));
  lcd.write(1);
  lcd.print("C");


if (0 <= currentRMS && currentRMS <= 2) {
    alert_state = 1;
    Serial.println("Trang thai: Binh Thuong");
  } else if (2 < currentRMS &&currentRMS <= 5) {
    alert_state = 2;
    Serial.println("Trang thai: Tien Canh Bao");
  } else if (5 < currentRMS && currentRMS <= 10) {
    alert_state = 3;
    Serial.println("Trang thai: Canh bao");
  }else  if (10 < currentRMS && currentRMS <= 15) {
    alert_state = 4;
    Serial.println("Trang thai: Tien Nguy Hiem");
  } else if (15 < currentRMS && currentRMS <= 100) {
    alert_state = 5;
    Serial.println("Trang thai: Nguy Hiem");
  } else {
    alert_state = 6;
    Serial.println("Trang thai: Cuc Nguy Hiem");
  }

  printMeasure("\t Irms: ", currentRMS, "mA");
  delay(20);
  updatedulieu();
}
void updatedulieu() {


  String data[6] = { String(h), String(t), String(test), String(currentRMS, 2) , String(V_battery, 2),String(alert_state) };

  //  String data[11]; // Mảng chứa 11 dữ liệu kiểu string
  //   data[0] = "Data1";
  //   data[1] = "Data2";
  //   // ... Các phần tử tiếp theo

  String combinedData = "";  // Khởi tạo chuỗi kết quả

  // Kết hợp các dữ liệu vào chuỗi
  for (int i = 0; i < 6; i++) {
    combinedData += data[i];
    if (i < 6) {
      combinedData += ",";  // Thêm dấu phẩy nếu không phải là phần tử cuối cùng
    }
  }
  combinedData += "\n";  // Thêm ký tự xuống dòng

  // Gửi chuỗi dữ liệu qua UART
  mySerial.print(combinedData);
  Serial.println(combinedData);
  delay(1000);
}

void printMeasure(String prefix, float value, String postfix) {
  Serial.print(prefix);
  Serial.print(value, 6);
  Serial.println(postfix);
}

float getcurrent() {
  float voltage;
  float current;
  float sum = 0;
  long time_check = millis();
  int counter = 0;

  while (millis() - time_check < 500) {

    int maxVL = 0;
    for (int i = 0; i < 90; i++) {


      float currentValue = ads.readADC_Differential_0_1();
      arr[i] = currentValue;
    }

    int temp;
    for (int k = 0; k < 90 - 1; k++) {
      for (int j = k + 1; j < 90; j++) {
        if (arr[j] > arr[k]) {
          // Hoán đổi hai phần tử nếu thứ tự không đúng
          temp = arr[j];
          arr[j] = arr[k];
          arr[k] = temp;
        }
      }
    }
    // for (int l =0 ;l<90 ; l++)
    // {
    //   Serial.print(l) ;
    //   Serial.print(" ") ;
    //   Serial.println(arr[l]) ;
    // }

    if (arr[6] < 500) {
      scale = 1;
    } else if (arr[6] > 500 && arr[6] <= 1000) {
      scale = 1.1;
    } else if (arr[6] > 1000 && arr[6] <= 2000) {
      scale = 1.18;
    } else if (arr[6] > 2000 && arr[6] <= 3000) {
      scale = 1.3;
    } else if (arr[6] > 3000 && arr[6] <= 4000) {
      scale = 1.33;
    } else if (arr[6] > 4000 && arr[6] <= 5000) {
      scale = 1.35;
    } else if (arr[6] > 5000 && arr[6] <= 7000) {
      scale = 1.45;
    } else {
      scale = 1.5;
    }


  
  maxVL = arr[6] * scale;
    Serial.print("  ");

    Serial.print(arr[6]);
    //current = maxVL*100000/32768;

    voltage = maxVL * multiplier;
    current = voltage * FACTOR;
    //current /= 1000.0;

    sum += sq(current);
    counter = counter + 1;
  }

  current = sqrt(sum / counter);
  return (current);
}
