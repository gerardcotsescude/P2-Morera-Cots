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
    portENTER_CRITICAL_ISR(&timerMux);                // Aumentamos el interruptor del contador dentro de una seccion critica
    interruptCounter++;                               //ya que la variable se comparte con el programa principal
    portEXIT_CRITICAL_ISR(&timerMux);
    
    //Filtro
    valor_act = digitalRead(4);                       //Leemos valor del pin donde esta  connectado el pulsador

    cambio_actual = valor_ant ^ valor_act;            //Funcion XOR de valor anterior y actual

    if (cambio_actual == 1 && cambio_anterior == 1)   //Si ambos cambios estan a nivel alto:
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
    Serial.begin(115200);                             //Velocidad de transmision
    timer = timerBegin(0, 80, true);                  //Iniciamos el timer
    timerAttachInterrupt(timer, &onTimer, true);      //Relacionamos el timer a la funcion
    timerAlarmWrite(timer, 10000, true);              //Especificar el valor del contador en el que se genera la interrupcion
    timerAlarmEnable(timer);                          //Habilitamos el temporizador
    pinMode(18,OUTPUT);                               //Definimos el pin 18 como salida
    pinMode(4,INPUT_PULLUP);                          //Definimos el pin 4 como entrada "pull up"(Necesaria en pulsador)
}

//Codigo que se ejecuta en bucle
void loop() 
{
    if (interruptCounter > 0)                         //Si el contador de interrupciones es mayor a 0
    {
        portENTER_CRITICAL(&timerMux);                //Como la variable se comparte con la funcion 
        interruptCounter--;                           //ISR decrementamos el contador dentro de una seccion critica
        portEXIT_CRITICAL(&timerMux);
        totalInterruptCounter++;                      //Aumentamos el numero total de interrupciones
        //En caso de orden a valor alto
        if (orden)                                    //Codigo que se ejecuta si se recibe la orden
        {
          Serial.println("Se ha pulsado en boton!");  //Escribimos mensaje de confirmacion
          orden = 0;                                  //Una vez ejecutado, reiniciamos la variable
        }
    }
}