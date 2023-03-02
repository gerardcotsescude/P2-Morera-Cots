#include <Arduino.h>

volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
bool IN,vact,vant = 0,cama = 0,cam,orden = 0;

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
    
    
    //Filter

    vact = digitalRead(4);

    cam = vant ^ vact;

    if (cam == 1 && cama == 1)
    {
      IN = digitalRead(18);
      vant = vact;
      orden = 1;
      cama = 0;
      digitalWrite(18,vact);
      return;
    }
    cama = cam;
}

void setup() 
{
    Serial.begin(115200);
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 10000, true);
    timerAlarmEnable(timer);
    pinMode(18,OUTPUT);
    pinMode(4,INPUT_PULLUP);
}

void loop() 
{
    if (interruptCounter > 0)
    {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);
        totalInterruptCounter++;
        
        //Serial.print("An interrupt has occurred. Total number: ");
        //Serial.println(totalInterruptCounter);
        if (orden)
        {
          Serial.println("boton!");
          orden = 0;
        }
    }
}