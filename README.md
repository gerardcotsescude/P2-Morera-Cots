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
Button 1 has been pressed 1606 times
Interrupt Detached!
```

Debido a que el en laboratorio no disponiamos de pulsadores, hemos simulado el puslador usando un cortocircuito, con lo cual aparecen rebotes, disparando el numero de veces que se presiona el pulsador.

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
```
