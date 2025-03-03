#include <xc.h>
#include "header.h"
#define _XTAL_FREQ 48000000

#define FREQ_S 784  // Frecuencia deseada en Hz (LA = 440 Hz)
#define FREQ_A4 440    // Nota La
#define FREQ_C5 523    // Nota Do
#define FREQ_E5 659    // Nota Mi

// Definiciones de segmentos
#define DISPLAY_0 0x3F
#define DISPLAY_1 0x06
#define DISPLAY_2 0x5B
#define DISPLAY_3 0x4F
#define DISPLAY_4 0x66
#define DISPLAY_5 0x6D
#define DISPLAY_6 0x7D
#define DISPLAY_7 0x07
#define DISPLAY_8 0x7F
#define DISPLAY_9 0x6F

// Arreglo para los números decimales
unsigned char display[10] = {
    DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3, DISPLAY_4,
    DISPLAY_5, DISPLAY_6, DISPLAY_7, DISPLAY_8, DISPLAY_9
};

// Reloj
// Variables para almacenar el tiempo en formato decimal
unsigned char segundos_unidadesR = 0, segundos_decenasR = 0;
unsigned char minutos_unidadesR = 0, minutos_decenasR = 0;
unsigned char horas_unidadesR = 0, horas_decenasR = 0;  

//Cronometro
// Variables para almacenar el tiempo en formato decimal
unsigned char segundos_unidadesCRONO = 0, segundos_decenasCRONO = 0;
unsigned char minutos_unidadesCRONO = 0, minutos_decenasCRONO = 0;
unsigned char horas_unidadesCRONO = 0, horas_decenasCRONO = 0;

//Temporizador
// Variables para almacenar el tiempo en formato decimal
unsigned char segundos_unidadesTEMPO = 0, segundos_decenasTEMPO = 0;
unsigned char minutos_unidadesTEMPO = 0, minutos_decenasTEMPO = 0;
unsigned char horas_unidadesTEMPO = 0, horas_decenasTEMPO = 0;

// Variables Sistema
unsigned char state = 1;
unsigned char set_start = 0;
unsigned char up_reset = 0;
unsigned char down_pausa = 0;
unsigned char antirrebote = 0;

volatile unsigned int millis = 0;  // Contador de milisegundos
volatile unsigned int sleep = 0;  // Contador de milisegundos
volatile unsigned int delayCount1 = 0;  // Contador de milisegundos
volatile unsigned int delayCount2 = 0;  // Contador de milisegundos
unsigned char banderaSleep = 1;


// Variables Reloj
unsigned char modoSetR = 0;
unsigned char inicioReloj = 1;
// Variable Crono
unsigned char modoPausaCrono = 1;
unsigned char inicioCrono = 1;
// Variable Tempo
unsigned char modoSetT = 0;
unsigned char inicioTempo = 1;
unsigned char banderaDecremento = 0;
unsigned char modoPausaTempo = 1;

// Alarma
unsigned char banderaAlarma = 0;
unsigned char alarm = 0;

// Funcion Antirebote
void antiR(){
    antirrebote = 1500;
}

void delay1(unsigned int count1){
    delayCount1 = count1;
}
void delay2(unsigned int count2){
    delayCount2 = count2;
}

void manejarDisplay(unsigned char multDisp, unsigned char modoSet, unsigned char modoSetT, unsigned char set_start, unsigned char set_startComparador,
                            unsigned char state, unsigned int delayCount2, unsigned char *TiempoR, 
                            unsigned char *TiempoC, unsigned char *TiempoT, 
                            unsigned char *display) {
    
    // Condiciones para mostrar el display según el modo y estado
    if ((modoSet == 1 && set_start == set_startComparador && state == 1) ||  // Modo SetR
        (modoPausaCrono == 1 && state == 2) ||                  // Modo Pausa Crono
        (modoSetT == 1 && set_start == set_startComparador && state == 3) ||  // Modo SetT
        (modoPausaTempo == 1 && state == 3)) {                  // Modo Pausa Tempo
        
        if (delayCount2 < 800) {
            //LATD = multDisp;  // Activar el display de segundos (unidades)
            LATD = (LATD & 0b11000000) | (multDisp & 0b00111111);

        }
    } else {
        //LATD = multDisp;  // Activar el display de segundos (unidades) por defecto
        LATD = (LATD & 0b11000000) | (multDisp & 0b00111111);
    }

    // Mostrar los segundos correctos según el estado
    if (state == 1) {
        LATB = display[*TiempoR];  // Segundos reloj
    } else if (state == 2) {
        LATB = display[*TiempoC];  // Segundos cronómetro
    } else if (state == 3) {
        LATB = display[*TiempoT];  // Segundos temporizador
    }
}

// Manejo de interrupciones
void __interrupt() ISR() {
    static unsigned char current_digit = 0;
    unsigned char ComparadorSeg = 3;
    unsigned char ComparadorMin = 2;
    unsigned char ComparadorHor = 1;
    unsigned char segundos_unidad = 0b000001;
    unsigned char segundos_decena = 0b000010;
    unsigned char minutos_unidad = 0b000100;
    unsigned char minutos_decena = 0b001000;
    unsigned char horas_unidad = 0b010000;
    unsigned char horas_decena = 0b100000;
    
    //Alarma Timer 2
    if (PIR1bits.TMR2IF) //Si se activa la interrupción del Timer1
    {
        //Alarma
        if(banderaAlarma){
            LATDbits.LATD6 = ~LATDbits.LATD6;  // Invertir el estado del pin RD6 (onda cuadrada)
        }
        // Limpiar la bandera de interrupción por cambio de estado
        PIR1bits.TMR2IF = 0; // Limpiar la bandera de interrupción
    }    
    
    //Display Timer 1
    if (PIR1bits.TMR1IF) {  // Si se activa la interrupción del Timer1
        PIR1bits.TMR1IF = 0; // Limpiar la bandera de interrupción
        TMR1 = 65036;   // Recargar el Timer para el próximo 1 ms
        
    // Modo, Set/Start, Up/Reset, Down/Pausa
    if (antirrebote == 0) {
        if (PORTAbits.RA0 == 1) { antiR(); state = (state + 1) % 4; }
        else if (PORTAbits.RA1 == 1) { antiR(); set_start = (set_start + 1) % 4; }
        else if (PORTAbits.RA2 == 1) { antiR(); up_reset++; }
        else if (PORTAbits.RA3 == 1) { antiR(); down_pausa++; }
    }
                
        LATD = 0;  // Apagar todos los displays antes de cambiar
        if(state != 0){
            switch(current_digit) {
            case 0:
                // Display Segundos Unidades
                manejarDisplay(segundos_unidad, modoSetR, modoSetT, set_start, ComparadorSeg, state, delayCount2, 
                       &segundos_unidadesR, &segundos_unidadesCRONO, &segundos_unidadesTEMPO, display);
                break;
            case 1:
                // Display Segundos Decenas
                manejarDisplay(segundos_decena, modoSetR, modoSetT, set_start, ComparadorSeg, state, delayCount2, 
                       &segundos_decenasR, &segundos_decenasCRONO, &segundos_decenasTEMPO, display);
                break;
            case 2:
                // Display Minutos Unidades
                manejarDisplay(minutos_unidad, modoSetR, modoSetT, set_start, ComparadorMin, state, delayCount2, 
                       &minutos_unidadesR, &minutos_unidadesCRONO, &minutos_unidadesTEMPO, display);
                break;
            case 3:
                // Display Minutos Decenas
                manejarDisplay(minutos_decena, modoSetR, modoSetT, set_start, ComparadorMin, state, delayCount2, 
                       &minutos_decenasR, &minutos_decenasCRONO, &minutos_decenasTEMPO, display);
                break;
            case 4:
                // Display Horas Unidades
                manejarDisplay(horas_unidad, modoSetR, modoSetT, set_start, ComparadorHor, state, delayCount2, 
                       &horas_unidadesR, &horas_unidadesCRONO, &horas_unidadesTEMPO, display);
                break;
            case 5:
                // Display Horas Decenas
                manejarDisplay(horas_decena, modoSetR, modoSetT, set_start, ComparadorHor, state, delayCount2, 
                       &horas_decenasR, &horas_decenasCRONO, &horas_decenasTEMPO, display);
                break;
            }

            current_digit++;  // Cambiar al siguiente display
            if (current_digit > 5) {
                current_digit = 0;  // Reiniciar después del último display
            }
        }
        millis++;            // Incrementar contador de milisegundos
        sleep++;
        delayCount1--;
        delayCount2++;
        antirrebote--;
    }
}

// Alarma
void ring(unsigned int frequency) {
    // Configurar Timer2 para generar la frecuencia deseada
    PR2 = (_XTAL_FREQ / (4 * frequency)) - 1;  // Cálculo para PR2 basado en la frecuencia deseada
    T2CON = 0b00000100;  // Timer2 habilitado, prescaler 1:1
    TMR2 = 0;  // Reinicia el timer
}

//TimerAlarma
void setup_timer2(void) {
    T2CONbits.TMR2ON = 0;
    T2CON = 0b00000100;  // Timer2 habilitado, prescaler 1:1 
    TMR2IE = 1;  // Habilitar interrupción de Timer2
    PEIE = 1;  // Habilitar interrupciones periféricas
    GIE = 1;  // Habilitar interrupciones globales
    T2CONbits.TMR2ON = 1;  // Activa el Timer2
}

// Reloj
void setup_timer1(void){
    // Configurar Timer 1
    T1CONbits.TMR1CS = 0;  // Temporizador interno
    T1CONbits.T1CKPS = 0b11; // Prescaler 1:8 (ajusta según necesidad)
    TMR1 = 65036;               // Valor para 1 ms (Fosc/4)

    // Habilitar interrupciones de Timer 1
    PIE1bits.TMR1IE = 1;   // Habilitar interrupción del Timer 1
    INTCONbits.PEIE = 1;   // Habilitar interrupciones periféricas
    INTCONbits.GIE = 1;    // Habilitar interrupciones globales

    // Iniciar Timer 1
    T1CONbits.TMR1ON = 1;  // Encender el Timer 1
}

//Testeo LEDs Verde, Amarillo y Rojo 
void status(){
    LATC = 0b000;  // Valor por defecto para state == 0

    if (state == 1) {
        LATC = (inicioReloj == 1) ? 0b001 : (modoSetR == 1 ? 0b010 : 0b100);
    } else if (state == 2) {
        LATC = (inicioCrono == 1) ? 0b001 : (modoPausaCrono == 1 ? 0b010 : 0b100);
    } else if (state == 3) {
        LATC = (inicioTempo == 1) ? 0b001 : ((modoPausaTempo == 1 || modoSetT == 1) ? 0b010 : 0b100);
    }
}


void relojDecrementoHora(unsigned char *horas_unidades, unsigned char *horas_decenas) {
    // Decrementar unidades de horas
    if (*horas_unidades == 0) {
        *horas_unidades = 9;  // Reiniciar a 9 si llega a 0
        
        // Decrementar decenas de horas
        if (*horas_decenas == 0) {
            // Si las horas llegan a 00, reiniciarlas a 23
            *horas_decenas = 2;
            *horas_unidades = 3;
        } else {
            *horas_decenas -= 1;

            // Ajuste para evitar horas fuera de rango (19, 09, etc.)
            if (*horas_decenas == 1 || *horas_decenas == 0) {
                *horas_unidades = 9;
            }
        }
    } else {
        // Decremento directo si las unidades son mayores que 0
        *horas_unidades -= 1;
    }
}

void relojDecrementoMinutoSegundo(unsigned char *unidades, unsigned char *decenas) {
    // Reducir de 00 a 59
    if (*unidades == 0 && *decenas == 0){
        *unidades = 9;
        *decenas = 5;
        return;
    }
    // Decrementar las unidades de los minutos
    if (*unidades == 0) {
        *unidades = 9;  // Reiniciar las unidades a 9 si llegan a 0

        // Decrementar las decenas de los minutos
        if (*decenas == 0) {
            // Si las decenas también son 0, ya estamos en 00, así que no decrementamos más
            *unidades = 0;  // Aseguramos que minutos_unidades también sea 0
        } else {
            // Decrementamos las decenas
            *decenas = *decenas - 1;
        }
    } else {
        // Si las unidades no son 0, simplemente las decrementamos
        *unidades = *unidades - 1;
    }
}


// Ajuste incremento
void relojAjuste(unsigned char *segundos_unidades, unsigned char *segundos_decenas,
                 unsigned char *minutos_unidades, unsigned char *minutos_decenas,
                 unsigned char *horas_unidades, unsigned char *horas_decenas) {
    
    if (*segundos_unidades == 10) {
        *segundos_unidades = 0;
        (*segundos_decenas)++;
    }

    // Verificar si el valor de segundos llega a 60
    if (*segundos_decenas == 6 && *segundos_unidades == 0) {
        *segundos_decenas = 0;

        // Incrementar minutos (unidades)
        (*minutos_unidades)++;
    }

    if (*minutos_unidades == 10) {
        *minutos_unidades = 0;
        (*minutos_decenas)++;
    }

    // Verificar si el valor de minutos llega a 60
    if (*minutos_decenas == 6 && *minutos_unidades == 0) {
        *minutos_decenas = 0;

        // Incrementar horas (unidades)
        (*horas_unidades)++;
    }

    // Verificar si el valor de horas llega a 24
    if (*horas_unidades == 10) {
        *horas_unidades = 0;
        (*horas_decenas)++;
    }

    if (*horas_decenas == 2 && *horas_unidades == 4) {
        // Resetear el reloj a 00:00:00
        *horas_unidades = 0;
        *horas_decenas = 0;
        *minutos_unidades = 0;
        *minutos_decenas = 0;
        *segundos_unidades = 0;
        *segundos_decenas = 0;
    }
}

void temporizadorDecremento(unsigned char *segundos_unidades, unsigned char *segundos_decenas,
                            unsigned char *minutos_unidades, unsigned char *minutos_decenas,
                            unsigned char *horas_unidades, unsigned char *horas_decenas) {
    // Comprobar si el temporizador está en 00:00:00
    if (*segundos_unidades == 0 && *segundos_decenas == 0 &&
        *minutos_unidades == 0 && *minutos_decenas == 0 &&
        *horas_unidades == 0 && *horas_decenas == 0) {
        banderaDecremento = 1; // Levantar bandera cuando llega a 00:00:00
        return; // Salir de la función ya que no hay más decrementos que realizar
    }

    // Decrementar las unidades de los segundos
    if (*segundos_unidades == 0) {
        *segundos_unidades = 9;

        // Decrementar las decenas de los segundos
        if (*segundos_decenas == 0) {
            *segundos_decenas = 5;

            // Decrementar unidades de minutos si segundos llegan a 00
            if (*minutos_unidades == 0) {
                *minutos_unidades = 9;

                // Decrementar decenas de minutos si minutos están en 00
                if (*minutos_decenas == 0) {
                    *minutos_decenas = 5;

                    // Decrementar unidades de horas si minutos están en 00
                    if (*horas_unidades == 0) {
                        *horas_unidades = 9;

                        // Decrementar decenas de horas si horas están en 00
                        if (*horas_decenas == 0) {
                            *horas_decenas = 2;  // Reiniciar en 23 horas
                            *horas_unidades = 3;
                        } else {
                            *horas_decenas -= 1;
                        }
                    } else {
                        *horas_unidades -= 1;
                    }
                } else {
                    *minutos_decenas -= 1;
                }
            } else {
                *minutos_unidades -= 1;
            }
        } else {
            *segundos_decenas -= 1;
        }
    } else {
        *segundos_unidades -= 1;
    }
}


void reloj() {
    // Verifica si estamos en modo "Reloj"
    if (state == 1) {
        // Modo set activado o apagado según set_start
        if (set_start > 0) {
            modoSetR = 1;   // Activa modo de configuración
            inicioReloj = 0;
        } else {
            modoSetR = 0;   // Continua el reloj
        }

        // Reinicio o ajustes según up_reset
        if (up_reset > 0) {
            if (modoSetR == 0) {
                // Reinicia el reloj
                segundos_unidadesR = segundos_decenasR = minutos_unidadesR = minutos_decenasR = 0;
                horas_unidadesR = horas_decenasR = 0;
                inicioReloj = 1;
            } else {
                // Modo set activado: ajustes según set_start
                switch (set_start) {
                    case 1:
                        horas_unidadesR++;
                        break;
                    case 2:
                        minutos_unidadesR++;
                        break;
                    case 3:
                        segundos_unidadesR++;
                        break;
                }
                relojAjuste(&segundos_unidadesR, &segundos_decenasR, 
                            &minutos_unidadesR, &minutos_decenasR, 
                            &horas_unidadesR, &horas_decenasR);
            }
            up_reset = 0;  // Limpieza
        }

        // Pausa o decremento según down_pausa
        if (down_pausa > 0) {
            if (modoSetR) {
                switch (set_start) {
                    case 1:
                        relojDecrementoHora(&horas_unidadesR, &horas_decenasR);
                        break;
                    case 2:
                        relojDecrementoMinutoSegundo(&minutos_unidadesR, &minutos_decenasR);
                        break;
                    case 3:
                        relojDecrementoMinutoSegundo(&segundos_unidadesR, &segundos_decenasR);
                        break;
                }
            }
            down_pausa = 0;  // Limpieza
        }
    }
    
    // Incremento de segundos si no está en modo set y el reloj está activo
    if (!modoSetR && !inicioReloj) {
        segundos_unidadesR++;
        relojAjuste(&segundos_unidadesR, &segundos_decenasR, 
                    &minutos_unidadesR, &minutos_decenasR, 
                    &horas_unidadesR, &horas_decenasR);
    }
}

// Cronometro
void cronometro() {
    if (state == 2) {
        // Configuración de modo y reinicio del cronómetro
        if (set_start > 0) {
            modoPausaCrono = 0;
            set_start = inicioCrono = 0;
        }
        if (up_reset > 0) {
            // Reset de tiempo e inicio del cronómetro
            segundos_unidadesCRONO = segundos_decenasCRONO = minutos_unidadesCRONO = 0;
            minutos_decenasCRONO = horas_unidadesCRONO = horas_decenasCRONO = 0;
            inicioCrono = modoPausaCrono = 1;
            up_reset = 0;  // Limpiar
        }
        if (down_pausa > 0) {
            modoPausaCrono = 1;  // Pausar cronómetro
            down_pausa = 0;      // Limpiar
        }
    }
    // Ejecutar cronómetro si no está en pausa
    if (!modoPausaCrono) {
        segundos_unidadesCRONO++;
        relojAjuste(&segundos_unidadesCRONO, &segundos_decenasCRONO,
                    &minutos_unidadesCRONO, &minutos_decenasCRONO,
                    &horas_unidadesCRONO, &horas_decenasCRONO);
    }
}

// Temporizador
void temporizador() {
    // Temporizador
    if (state == 3) {
        modoSetT = (set_start > 0) ? 1 : 0; 
        if (modoSetT) { modoPausaTempo = inicioTempo = 0; }

        // Desactivar modo set 4 pulsacion
        if (up_reset > 0) {
            if (!modoSetT) { // Modo set apagado: reinicia el temporizador
                banderaDecremento = alarm = banderaAlarma = 0;
                inicioTempo = 1;
                segundos_unidadesTEMPO = segundos_decenasTEMPO = minutos_unidadesTEMPO = minutos_decenasTEMPO = 0;
                horas_unidadesTEMPO = horas_decenasTEMPO = 0;
            } else { // Modo set activado: ajustar horas, minutos o segundos
                if (set_start == 1) horas_unidadesTEMPO++;
                else if (set_start == 2) minutos_unidadesTEMPO++;
                else if (set_start == 3) segundos_unidadesTEMPO++;
                relojAjuste(&segundos_unidadesTEMPO, &segundos_decenasTEMPO,
                            &minutos_unidadesTEMPO, &minutos_decenasTEMPO,
                            &horas_unidadesTEMPO, &horas_decenasTEMPO);
            }
            up_reset = 0; // Limpiar
        }

        if (down_pausa > 0) {
            if (!modoSetT) { // Modo set apagado: pausar o reanudar
                modoPausaTempo = !modoPausaTempo;
            } else { // Modo set activado: decremento de horas, minutos o segundos
                if (set_start == 1) relojDecrementoHora(&horas_unidadesTEMPO, &horas_decenasTEMPO);
                else if (set_start == 2) relojDecrementoMinutoSegundo(&minutos_unidadesTEMPO, &minutos_decenasTEMPO);
                else if (set_start == 3) relojDecrementoMinutoSegundo(&segundos_unidadesTEMPO, &segundos_decenasTEMPO);
            }
            down_pausa = 0; // Limpiar
        }
    }

    // Finalización del temporizador y decremento automático
    if (!modoSetT && !modoPausaTempo && !inicioTempo) {
        alarm = (banderaDecremento == 1) ? 1 : 0;
        if (!alarm) {
            temporizadorDecremento(&segundos_unidadesTEMPO, &segundos_decenasTEMPO,
                                   &minutos_unidadesTEMPO, &minutos_decenasTEMPO,
                                   &horas_unidadesTEMPO, &horas_decenasTEMPO);
        }
    }
}


// Alarma tono
void cancion() {
    if (sleep == 0 && !banderaAlarma) {
        ring(FREQ_A4);
        banderaAlarma = 1;
    } else if (sleep == 300) {
        ring(FREQ_C5);
    } else if (sleep == 600) {
        ring(FREQ_E5);
    } else if (sleep == 900) {
        ring(FREQ_A4);
    } else if (sleep == 1400 || sleep == 1650) {
        banderaAlarma = (sleep == 1400) ? 0 : banderaAlarma;
        sleep = (sleep == 1650) ? 0 : sleep;
    }
}

void setup(void){
    // Inicializar el reloj a 00:00:00
    horas_unidadesR = 0;    
    horas_decenasR = 0;     
    minutos_unidadesR = 0;  
    minutos_decenasR = 0;   
    segundos_unidadesR = 0;  
    segundos_decenasR = 0;  
    // Inicializar el Crono a 00:00:00
    horas_unidadesCRONO = 0;    
    horas_decenasCRONO = 0;     
    minutos_unidadesCRONO = 0;  
    minutos_decenasCRONO = 0;   
    segundos_unidadesCRONO = 0;  
    segundos_decenasCRONO = 0;  
    // Inicializar el Tempo a 00:01:00
    segundos_unidadesTEMPO = 0;
    segundos_decenasTEMPO = 0;
    minutos_unidadesTEMPO = 0;
    minutos_decenasTEMPO = 0;
    horas_unidadesTEMPO = 0;
    horas_decenasTEMPO = 0;
}

void modo() {
    // Actualizar modos de acuerdo al estado
    modoSetR = (state == 1) ? modoSetR : 0;
    modoSetT = (state == 3) ? modoSetT : 0;

    // Leds por estado
    if (state == 0) {
        // Apagar todos los LEDs y resetear parámetros
        LATE = 0;
        if (banderaAlarma) T2CONbits.TMR2ON = 0;  // Apagar alarma
        set_start = up_reset = down_pausa = 0;
    } else {
        // Configurar LED según estado
        LATE = 1 << (state - 1);
    }
}


void main(void) {
    // Configurar puertos como salidas
    TRISC = 0x00;
    TRISB = 0x00;  
    TRISD = 0x00;  
    TRISE = 0x00;  
    
    LATC = 0x00;
    LATB = 0x00;
    LATD = 0x00;
    LATE = 0x00;

    // Desactivar entradas analógicas en RA0, RA1, RA2 y RA3
    PORTA = 0;
    ADCON1 = 0b00001111;// Puertos digitales          
    CVRCON = 0b00000000;// Desactivar CVREF
    CMCON = 0b00001111; // Desactivar comparadores
    UCON = 0b00000000;  // Desactivar USB
    
    // Configurar RA0, RA1, RA2 y RA3 como entradas
    TRISAbits.TRISA0 = 1;  
    TRISAbits.TRISA1 = 1;  
    TRISAbits.TRISA2 = 1;  
    TRISAbits.TRISA3 = 1;  
    // Habilitar la interrupción por cambio de estado en el puerto A (RA0-RA4)
    INTCONbits.RBIE = 1;   // Habilitar interrupción por cambio de estado en puerto B
    INTCONbits.RBIF = 0;   // Limpiar bandera de interrupción de puerto B
    INTCONbits.GIE = 1;    // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;   // Habilitar interrupciones periféricas

    // Encender RB7 permanentemente (punto)
    //LATBbits.LATB7 = 1;  
    //TRISDbits.TRISD7 = 0;  // Configurar RD6 como salida

    // Debug
    TRISAbits.TRISA4 = 0;  // Configurar RD7 como salida
    LATAbits.LATA4 = 0;    // Inicializar en bajo
    
    // Setup relojs
    setup();
    
    // Alarma
    TRISDbits.TRISD6 = 0;  // Configurar RD6 como salida
    setup_timer1();// Configurar el Timer1 para generar la señal
    setup_timer2();

    while(1) {
        //Contadores
        if (sleep > 1700){
            sleep = 0;
        }
        if (delayCount2 >= 1000){
            delayCount2 = 0;
        }
        //__delay_us(200);  // Tiempo entre ciclos de multiplexado
        
        // Funciones Segundo Plano
        // Verificar el tiempo transcurrido
        if (millis >= 1000) {  // Si ha pasado 1000 ms = 1segundo 
            // Realiza la acción deseada, como encender un LED
            millis = 0;        // Reiniciar contador
            reloj();
            cronometro();
            temporizador();
            // Debug
            // Alternar el estado del pin RD7 para depuración
            LATAbits.LATA4 = ~LATAbits.LATA4;
        }
        
        // Alarma
        if(alarm){
            cancion();
        }
        
        
        // Led Control
        modo();
        status();
    }
}