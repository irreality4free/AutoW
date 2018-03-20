 #include <LCD_1602_RUS.h>
#include <Stepper.h>
#include <SoftwareSerial.h>


// Символ новой строки.
#define NEW_LINE '\n'

// Символ признак конца строки.
#define CLEAR_CHAR '\0'


const int stepsPerRevolution = 200;

LCD_1602_RUS lcd(0x27, 16, 2); 

int PIN = 6;
const byte rxPin = 11;
const byte txPin = 9;

// set up a new serial object
SoftwareSerial mySerial (rxPin, txPin);


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

void write_state(char num)
{
  lcd.setCursor(15, 1);
  lcd.write(num);

}


void Go(int del) {
  digitalWrite(PIN, HIGH);
  delay(del);
  digitalWrite(PIN, LOW);
  delay(del);
}

void Read_uart()
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

void parseIt(String input_string) {


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


void setup() {
  //lcd
  digitalWrite(3, HIGH);
  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея
  lcd.print("\"ЛАР технологии\"");
  lcd.setCursor(0, 1);
  lcd.print("Эскобар-мини 1.0");
  delay(500);
  lcd.clear();

  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(9600);

  pinMode (Button, INPUT);
  digitalWrite(3, HIGH);

}

void loop() {
  //   put your main code here, to run repeatedly:
  dose = analogRead(Pot); //Считываем показания потенциометра
  dose = dose / 4;
  if (work == 0) // Кнопка не нажата, режим ожидания(выводим показания весов и выбранной дозировки)
  {
    Read_uart();

    
    if (tare == 0) //Не включена функция тарирования
    {
      //lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Установите тару ");
      lcd.setCursor(0, 1);
      lcd.print("    на весы     ");

      write_state('1');

    }

    else if (tare == 1 )
    {
      //lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("        Запуск->");
      lcd.setCursor(0, 1);
      lcd.print("Доза:");
      lcd.setCursor(5, 1);
      lcd.print((String)dose);
      lcd.print("г       ");
      write_state('2');
      if (digitalRead(Button) == HIGH) {
        work = 1; //Запускаем процесс взвешивания
      }
    }
  }

  if(work == 1) //Точное дозирование
  {
    while(1){
    //Кнопка уже нажата, крутим шнек на полной скорости, вес < дозировки * 0.2
    if (mass < dose - 1.85)
    {

      Go(0);

      Read_uart();
      lcd.setCursor(0, 0);
      lcd.print(mass);
      /*
        lcd.setCursor(0, 0);
        lcd.print("--Дозирование--");
        lcd.setCursor(0, 1);
        lcd.print("Готово:");
        lcd.print(mass);
        lcd.print("г   ");
        write_state('3');*/

    }
//   
    if (mass < dose - 0.85 && mass >= dose -1.85)
    {

      Go(5);

      Read_uart();
      lcd.setCursor(0, 0);
      lcd.print(mass);
      /*
        lcd.setCursor(0, 0);
        lcd.print("--Дозирование--");
        lcd.setCursor(0, 1);
        lcd.print("Готово:");
        lcd.print(mass);
        lcd.print("г   ");
        write_state('3');*/

    }

    //Кнопка уже нажата, крутим шнек порциями, вес < дозировки * 0.2
    if (mass < dose && mass >= dose -0.85)
    {
      Go(20);
      delay(200);
      Read_uart();
      lcd.setCursor(0, 0);
      lcd.print(mass);
      /*
        lcd.setCursor(0, 0);
        lcd.print("--Дозирование--");
        lcd.setCursor(0, 1);
        lcd.print("Готово:");
        lcd.print(mass);
        lcd.print("г   ");

        write_state('4');*/

    }

    // Кнопка уже нажата, шнек остановлен, вес >= дозировки, взвешивание завершено, выводим просьбу заменить баночку
    if (mass >= dose - 0.3)
    {
      write_state('5');

      //Stop();
      delay(2000);
      if (finished == 1)
      {
        lcd.setCursor(0, 0);
        lcd.print("-Итоговая масса-");
        lcd.setCursor(0, 1);
        lcd.print("     ");
        lcd.print(mass);
        lcd.print("г      ");
        delay(3000);
        work = 0;
        write_state('6');
        break;
      }
    }
    }
  }



}
