#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

// Definisi Pin sesuai jalur kabel di gambar Wokwi
#define DHTPIN 15
#define DHTTYPE DHT22
#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 12
#define SERVO_PIN 13

// Inisialisasi Objek Komponen
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
Servo kranServo;

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Setup Servo menggunakan library ESP32Servo
  kranServo.attach(SERVO_PIN);
  kranServo.write(0); // Posisi awal tertutup

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Plant IoT");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
  lcd.clear();
}

float ukurJarakTangki() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0344 / 2;
  return distance;
}

void loop() {
  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();
  float jarakTangki = ukurJarakTangki();

  if (isnan(suhu) || isnan(hum)) {
    Serial.println("Gagal membaca sensor DHT22!");
    return;
  }

  Serial.printf("Suhu: %.1f C | Hum: %.1f %% | Jarak Tangki: %.1f cm\n", suhu, hum, jarakTangki);

  // Logika 1: Peringatan jika tangki air habis (jarak > 20 cm)
  if (jarakTangki > 20.0) {
    lcd.setCursor(0, 0);
    lcd.print("Status: BAHAYA! ");
    lcd.setCursor(0, 1);
    lcd.print("Tangki Air Habis");
    
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
    return;
  } else {
    noTone(BUZZER_PIN);
  }

  // Logika 2: Penyiraman otomatis jika kelembapan < 50%
  if (hum < 50.0) {
    lcd.setCursor(0, 0);
    lcd.print("Menyiram Tanaman");
    lcd.setCursor(0, 1);
    lcd.print("Kran: TERBUKA   ");
    
    kranServo.write(90); // Buka kran
    delay(3000);       // Siram selama 3 detik
    kranServo.write(0);  // Tutup kran
    lcd.clear();
  }

  // Tampilan Monitoring Normal di LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(suhu, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0, 1);
  lcd.print("Hum : ");
  lcd.print(hum, 1);
  lcd.print("%    ");

  delay(2000);
}
