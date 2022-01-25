/*

 RC Light control Arduino Pro Micro (Leonardo) v1.1

  Описание:
  - PINCH1 - канал ГАЗА
  - PINCH2 - канал ТУМБЛЕРА
  - PINCH3 - канал ТУМБЛЕРА ГИРЛЯНДЫ

  Возможности: 
  - Проверка сигнала ТОЛЬКО при включении (если сигнала нет - горит АВАРИЙНАЯ СИГНАЛИЗАЦИЯ)
  - Автокалибровка при включении (после проверки сигнала)
  - Газ вперед - лампы не горят
  - Газ назад - горит ЗАДНИЙ ХОД
  - Газ в нейтрали - горит СТОП
  - Тублер в положении отличающемся от начального включает СВЕТ и ГАБАРИТЫ

*/

#include <Arduino.h>

// ----------------------- ПИНЫ ---------------------------
// Входные пины
#define PINCH1 4 // Пин канала ГАЗА
#define PINCH2 5 // Пин канала ТУМБЛЕРА
#define PINCH3 6 // Пин канала ТУМБЛЕРА ГИРЛЯНДЫ
// Выходные пины
#define REVERS_LIGHT 15  // Пин ЗАДНЕГО ХОДА
#define STOP_LIGHT 16    // Пин СТОП СИГНАЛА
#define LEFT_LIGHT 20    // Пин ЛЕВЫХ ГАБАРИТОВ
#define RIGHT_LIGHT 18   // Пин ПРАВЫХ ГАБАРИТОВ
#define FRONT_LIGHT 14   // Пин СВЕТА
#define FRONT_GARLAND 10 // Пин ГИРЛЯНДА
// ----------------------- ПИНЫ ---------------------------

// --------------------- ПЕРЕМЕННЫЕ ----------------------
int ppm_throttle, ppm_switch_light, ppm_switch_garland;
int throttle_min;          // Нижняя граница нейтрали газа,
int throttle_max;          // Верхняя граница нейтрали газа
int switch_light_min;      // Нижняя граница тумблера
int switch_light_max;      // Верхняя граница тумблера
int switch_garland_min;    // Нижняя граница тумблера гирлянды
int switch_garland_max;    // Верхняя граница тумблера гирлянды
bool calibration_flag = 0; // Флаг калибровки
// --------------------- ПЕРЕМЕННЫЕ ----------------------

// --------------------- ИНИЦИАЛИЗАЦИЯ ----------------------
void setup()
{
  //Serial.begin(9600);
  pinMode(PINCH1, INPUT);         // Вход канала ГАЗА
  pinMode(PINCH2, INPUT);         // Вход канала ТУМБЛЕРА
  pinMode(STOP_LIGHT, OUTPUT);    // Выход для ламп СТОП СИГНАЛА
  pinMode(REVERS_LIGHT, OUTPUT);  // Выход для ламп ЗАДНЕГО ХОДА
  pinMode(FRONT_LIGHT, OUTPUT);   // Выход для ламп переднего СВЕТА
  pinMode(LEFT_LIGHT, OUTPUT);    // Выход для ламп ЛЕВЫХ ГАБАРИТОВ
  pinMode(RIGHT_LIGHT, OUTPUT);   // Выход для ламп ПРАВЫХ ГАБАРИТОВ
  pinMode(FRONT_GARLAND, OUTPUT); // Выход для ламп ГИРЛЯНДЫ
}
// --------------------- ИНИЦИАЛИЗАЦИЯ ----------------------

// ----- Аварийная сигнализация -----
void alarm()
{
  static uint32_t tmr;
  static bool flag;

  uint32_t period;
  if (flag)
    period = 700;
  else
    period = 350;

  if (millis() - tmr > period)
  {
    tmr = millis();
    digitalWrite(LEFT_LIGHT, !digitalRead(LEFT_LIGHT));
    digitalWrite(RIGHT_LIGHT, !digitalRead(RIGHT_LIGHT));
    flag = !flag;
  }
}

// ----- Калибровка -----
void calibration()
{
  if (calibration_flag == 0)
  { // Начало цикла калибровки
    for (int i = 0; i < 10; i++)
    {
      ppm_throttle = pulseIn(PINCH1, HIGH); // Чтение канала ГАЗА
      throttle_min = ppm_throttle - 25;     // Нижняя граница нейтрали ГАЗА
      throttle_max = ppm_throttle + 25;     // Верхняя граница нейтрали ГАЗА

      ppm_switch_light = pulseIn(PINCH2, HIGH); // Чтение канала ТУМБЛЕРние канала gppm_switch_lightА
      switch_light_min = ppm_switch_light - 50; // Нижняя граница нейтрали ТУМБЛЕРа нейтрали gppm_switch_lightА
      switch_light_max = ppm_switch_light + 50; // Верхняя граница нейтрали ТУМБЛЕРа нейтрали gppm_switch_lightА

      ppm_switch_garland = pulseIn(PINCH3, HIGH);
      switch_garland_min = ppm_switch_garland - 50;
      switch_garland_max = ppm_switch_garland + 50;

      // Serial.print("Газ: ");
      // Serial.print(ppm_throttle);
      // Serial.print(" | Тумблер: ");
      // Serial.println(ppm_switch_light );

      digitalWrite(STOP_LIGHT, 1);
      digitalWrite(LEFT_LIGHT, 1);
      digitalWrite(RIGHT_LIGHT, 1);
      digitalWrite(REVERS_LIGHT, 1);
      digitalWrite(FRONT_LIGHT, 1);
    }
    // Конец цикла калибровки
    digitalWrite(STOP_LIGHT, 0);
    digitalWrite(LEFT_LIGHT, 0);
    digitalWrite(RIGHT_LIGHT, 0);
    digitalWrite(REVERS_LIGHT, 0);
    digitalWrite(FRONT_LIGHT, 0);
    calibration_flag = 1;
  }
}

// ----- Канал ГАЗА -----
void ch1()
{
  ppm_throttle = pulseIn(PINCH1, HIGH); // Чтение канала ГАЗА

  if (ppm_throttle < throttle_min) // Условие включения ЗАДНЕГО ХОДА
  {
    digitalWrite(REVERS_LIGHT, HIGH);
  }
  else
  {
    digitalWrite(REVERS_LIGHT, LOW);
  }

  if (throttle_min <= ppm_throttle && ppm_throttle <= throttle_max) // Условие включения СТОП СИГНАЛА
  {
    digitalWrite(STOP_LIGHT, HIGH);
  }
  else
  {
    digitalWrite(STOP_LIGHT, LOW);
  }
}

// ----- Канал ТУМБЛЕРА ГАБАРИТОВ и ФАР -----
void ch2()
{
  ppm_switch_light = pulseIn(PINCH2, HIGH);               // Чтение канала ТУМБЛЕРА
  if (ppm_switch_light < 1600 && ppm_switch_light > 1400) // Условие включения ГАБАРИТОВ
  {
    digitalWrite(LEFT_LIGHT, HIGH);
    digitalWrite(RIGHT_LIGHT, HIGH);
    digitalWrite(FRONT_LIGHT, LOW);
  }
  else if (ppm_switch_light < 2100 && ppm_switch_light > 1600) // Условие включения ФАР
  {
    digitalWrite(FRONT_LIGHT, HIGH);
  }
  else // Условие отключения ФАР и ГАБАРИТОВ
  {
    digitalWrite(FRONT_LIGHT, LOW);
    digitalWrite(LEFT_LIGHT, LOW);
    digitalWrite(RIGHT_LIGHT, LOW);
  }
}

// ----- Канал ТУМБЛЕРА ГИРЛЯНДЫ -----
void ch3()
{
  ppm_switch_garland = pulseIn(PINCH3, HIGH);                                             // Чтение канала ТУМБЛЕРА ГИРЛЯНДЫ
  if (ppm_switch_garland < switch_garland_min || ppm_switch_garland > switch_garland_max) // Условие включения ГИРЛЯНДЫ
  {
    digitalWrite(FRONT_GARLAND, HIGH);
  }
  else
  {
    digitalWrite(FRONT_GARLAND, LOW);
  }
}

// ------------------------------ ОСНОВНОЙ ЦИКЛ -------------------------------
void loop()
{
  while (pulseIn(PINCH1, HIGH) == 0 && pulseIn(PINCH2, HIGH) == 0) // Проверка сигнала. Может сработать только при первом включении
  {
    alarm();
  }
  calibration(); // Калибровка нейтрали при включении или сбросе
  ch1();         // Чтение канала ГАЗА, управление СТОПОМ и ЗАДНИМ ХОДОМ
  ch2();         // Чтение канала ТУМБЛЕРА, управление ГАБАРИТАМИ и ФАРАМИ
  ch3();         // Чтение канала ТУМБЛЕРА, управление ГИРЛЯНДОЙ
}
// ------------------------------ ОСНОВНОЙ ЦИКЛ -------------------------------
