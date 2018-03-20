#include <LCD_1602_RUS.h>
#include <Stepper.h>
#include <SoftwareSerial.h>

class AutoW {
  public:
    void write_state(char num);
    void Go(int del);
    void Read_uart();
    void parseIt(String input_string);
    void init();
    void run();
    void PrintFinalMass();
    void PrintSetTare();
    void PrintDoze();
   


  private:
  


#define NEW_LINE '\n' // Символ новой строки.
#define CLEAR_CHAR '\0' // Символ признак конца строки.
    const int stepsPerRevolution = 200;

    int PIN = 6;
    int rxPin = 11;
    int txPin = 9;
    LCD_1602_RUS lcd{0x27, 16, 2};
    SoftwareSerial mySerial {rxPin, txPin};

    float test_doze = 1.0;
    int Button = 13;
    int Pot = A7;
    String input_string = "";
    float mass; //Вес на весах
    //String mass;
    int work = 0; //Кнопка старта уже нажата
    int finished = 0; //Завершение процесса взвешивания
    int tare = 0; //Функция тарирования
    int dose; //Дозировка
    int max_dose = 150; //Максимальный вес
    int min_dose = 50; //Минимальный вес
};



void AutoW::write_state(char num)
{
  lcd.setCursor(15, 1);
  lcd.write(num);

}

void AutoW::Go(int del) {
  digitalWrite(PIN, HIGH);
  delay(del);
  digitalWrite(PIN, LOW);
  delay(del);
}

void AutoW::Read_uart()
{
  long t = 0;
  while (mySerial.available() > 0 )
  {
    char c = mySerial.read();

    if (c == NEW_LINE) { //считается, что команда завершена, если пришел символ перевода строки
      parseIt(input_string);
      input_string = "";
      t = millis();
      Serial.println(mass);
    }

    else {
      input_string += c;
    }
  }
}

void AutoW::parseIt(String input_string) {

  if (input_string.substring(0, 2) == "US") {
    finished = 0;
  }
  else if (input_string.substring(0, 2) == "ST") {
    finished = 1;
  }
  if (input_string.substring(3, 5) == "GS") {
    tare = 0;
  }
  else if (input_string.substring(3, 5) == "NT") {
    tare = 1;
  }
  mass = input_string.substring(5, 13).toFloat();
}

void AutoW::init() {

  digitalWrite(3, HIGH);
  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея
  lcd.print("\"ЛАР технологии\"");
  lcd.setCursor(0, 1);
  lcd.print("Эскобар-мини 1.0");
  delay(500);
  lcd.clear();
  Serial.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(9600);
  pinMode (Button, INPUT);
  digitalWrite(3, HIGH);

}


void AutoW::PrintFinalMass() {
  lcd.setCursor(0, 0);
  lcd.print("-Итоговая масса-");
  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.print(mass);
  lcd.print("г      ");
}

void AutoW::PrintSetTare() {
  lcd.setCursor(0, 0);
  lcd.print("Установите тару ");
  lcd.setCursor(0, 1);
  lcd.print("    на весы     ");
}

void AutoW::PrintDoze() {
  lcd.setCursor(0, 0);
  lcd.print("        Запуск->");
  lcd.setCursor(0, 1);
  lcd.print("Доза:");
  lcd.setCursor(5, 1);
  lcd.print((String)dose);
  lcd.print("г       ");
}

void AutoW::run() {
  //   put your main code here, to run repeatedly:
  dose = analogRead(Pot); //Считываем показания потенциометра
  dose = dose / 4;
  if (work == 0) // Кнопка не нажата, режим ожидания(выводим показания весов и выбранной дозировки)
  {
    Read_uart();
    if (tare == 0) //Не включена функция тарирования
    {
      //lcd.clear();
      PrintSetTare();
      write_state('1');
    }

    else if (tare == 1 )
    {
      //lcd.clear();
      PrintDoze();

      write_state('2');
      if (digitalRead(Button) == HIGH) {
        work = 1; //Запускаем процесс взвешивания
      }
    }
  }

  if (work == 1) //Точное дозирование
  {
    //Кнопка уже нажата, крутим шнек на полной скорости, вес < дозировки * 0.2
    if (mass < dose - 2.0)
    {

      Go(5);
      Read_uart();
      lcd.setCursor(0, 0);
      lcd.print(mass);

    }

    //Кнопка уже нажата, крутим шнек порциями, вес < дозировки * 0.2
    if (mass < dose && mass >= dose - 2.0)
    {
      Go(20);
      delay(200);
      Read_uart();
      lcd.setCursor(0, 0);
      lcd.print(mass);
    }
    if (mass >= dose - 0.2)// Кнопка уже нажата, шнек остановлен, вес >= дозировки, взвешивание завершено, выводим просьбу заменить баночку
    {
      write_state('5');
      delay(2000);
      if (finished == 1)
      {
        PrintFinalMass();
        work = 0;
        write_state('6');
      }
    }
  }
}


AutoW auto_w;
void setup() {
  auto_w.init();
}

void loop() {
  auto_w.run();
}
