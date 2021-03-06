#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <SPI.h>
#include "getTemp.h"  // Вынесена функция температуры в отдельный файл и OneWire.h Wire.h. 

// Дисплей Nokia 5110
//  LCD Nokia 5110 ARDUINO
//  1 RST 3
//  2 CE 4
//  3 DC 5
//  4 DIN 6
//  5 CLK 7
//  6 VCC 3.3V
//  7 LIGHT GND
//  8 GND GND
// ----------  Определение пинов устройств и переменных
//  На 9 пине датчик температуры.

  #define pinButton  11  //пин кнопки
  #define pinVRX A0 //Джойстик Х
  #define pinVRY A1 //Джойстик Y
  #define pizoPin  8 // пин зумера
  
  OneWire ds(9); // пин датчика температуры DS
  int getTemp(OneWire *ds); // Вынесено в файл и должно быть объявлено в main.

  Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

 bool button_state = false; //состояние кнопки
 int valButton; // счётчик удержания кнопки ++ 
 long timeButton = 2000; // установленно в 2000 фактически 2сек.
 uint32_t ms_button = 0;  //Время нажатия кнопки для устранения антидребезга

 int valButtonSub; // счётчик нажатия кнопки ++ 
 long timeButtonSub = 3; // установленно в 3 фактиечески 3мс.
 uint32_t ms_buttonSub = 0;  //Время нажатия кнопки для устранения антидребезга
 bool buttonStateSub = false;

 unsigned long timeLoopAlarm; //Время для таймера моргающего экраном аларма
 const int watermeterPin = 2; // пин датчика воды

 short int flagMenu = 0; // флаг меню, по этому флагу понимаем в какое меню попадаем 0 - Temp, 1 = Flow.
 
 int temp; // переменная температуры
 int setTemp = 30; //значение предустановленной критичной температуры
 int setFlow = 100; //значение предустановленного критическго потока
// Переменные потока датчика воды
 volatile int  pulse_frequency;
 unsigned int  literperhour;
 unsigned long currentTime, loopTime;
 byte sensorInterrupt = 0;

// Какртинка заставка, при загрузке
static const unsigned char PROGMEM logo[] = 
{
B11111111,B00000001,B10000001,B11111110,B00000000,B00000111,B11111100,B00000000,B01100000,B11111111,
B11111100,B00000111,B11100000,B01111111,B10000000,B00111111,B11111111,B00000000,B01111000,B00011111,
B11110000,B00000111,B11100000,B00011111,B10000000,B00111111,B11111111,B00000000,B01111000,B00000111,
B11100000,B00001111,B11000000,B00001111,B10000000,B00111111,B11111111,B00000000,B01111000,B00000011,
B11100000,B00001111,B11000000,B00001111,B10000000,B00111111,B11111111,B00000000,B01111000,B00000011,
B11100000,B00001111,B11000000,B00011111,B10000000,B00111111,B11111111,B00000000,B01111000,B00000011,
B11111000,B00001111,B11000000,B00111111,B10000000,B00111111,B10011111,B00000000,B11111000,B00000111,
B11111110,B00000111,B11000000,B11111111,B10000000,B00111110,B00011110,B00000000,B11111000,B00111111,
B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,
B11111111,B11111111,B10000001,B11111111,B11111111,B11111111,B11111111,B10000000,B11111111,B11111111,
B11111111,B11111111,B11100000,B01111111,B11111111,B11111111,B11111110,B00000011,B11111111,B11111111,
B11111111,B11111110,B00000000,B00011111,B11111111,B11111111,B11111000,B00000000,B01111111,B11111111,
B11100000,B01111100,B00000000,B00000000,B01111111,B11111111,B00000000,B00000000,B00111111,B00000011,
B11100111,B00111100,B00011111,B10000000,B00001111,B11110000,B00000001,B11111000,B00011110,B01111011,
B11100001,B10111100,B00111111,B11111000,B00000001,B10000000,B00011111,B11111100,B00011110,B11000011,
B11100011,B11011100,B00111111,B11111111,B00000001,B10100000,B11111111,B11111110,B00111101,B11100011,
B11110000,B01111100,B00011111,B11111111,B11111111,B11111111,B11111111,B11111000,B00111111,B00000111,
B11110000,B11111100,B00001111,B11111111,B11111111,B11111111,B11111111,B11110000,B00111111,B10001111,
B11111011,B11111110,B00000000,B01110000,B01111111,B11111111,B00001110,B00000000,B01111111,B11101111,
B11111111,B11111110,B00000000,B00000000,B00001111,B11110000,B00000000,B00000000,B01111111,B11111111,
B11111111,B11111111,B00000000,B00000111,B11111111,B11111111,B11100000,B00000000,B11111111,B11111111,
B11111111,B11111111,B11000000,B00011100,B00000011,B11100000,B00111000,B00000011,B11111111,B11111111,
B11111111,B11111111,B11111111,B10011001,B11100000,B00000111,B10011001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11110011,B11111000,B00011111,B11001101,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11110011,B11111110,B01111111,B11001111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111000,B00001110,B01000000,B00011111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111100,B00000000,B00000000,B00011111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11000000,B00000000,B00000000,B00000011,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11000000,B00000000,B00000000,B00000011,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000000,B00000000,B00000000,B00000001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000000,B00000000,B00000000,B00000001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000000,B00000000,B00000000,B00000001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000000,B00011111,B11111000,B00000001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000001,B11111001,B00001111,B11000000,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11111111,B00000000,B00000000,B01111111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B10000000,B00000011,B11100000,B00000001,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11100000,B11011111,B11111000,B00000111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11100111,B11111111,B11111111,B10000111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11110111,B11111111,B11111111,B11101111,B11111111,B11111111,B11111111,
B11111111,B11111111,B11111111,B11110111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,
B11000011,B11000011,B11100110,B01111100,B00111100,B11100001,B11100111,B11000111,B00110000,B11110111,
B11100001,B11000011,B10001110,B00011111,B00001111,B11100001,B11011111,B10000111,B11111000,B01111111,
B11110000,B11000011,B00001110,B00011101,B10000011,B11100001,B10001111,B10000110,B11111100,B00111111,
B11111000,B11000011,B10001110,B00011101,B11100001,B11100001,B10000111,B10000111,B11111110,B00011111,
B11001101,B10000011,B11000110,B01111100,B11110001,B11100000,B10000011,B10000111,B00111100,B00011111  
  };

//Значок потока, для обозначения с датчика потока
static const unsigned char PROGMEM cooler[] =
{ 
B00000110,B00011111,B11000000,
B00011110,B00011111,B10000000,
B00111110,B00011111,B00000000,
B01111000,B00011111,B10000000,
B11110000,B00011011,B11000000,
B11100000,B00000001,B11000000,
B11100000,B00000001,B11000000,
B11100000,B00000001,B11000000,
B11100000,B00000001,B11000000,
B11100000,B00000001,B11000000,
B11110110,B00000011,B11000000,
B01111110,B00000111,B10000000,
B00111110,B00011111,B00000000,
B01111110,B00011110,B00000000,
B11111110,B00011000,B00000000
};

// Значок нагрева воды
static const unsigned char PROGMEM heat[] =
{ 
B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,
B00001100,B00100001,B10000000,
B00001000,B01100011,B00000000,
B00011000,B01100011,B00000000,
B00011000,B11100011,B00000000,
B00011100,B11100011,B00000000,
B00011100,B01100011,B10000000,
B00001110,B01110011,B10000000,
B00001110,B00110001,B11000000,
B00000110,B00110001,B11000000,
B00000110,B00110000,B11000000,
B00000100,B00110001,B10000000,
B00001100,B00100001,B10000000,
B00001000,B01000001,B00000000,
B00000000,B00000000,B00000000,
B00111111,B11111111,B11000000,
B00111111,B11111111,B11000000,
B00111111,B11111111,B11000000,
B00000000,B00000000,B00000000
};

// Значок восклицательный знак для вывода алармов.
static const unsigned char PROGMEM warning[] = 
{
B00000000,B11111111,B00000000,
B00000011,B11111111,B11000000,
B00001111,B00000000,B11110000,
B00011100,B00000000,B00111000,
B00110000,B00111100,B00001100,
B01110000,B00111100,B00001110,
B01100000,B00111100,B00000110,
B11000000,B00111100,B00000011,
B11000000,B00011000,B00000011,
B11000000,B00011000,B00000011,
B11000000,B00011000,B00000011,
B11000000,B00011000,B00000011,
B11000000,B00000000,B00000011,
B01100000,B00011000,B00000110,
B01110000,B00111100,B00001110,
B00110000,B00111100,B00001100,
B00011100,B00000000,B00111000,
B00001111,B00000000,B11110000,
B00000011,B11111111,B11000000,
B00000000,B11111111,B00000000
};

// функция полученя данных с датчика потока, работает по прерыванию
void  getFlow ()
{
  pulse_frequency++;
}

/*Функция вывода на экран */
int displayShow(int f, int t)
 {
  
    
    display.drawBitmap(0,0,heat,24,20,1);
    display.setTextSize(3);
    display.setCursor(26,0);
    display.println(t);
    display.setTextSize(1);
    display.setCursor(63,0);
    display.println("o");
    display.setTextSize(2);
    display.setCursor(67,7);
    display.println("C");
    //
    //display.setTextSize(1);
    display.drawBitmap(0,22,cooler,24,15,f);  // удобно f ставить, если = 0 то иконки нет.
    display.setTextSize(2);
    display.setCursor(26,22);
    display.println(f, DEC);
    //
    display.setTextSize(1);
    display.setCursor(0,40);
    display.print("[T-");
    display.print(setTemp);
    display.print("]");
    display.print("[F-");
    display.print(setFlow);
    display.print("]");

    
    display.display();
    display.clearDisplay();

    return 0;
  }

/*Функция вывода сигнализации на экран*/
int displayAlarm(int errorcode, int f, int t) 
{
switch (errorcode) { 

  case 1:
    tone (pizoPin,500,500);
    digitalWrite(10, LOW); //зажигает подсветку.
    display.clearDisplay();
    display.drawBitmap (0,15,warning,24,20,1);
    display.setTextSize(1);
    display.setCursor(10,0);
    display.println("HIGH WATER!");
    display.setTextSize(3);
    display.setCursor(28,15);
    display.println(t);
    display.setTextSize(1);
    display.setCursor(65,12);
    display.println("o");
    display.setTextSize(2);
    display.setCursor(68,20);
    display.println("C");
    display.setTextSize(1);
    display.setCursor(0,40);
    display.println("STOP WORK NOW!");
    display.display();
    
    if (millis() - timeLoopAlarm >= 3000) {
         digitalWrite(10, HIGH); // гасим экран.
     timeLoopAlarm = millis();
     
     }
     
     
    
    break;

  case 2:
     break;

 
     }  
  return 0;
  } 
/*Данная функция по работе с меню установки температуры*/
int menuSet() {

  switch (flagMenu)
  {
  case 0:
    
    digitalWrite(10, LOW); //зажигает подсветку.
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("> Set temp [T]");
    display.setTextSize(2);
    display.setCursor(5,15);
    display.print("<-");
    display.print(setTemp, DEC);
    display.print("+>");
    display.setTextSize(1);
    display.setCursor(0,40);
    display.println("Hold to next");
    display.display();
    
    break;
  case 1:
    
    digitalWrite(10, LOW); //зажигает подсветку.
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("> Set flow [F]");
    display.setTextSize(2);
    display.setCursor(0,15);
    display.print("<-");
    display.print(setFlow, DEC);
    display.print("+>");
    display.setTextSize(1);
    display.setCursor(0,40);
    display.println("Hold to save");
    display.display();
    
    break;
  default:
    break;
  }
    
    
    // Обработка событий кноки

    if (digitalRead(pinButton)==LOW )  {
              // считаем каждую миллисекунду и инкрементируем valButtonSub
           if (millis() - ms_buttonSub >= 1) {
            ms_buttonSub = millis();
             valButtonSub++;
            }
    }
       //Если кнопка отпущена, то сбрасываем valButtonSub считая, что это  дребезг или не нажатие.
     else { valButtonSub = 0;}
            
       // Кнопка нажата и проверяем значение valButtonSub, если оно совпадает с длительностью *timeButton то выполняем действие
     if (valButtonSub >= timeButtonSub) {
          buttonStateSub = true; // ставим, что кнопка нажата и удержана в сабменю.
    
           // Данное для выхода из сабменю если кнопка нажата дольше.
            if(valButtonSub >= 150 && flagMenu == 0) {
           
               valButtonSub = 0;
               display.clearDisplay();
               flagMenu = 1;
            }
            if(valButtonSub >= 150 && flagMenu == 1) {
           
               valButtonSub = 0;
               display.clearDisplay();
               flagMenu = 0;
               button_state = false;
            }

            
     } 
                
       // Обработка событий джойстика
       
      if (analogRead(pinVRY) > 1000) {  // Если стик вправо то ++
       
       switch (flagMenu)
       {
       case 0:
        setTemp++;
        delay(500);
         break;

        case 1:
        setFlow++;
        delay(500);
        break; 
       
       default:
         break;
       }
        
      
      }
    
    
      if (analogRead(pinVRY) < 400) {  // Если стик влево то --
        
        switch (flagMenu)
       {
       case 0:
        setTemp--;
        delay(500);
         break;

        case 1:
        setFlow--;
        delay(500);
        break; 
       
       default:
         break;
       }
        
       }
 
 return 0;
}

/*Основная функция, запрашивает поток и температуру, а затем отсылает на дисплей.*/
int rootSys() {

currentTime = millis();
  if (currentTime >= (loopTime + 1000))
  {
    loopTime = currentTime;
    literperhour = (pulse_frequency * 60 / 7.5);
    pulse_frequency = 0;

    // дёргаем датчик температуры и забираем данные.
    //temp = getTemp();   
temp = getTemp(&ds);   

 
 //Проверка условий температуры и потока воды
   if (temp >=setTemp  ) 
   {
     int error = 1;   // ставим номерок кода и передаём в функцию Аларма, там само пусть разбирается что писать.
     displayAlarm(error, literperhour, temp); // выводим ошибку.
    
    } 


    if (temp < setTemp) {
    
    //Отправляем данные на экран с текущими показателями.
    displayShow(literperhour, temp);
    }

    // Вынес в loop
    //if (button_state && (temp < setTemp)) {
    //   menuSet();
    //}
       
   }

return 0;
}

void setup()
{
  pinMode(watermeterPin, INPUT);
  // прерывание на пине к которому подключен датчик воды, дёргает когда приходят данные
  attachInterrupt(sensorInterrupt, getFlow, FALLING);
  
  // Проверяем данные и выводим без задержки сравнивая время.
  currentTime = millis();
  loopTime = currentTime;

  /* LCD дисплей, инициализация и заставка */
  display.begin();
  display.setContrast(20);
  display.clearDisplay();
  display.drawBitmap(2,0,logo,80,48,1);
  display.display(); // показываем заставку
  delay(1000);
  display.clearDisplay();   // очищаем экран и буфер
  display.setTextColor(BLACK); // установка цвета текста
  pinMode (10,OUTPUT); //подсветка экрана
  //digitalWrite(10, LOW);

   
 // Пищим при запуске.
tone(pizoPin, 400, 300);
delay(300);
tone(pizoPin, 800, 300);
delay(300);
tone(pizoPin, 1500, 300);

pinMode(pinButton, INPUT);

}


void loop ()
{

/* Кнопка */
// Если кнопка нажата
if (digitalRead(pinButton)==LOW && !button_state) 
{
  
  // считаем каждую миллисекунду и инкрементируем valButton
 if (millis() - ms_button >= 1) {
  ms_button = millis();
  valButton++;
 }
}
//данный кусок сработает если кнопка будет в LOW и сбрсит valButton
else { valButton = 0;}

//пока кнопка нажата, проверяем значение valButton и если оно совпадает с длительностью timeButton то выполняем действие
// timeButton это и есть то самое значение задержки удержания кнопки
if (valButton >= timeButton && !button_state ) {
//digitalWrite(10, (!digitalRead(10))); //зажигает подсветку.
 button_state = true;
 valButton = 0;
}


 //Если кнопка не нажата была, то вертим показания на дисплей rootSys, но если дисплей, то тольк работаем в меню, иначе
 // задержка на работу раз в сек в rootSys тормозит работу кнопок.
  
    if (button_state /*&& (temp < setTemp)*/) {
       menuSet();
    } else {
      rootSys();
    }
   


    if (analogRead(pinVRX) > 1000) {
      digitalWrite(10, LOW); //зажигает подсветку.      
    }

    if (analogRead(pinVRX) < 400) {
      digitalWrite(10, HIGH); //зажигает подсветку.
    }



}

