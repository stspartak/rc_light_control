/*

 RC Light control Arduino Pro Micro (Leonardo) v1.0

  Описание
  - PINCH1 - канал газа (управляет 2 выходами)
  - PINCH2 - тумблер (управляет 2 выходами)

  Версия 1.0
  - Автокалибровка при включении
  - Нет сигнала - горит АВАРИЙНАЯ СИГНАЛИЗАЦИЯ
  - Газ вперед - лампы не горят
  - Газ в нейтрали - горит СТОП
  - Газ в нейтрали 5сек - горит АВАРИЙНАЯ СИГНАЛИЗАЦИЯ
  - Газ назад - горит ЗАДНИЙ ХОД.
  - Тублер в положении отличающемся от начального включает СВЕТ.

*/

#include <Arduino.h>

// ----------------------- ПИНЫ ---------------------------
// Входные пины
#define PINCH1 4 // Пин канала ГАЗА
#define PINCH2 5 // Пин канала ТУМБЛЕРА
// Выходные пины
#define REVERS_LIGHT 10 // Пин ЗАДНЕГО ХОДА
#define STOP_LIGHT 14   // Пин СТОП СИГНАЛА
#define REAR_LIGHT 18   // Пин ГАБАРИТОВ
#define FRONT_LIGHT 20  // Пин СВЕТА
// ----------------------- ПИНЫ ---------------------------

// --------------------- ПЕРЕМЕННЫЕ ----------------------
int ppm_throttle, ppm_switch;
int throttle_min;          // Нижняя граница нейтрали газа,
int throttle_max;          // Верхняя граница нейтрали газа
int switch_min;            // Нижняя граница тумблера
int switch_max;            // Верхняя граница тумблера
bool calibration_flag = 0; // Флаг калибровки

uint32_t last_time;
int throttle_state = 0;
// --------------------- ПЕРЕМЕННЫЕ ----------------------

// --------------------- ИНИЦИАЛИЗАЦИЯ ----------------------
void setup()
{
  pinMode(PINCH1, INPUT);        // Вход канала ГАЗА
  pinMode(PINCH2, INPUT);        // Вход канала ТУМБЛЕРА
  pinMode(STOP_LIGHT, OUTPUT);   // Выход для ламп СТОП СИГНАЛА
  pinMode(REVERS_LIGHT, OUTPUT); // Выход для ламп ЗАДНЕГО ХОДА
  pinMode(FRONT_LIGHT, OUTPUT);  // Выход для лам переднего СВЕТА
  pinMode(REAR_LIGHT, OUTPUT);   // Выход для лам ГАБАРИТОВ
}
// --------------------- ИНИЦИАЛИЗАЦИЯ ----------------------

// ----- Проверка сигнала -----
bool chekSignal()
{
  return pulseIn(PINCH1, HIGH) < 1 && pulseIn(PINCH2, HIGH) < 1;
}

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
    digitalWrite(REAR_LIGHT, !digitalRead(REAR_LIGHT));
    flag = !flag;
  }
}

// ----- Калибровка -----
void calibration()
{
  if (calibration_flag == 0)
  {
    while (millis() < 3000)
    {
      digitalWrite(REAR_LIGHT, HIGH);       // Начало калибровки. Включаем СТОП СИГНАЛ на 3 секунды
      ppm_throttle = pulseIn(PINCH1, HIGH); // Чтение канала ГАЗА
      throttle_min = ppm_throttle - 25;     // Нижняя граница нейтрали ГАЗА
      throttle_max = ppm_throttle + 25;     // Верхняя граница нейтрали ГАЗА
      ppm_switch = pulseIn(PINCH2, HIGH);   // Чтение канала ТУМБЛЕРА
      switch_min = ppm_switch - 50;         // Нижняя граница нейтрали ТУМБЛЕРА
      switch_max = ppm_switch + 50;         // Верхняя граница нейтрали ТУМБЛЕРА
    }
    digitalWrite(REAR_LIGHT, LOW); // Конец калибровки. Выключаем лампы ГАБАРИТОВ
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

  // if (millis() - last_time > 1000) // условие включения АВАРИЙНОЙ СИГНАЛИЗАЦИИ
  // {
  //   if (ppm_throttle > throttle_min && ppm_throttle < throttle_max)
  //   {
  //     if (throttle_state == 5)
  //     {
  //       alarm();
  //     }
  //     else
  //     {
  //       throttle_state++;
  //     }
  //   }
  //   else
  //   {
  //     throttle_state = 0;
  //   }
  // }
}

// ----- Канал ТУМБЛЕРА -----
void ch2()
{
  ppm_switch = pulseIn(PINCH2, HIGH);                     // Чтение канала ТУМБЛЕРА
  if (ppm_switch < switch_min || ppm_switch > switch_max) // Условие включения СВЕТА и ГАБАРИТОВ
  {
    digitalWrite(FRONT_LIGHT, HIGH);
    digitalWrite(REAR_LIGHT, HIGH);
  }
  else if (ppm_switch > switch_min || ppm_switch < switch_max || ppm_switch == 0)
  {
    digitalWrite(FRONT_LIGHT, LOW);
    digitalWrite(REAR_LIGHT, LOW);
  }
}

// ------------------------------ ОСНОВНОЙ ЦИКЛ -------------------------------
void loop()
{
  // if (chekSignal())
  // {
  //   alarm();
  // }
  // else
  // {
    calibration(); // Калибровка нейтрали при включении или сбросе (3 секунды)
    ch1();         // Чтение канала ГАЗА, управление СТОПОМ и ЗАДНИМ ХОДОМ
    ch2();         // Чтение канала ТУМБЛЕРА, управление СВЕТОМ
  //}
}
// ------------------------------ ОСНОВНОЙ ЦИКЛ -------------------------------
