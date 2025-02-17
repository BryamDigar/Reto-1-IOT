#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MQUnifiedsensor.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define LED_PIN 27
#define BUZZER_PIN 26
#define FLAME_PIN 25
#define Board "ESP-32"
#define Pin 34  // Usamos un pin ADC adecuado para el ESP32 v1

/***********************Software Related Macros************************************/
#define Type "MQ-2"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12 
#define RatioMQ2CleanAir 9.83

#define RED_PIN 5
#define GREEN_PIN 18
#define BLUE_PIN 19

#define TEMP_LOW 8.4
#define TEMP_HIGH 13
#define HUMI_LOW 75
#define HUMI_HIGH 80
#define FIRE_THRESHOLD LOW

#define I2C_ADDR 0x27
#define LCD_COLUMNS 16
#define LCD_LINES 2

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
int valor_gas = 0; 

void setup() {
  lcd.clear();
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  MQ2.setRegressionMethod(1);
  MQ2.setA(36974); 
  MQ2.setB(-3.109);
  MQ2.init(); 

  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("----*----");
  lcd.setCursor(2, 1);
  lcd.print("Alarm System");
  delay(1000);
  lcd.clear();

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i <= 10; i++) {
    MQ2.update();
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  Serial.println(calcR0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQ2 = ");
  lcd.print(calcR0);
  delay(500);
}

void loop() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  Serial.println("Temperatura:" + String(temp));
  lcd.print("'C");
  lcd.setCursor(0, 1);
  lcd.print("Humi: ");
  lcd.print(humi);
  Serial.println("humedad" + String(humi));
  lcd.print(" %");
  delay(1000);

  MQ2.update();
  MQ2.readSensor();
  valor_gas = analogRead(Pin);
  valor_gas = map(valor_gas, 0, 4095, 0, 100);
  Serial.println(valor_gas);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MQ2 = ");
  lcd.print(valor_gas);
  delay(1000);
  lcd.clear();

  if (digitalRead(FLAME_PIN) == LOW) {  // El sensor activa LOW en presencia de fuego
    digitalWrite(LED_PIN, HIGH);  // Encender LED de alerta
    tone(BUZZER_PIN, 1000);       // Activar buzzer
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("FUEGO");
    lcd.setCursor(0, 1);
    lcd.print("DETECTADO!!");
    Serial.println("** ¡Fuego detectado! **");
    delay(1000);
    lcd.clear();
  } else {
    digitalWrite(LED_PIN, LOW);  // Apagar LED
    noTone(BUZZER_PIN);          // Apagar buzzer
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NO HAY");
    lcd.setCursor(0, 1);
    lcd.print("FUEGO");
    Serial.println("No se detecta fuego");
    delay(1000);
    lcd.clear();
  }
  delay(100);
  if(temp > TEMP_LOW && temp < TEMP_HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp ok");
    lcd.setCursor(0, 1);
    lcd.print("Sin alertas");
    delay(1000);
    setRGB(0, 255, 0);
    lcd.clear();
  } else if(temp > TEMP_HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp alta");
    lcd.setCursor(0, 1);
    lcd.print("Precaucion");
    delay(1000);
    setRGB(255, 0, 0);
    lcd.clear();
    if (humi < HUMI_LOW) {
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Humedad baja");
      lcd.setCursor(0, 1);
      lcd.print("Fuego detectado!");
      delay(1000);
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
      lcd.clear();
    }
  } else if(temp < TEMP_LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperatura baja");
    lcd.setCursor(0, 1);
    lcd.print("Cuidado frio");
    delay(1000);
    setRGB(0, 0, 255);
    lcd.clear();
  }
  if (humi < HUMI_LOW) {
    digitalWrite(LED_PIN, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humedad baja");
    lcd.setCursor(0, 1);
    lcd.print("Posible incendio");
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  }
}

void setRGB(int red, int green, int blue) {
  ledcWrite(0, 255 - red);
  ledcWrite(1, 255 - green);
  ledcWrite(2, 255 - blue);
}

