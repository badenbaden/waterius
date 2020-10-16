#ifndef _WATERIUS_SETUP_h
#define _WATERIUS_SETUP_h

#include <Arduino.h>

/*
#define BUILD_WATERIUS_4C2W 1
*/

#ifdef BUILD_WATERIUS_4C2W
#define WATERIUS_4C2W 1  // attiny84 - 4 счетчика импульсов
#else
#define WATERIUS_2C 0    // attiny85 - 2 счетчика импульсов
#endif 


/* 
	Включение логирования
	3 pin TX -> RX (TTL-USB 3.3 или 5в), 9600 8N1
	При логировании не работает счетчик2 на 3-м пине (Вход 2).

    #define LOG_ON
*/

#ifndef LOG_ON
    #define LOG_BEGIN(x)
    #define LOG(x)
#else
    #undef LOG_BEGIN
    #undef LOG

    // TinyDebugSerial на PB3 только в attiny85, 1MHz
    #if defined(WATERIUS_2C)
        #include "TinyDebugSerial.h"
        #define LOG_BEGIN(x)  mySerial.begin(x)
        #define LOG(x) mySerial.print(millis()); mySerial.print(F(" : ")); mySerial.println(x);
    #endif 
    #if defined(WATERIUS_4C2W)
    // TinySoftwareSerial на PB3 attiny84, 1MHz
        #define LOG_BEGIN(x)  Serial.begin(x); ACSR &=~(1<<ACIE); ACSR |=~(1<<ACD);  //only TX
        #define LOG(x) Serial.print(millis()); Serial.print(F(" : ")); Serial.println(x);
    #endif 
#endif

//#define TEST_WATERIUS   // Тестирование счетчика при помощи Arduino

/*
	Период отправки данных на сервер, мин
*/
#ifdef TEST_WATERIUS
unsigned int WAKE_EVERY_MIN   10
#else
unsigned int WAKE_EVERY_MIN   24 * 60U  // 1U 
#endif

/*
	Аварийное отключение, если ESP зависнет и не пришлет команду "сон".
*/
#define WAIT_ESP_MSEC    120000UL      

/*
	Сколько милисекунд пользователь может 
	настраивать ESP. Если не закончил, питание ESP выключится.
*/
#define SETUP_TIME_MSEC  600000UL 

/*
    время долгого нажатия кнопки, милисекунд 
*/
#define LONG_PRESS_MSEC  3000   
       

#if defined(WATERIUS_2C)
struct Data {
    uint32_t value0;
    uint32_t value1;
};

struct CounterState { // не добавляем в Data, т.к. та в буфере кольцевом
    uint8_t  state0;  // состояние входа
    uint8_t  state1; 
};

struct ADCLevel {
    uint16_t      adc0;
    uint16_t      adc1; 
};
#endif

#if defined(WATERIUS_4C2W)

#define LEAK_CHECK_PERIOD 31   //8 сек, т.к. 8 * 4 раза в сек - 1 = 32

struct Data {
    uint32_t value0;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
};

struct CounterState { // не добавляем в Data, т.к. та в буфере кольцевом
    uint8_t state0;  // состояние входа
    uint8_t state1; 
    uint8_t state2;
    uint8_t state3;
};

struct ADCLevel {
    uint16_t adc0;
    uint16_t adc1; 
    uint16_t adc2; 
    uint16_t adc3; 
};
#endif

struct Header {

    /*
    Версия прошивки
    */
    uint8_t       version;
    
    /*
    Причина перезагрузки (регистр MCUSR datasheet 8.5.1):
         0001 - PORF: Power-on Reset Flag. Напряжение питания было низкое или 0.
         0010 - EXTRF: External Reset Flag. Пин ресет был в низком уровне.
         0100 - BORF: Brown-out Reset Flag. Напряжение питание было ниже требуемого. 
         1000 - WDRF: Watchdog Reset Flag. Завершение работы таймера.

    8  - 1000 - WDRF
    9  - 1001 - WDRF + PORF
    10 - 1010 - WDRF + EXTRF
    */
    uint8_t       service; 

    /*
    Напряжение питания в мВ.
    */
    uint32_t      voltage;
    
    /*
    Количество перезагрузок.
    */
    uint8_t       resets;

    /*
    Модификация
    0 - Классический. 2 счетчика
    1 - 4C2W. 4 счетчика
    */
    uint8_t       model;

    CounterState  states;  //TODO убрать
    Data          data;
    ADCLevel      adc;

    // HEADER_DATA_SIZE

    uint8_t       crc;
    uint8_t       reserved2;
};  //22 байт

#if defined(WATERIUS_2C)
    #define HEADER_DATA_SIZE 22
#else
    #define HEADER_DATA_SIZE 36

    struct LeakHeader
    {
        uint8_t       state1;
        uint8_t       state2;
        uint16_t      adc1;
        uint16_t      adc2;

        //LEAK_HEADER_SIZE

        uint8_t       crc;
        uint8_t       reserved2;
    }; //8 байт
    #define LEAK_HEADER_SIZE 6
    #define LEAK_HEADER_SIZE_CRC 8
#endif

 #define TX_BUFFER_SIZE HEADER_DATA_SIZE + 2

#endif
