# Práctica 2. Gerard Cots y Joel J. Morera

## Interrupción por GPIO

Con el fin de llevar a cabo una interrupción por hardware, en el siguiente codigo se define un boton, que se implementa en una protoboard. 

Este genera una interrupción cada vez que se pulsa, apareciendo por pantalla el numero de veces que se ha pulsado. 

Al cabo de un minuto se desconecta la interrupción, llamando a la función `detachInterrup(GPIOPin)`.

El codigo es el siguiente:

```cpp
#include <Arduino.h>

struct Button 
{
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};


Button button1 = {18, 0, false};


void IRAM_ATTR isr()
{
    button1.numberKeyPresses += 1;
    button1.pressed = true;
}


static uint32_t lastMillis = 0;


void setup() 
{
    Serial.begin(115200);
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, FALLING);
}
void loop() {
    if (button1.pressed) 
    {
        Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
        button1.pressed = false;
    }

    //Detach Interrupt after 1 Minute
    if (millis() - lastMillis > 60000) 
    {
        lastMillis = millis();
        detachInterrupt(button1.PIN);
        Serial.println("Interrupt Detached!");
    }
} 
```

Las salidas que se han obtendido de este codigo son:

```bash
Button 1 has been pressed 1 times
Button 1 has been pressed 2 times
Button 1 has been pressed 3 times
Button 1 has been pressed 4 times
Button 1 has been pressed 5 times
...
Button 1 has been pressed 2606 times
Interrupt Detached!
```

( Debido a que el en laboratorio no disponiamos de pulsadores, hemos simulado el puslador usando un cortocircuito, con lo cual aparecen rebotes, disparando el numero de veces que se presiona el pulsador. )

## Interrupción por Timer

Seguidamente, se describe un timer

El codigo es el siguiente:

```c
#include <Arduino.h>

volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() 
{
    Serial.begin(115200);
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);
}

void loop() 
{
    if (interruptCounter > 0)
    {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);
        totalInterruptCounter++;
        
        Serial.print("An interrupt has occurred. Total number: ");
        Serial.println(totalInterruptCounter);
    }
}
```

Las salidas que se han obtendido de este codigo son:

```bash
An interrupt has occurred. Total number: 1
An interrupt has occurred. Total number: 2
An interrupt has occurred. Total number: 3
An interrupt has occurred. Total number: 4
An interrupt has occurred. Total number: 5
An interrupt has occurred. Total number: 6
An interrupt has occurred. Total number: 7
An interrupt has occurred. Total number: 8
An interrupt has occurred. Total number: 9
...
```

## Extra: Filtrado pulsador

Como ejercicio complementario, en este codigo se describe una interrupción por timer que se encarga de filtrar la activación de un pulsador.

Concretamente, durante intervalos de 10 milisegundos se revisa la situación del pulsador, en caso de dos cambios de estado consecutivos a voltaje alto, se activa una orden, que posteriormente, en el bucle principal, activiará un pequeña parte del codigo.

Cada cambio se define como la operacion XOR de del valor actual y del anterior, en caso que se detecte un cambio de estado, el cambio será cierto.

El cambio de estado actual se guarda en la variable "cama" que almacena el valor anterior de cambio de estado. Si en la siguiente iteración se detecta otro cambio de estado, se actualiza la orden a valor alto.

```c
#include <Arduino.h>

//Declaracion de variables para el timer
volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//Declaracion variables para el filtro
bool valor_act,valor_ant = 0,cambio_anterior = 0,cambio_actual,orden = 0;


//Codigo de la interrupcion TIMER
void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
    
    //Filtro
    valor_act = digitalRead(4);                       //Leemos valor del pin donde esta  connectado el pulsador

    cambio_actual = valor_ant ^ valor_act;            //Funcion XOR de valor anterior y actual

    if (cambio_actual == 1 && cambio_anterior == 1) //Si ambios cambios estan a nivel alto:
    {
      orden = 1;                                      //Ponemos "orden" en valor alto
      digitalWrite(18,valor_act);                     //Escribimos el valor por el pin 18 para ver cuando se efectua
                                                      // la pulsacion
      valor_ant = valor_act;                          //Guardamos el nuevo valor en el valor anterior                                  
      cambio_anterior = 0;                            //Ponemos el cambio anterior a 0
      return;                                         //Salimos del timer
    }
    cambio_anterior = cambio_actual;                  //En caso contrario guardamos el cambio en la variable cambio anterior
}

//Codigo que solo se ejecuta al inicio
void setup() 
{
    Serial.begin(115200);                             //Definimos baud rate
    timer = timerBegin(0, 80, true);                  //Iniciamos el timer
    timerAttachInterrupt(timer, &onTimer, true);      //Relacionamos el timer a la funcion
    timerAlarmWrite(timer, 10000, true);              
    timerAlarmEnable(timer);
    pinMode(18,OUTPUT);                               //Definimos el pin 18 como salida
    pinMode(4,INPUT_PULLUP);                          //Definimos el pin 4 como entrada "pull up"(Necesaria en pulsador)
}

//Codigo que se ejecuta en bucle
void loop() 
{
    if (interruptCounter > 0)                         //
    {
        portENTER_CRITICAL(&timerMux);                //
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);
        totalInterruptCounter++;
        if (orden)                                    //Codigo que se ejecuta si se recibe la orden
        {
          Serial.println("Se ha pulsado en boton!");  //Escribimos mensaje de confirmacion
          orden = 0;                                  //Una vez ejecutado, reiniciamos la variable
        }
    }
}
```

Como resultado de este codigo, podemos ver que ante los rebotes, el codigo lo lee como una única pulsacion:
![Filtrado](https://github.com/gerardcotsescude/P2-morera-cots.git/blob/main/vlcsnap-2023-03-03-01h30m53s959.png?raw=true)