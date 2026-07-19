#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int buzzer = 8;
int trigger_button = 2;
int hour_button = 7;
int minute_button = 3;

struct Alarm { //struct to hold all important values of the alarm
  int hour;
  int minute;
  int second;
  bool enabled;
  bool triggered;
};

Alarm alarms[] = {
  {9, 43, 0, true, false}
};

const unsigned long alarmLength = 60000; //maximum length of alarm
bool alarmRinging = false;
bool settingAlarm = false;
unsigned long alarmStart = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 1000; //slows down lcd refreshes so screen isn't buggy
const unsigned long debounce = 300; //adds a delay between button presses so its not too rapid
unsigned long lastHourPress = 0;
unsigned long lastMinutePress = 0;
unsigned long lastActivity = 0;

void alarmTriggered() {
  alarmRinging = true;
  alarmStart = millis();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ALARM TRIGGERED");
  lcd.setCursor(0,1);
  lcd.print("PRESS TO MUTE");
}

void stopAlarm() {
  alarmRinging = false;
  alarmStart = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  noTone(buzzer);
}

void setTime() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET ALARM TIME");
  lcd.setCursor(0,1);
  lcd.print(alarms[0].hour);
  lcd.print(":");
  lcd.print(alarms[0].minute);
}

void alarmReached(DateTime now) {
  if (alarms[0].hour == now.hour() && alarms[0].minute == now.minute()) { //checks if conditions of alarm are met
    if (!alarms[0].triggered) { //checks if triggered is not yet true to avoid refiring
    alarms[0].triggered = true;
    alarmTriggered();
    }
  }
    else {
      alarms[0].triggered = false;
    }
}

void setup() {
  pinMode(buzzer, OUTPUT); 
  pinMode(hour_button, INPUT_PULLUP);
  pinMode(minute_button, INPUT_PULLUP);
  pinMode(trigger_button, INPUT_PULLUP);

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  lcd.init();
  lcd.backlight();

}

void loop() {
  unsigned long currentMillis = millis();

  if (digitalRead(hour_button) == LOW && (currentMillis - lastHourPress >= debounce) ) { //checks if hour button has been pressed and whether enough time has passed since last button press (300 ms)
    alarms[0].hour = (alarms[0].hour + 1) % 24;
    settingAlarm = true;
    lastHourPress = currentMillis;
    lastActivity = currentMillis;
    setTime();
  }
  if (digitalRead(minute_button) == LOW && (currentMillis - lastMinutePress >= debounce) ) { //checks if minute button has been pressed and whether enough time has passed since last button press (300 ms)
    alarms[0].minute = (alarms[0].minute + 1) % 60;
    settingAlarm = true;
    lastMinutePress = currentMillis;
    lastActivity = currentMillis;
    setTime();
  }
  if(settingAlarm && (currentMillis - lastActivity >= 5000)) { //if button has not been pressed for 5 seconds, resets back to clock screen
    settingAlarm = false;
    lcd.clear();
  }

  if ((currentMillis - alarmStart >= alarmLength) && alarmRinging) { //checks if alarm has going for a minute, at which point it will stop
    stopAlarm();
  }

  if(alarmRinging) {
    tone(buzzer, 500);
  }

  if (digitalRead(trigger_button) == LOW && alarmRinging) {
    stopAlarm();
  }

  if((currentMillis - previousMillis >= interval)) { //checks if enough time has passed to refresh lcd
    previousMillis = millis();
    DateTime now = rtc.now();
    alarmReached(now);
    if (!settingAlarm && !alarmRinging) {
    lcd.setCursor(0,0);
    lcd.print(now.month()); 
    lcd.print("/");
    lcd.print(now.day());
    lcd.print("/");
    lcd.print(now.year());
    lcd.setCursor(0,1);
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second());
    }
  }
}
