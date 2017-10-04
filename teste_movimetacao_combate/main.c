#include <msp430g2253.h>


/***************************************************************************
 * Título: Código de Controle de movimentação para robôs
 * de combate.
 *
 * Descrição: Consiste em uma opção de algorítmo que pode ser implementado
 * no controle de movimentação, em coordenadas cartesianas, de robôs que
 * possuem pelomenos 1 motor em cada lado. A movimentação e lógica utili-
 * zadas baseiam-se no controle de apenas por um Joystick. Com a opção do
 * "Fail-safe", para o caso de perdas de sinal com o rádio controle.
 *
 * Autores:
 *
 *- Daniel Carvalho.
 *- Guilherme Felix.
 *
 * Data: 22/08/2017
 * Modificado em: 23/09/2017 por Daniel Carvalho.
 *
 * - Eletrônica de Combate - Equipe de competições TITANS® - UnB-Gama
 * **************************************************************************
      A ideia inicial era deixar a CPU desligada para fazer a leitura dos 2
     sinais, porém a interrupção impossibilitou a leitura simultânea dos ca-
     nais, além de dificultar a detecção de falhas.
     A interrupção iria ser chamada 2 vezes para cada leitura, visto que
     precisamos determinar o inicio e fim do pulso. Ao fim da leitura, a
     interrupção "ligaria" a CPU denovo para o processamento.

     OBS: Devemos garantir que o processamento seja mais rápido que a fre-
     quencia do sinal. A interrupção, ou qualquer método de aquisição de da-
     dos não deve "atrapalhar" o processamento no sentido de durar mais que 1
     período do sinal de entrada.
 ************************************************************************/
// PWM Right 1 (P2.1)
// PWM Left (P2.4)
// Input channel 1 (P1.1)
// Input channel 2 (P1.2)
// Enables (P1.X):
#define left_forward BIT0 // inputs para a ponte h
#define left_reverse BIT3
#define right_forward BIT4
#define right_reverse BIT5

#define R TA1CCR1    // PWM do canal 1 (P2.1)
#define L TA1CCR2    // PWM do canal 2 (P2.4)

// PWM do canal 1 (P2.1)
// PWM do canal 2 (P2.4)
// Input channel 1 (P1.1)
// Input channel 2 (P1.2)
// Variáveis globais
//unsigned int T0 = 0;
unsigned int Pulse_1 = 0;
unsigned int Pulse_2 = 0; // Variáveis de aquisição de dados
int coord_X = 0,coord_Y = 0; // Valores das coordenadas no algorítmo.


// Funções
void Fail_safe(void){
    Pulse_1 = 1500;
    Pulse_2 = 1500;
}
long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Pulse_read(void){
    TA0CCTL0 = CAP + SCS + CCIS_0 + CM_3; // Seta o TA0.0 para operar no modo captura de bordas de subida ou descida.
    while ((TA0CCTL0 & CCIFG) == 0);      // Enquando não ocorrer a captura no canal 1, a função ficará travada nessa linha.
    if ((P1IN & BIT1) != 0){              // Analisa se a captura que ocorreu foi de borda de subida.
        //T0 = 0;
        TA0CCTL0 &= ~CCIFG;               // Limpa a flag de captura.
        TA0R = 0;
        TA0CTL = TASSEL_2 + MC_2;         // Ajusta as configurações do timer e inicia o contador no modo contínuo (até 0xFF).
        while((TA0CCTL0 & CCIFG) == 0);   // espera a segunda captura do canal 1.
        Pulse_1 = TA0CCR0;                // armazena o valor lido do primeiro canal em "ms" na variável Pulse_1.
        TA0R = 0;                         // Zera o contador.
        TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3;   // Configura o timer TA0.1 para as mesmas configurações do timer TA0.0.
        while ((TA0CCTL1 & CCIFG) == 0);  // Aguarda a captura do canal 2.
        Pulse_2 = TA0CCR1;                // Armazena o valor do canal 2, em "ms".
    }
    if(Pulse_1 < 900 || Pulse_2 < 900 || Pulse_1 > 2100 || Pulse_2 > 2100)
        Fail_safe();
    //return T0;
}
// Configurações principais encontram-se na main
int main(void) {

    // Watchdog timer e clock Set-Up
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;  // A configuração dos timers leva em conta o clock da CPU.(1MHz)
    DCOCTL = CALDCO_1MHZ;   // A mudança nessas definições altera o comportamento do código.
    P1DIR = 0xFF;   // Define todas as portas inutilizadas como saídas.

    // PWM output Set-Up
    P2DIR |= BIT1 | BIT4; // P2.1 e P2.4 como saidas digitais (PWM)
    P2SEL |= BIT1 | BIT4;

    // Timer1 Set-Up (PWM)
    TA1CCR0 = 2000-1;       // Periodo do PWM (F = 1kHz)
    TA1CCTL1 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.1)
    TA1CCTL2 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.2)
    TA1CTL |= TASSEL_2 + MC_1;
    TA1CCR1 = 0; // PWM duty cycle 1 (deve mudar via software).
    TA1CCR2 = 0; // PWM duty cycle 2 - CCR1/(CCR0+1) * 100

    // Capture input Set-Up
    P1DIR &= ~(BIT1 + BIT2);    // Define os pinos de captura como entradas.
    P1SEL |= BIT1 | BIT2;       // Utilizando as funções secundárias do pino.
    //P2SEL &= ~ (BIT1 + BIT2);
    P1REN &= ~(BIT1 + BIT2);    // Seta os resistores de pullup/down.
    P1OUT |= (BIT1 + BIT2);     // Define os resistores de pullup;


    // Timer0 Set-Up (Capture)
    TA0CTL = TASSEL_2 + MC_3;   // Modo contínuo de contagem e sync com SMclock.
    // captura up/down + sync com SM clock + interrupt enable P1.1.
    //TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3; // captura up/down + sync com SM clock + interrupt enable P1.2.
    //__bis_SR_register(CPUOFF + GIE); // Habilitando interrupção e aguardando a leitura dos sinais.

    for(;;){

        Pulse_read();

        coord_X = map((long)Pulse_1,1000,2000,-1550,1550);
        coord_Y = map((long)Pulse_2,1000,2000,-1550,1550);

        if(abs(coord_X) < 20 && abs(coord_Y) < 20){ // --> Nessa configuração, valores entre os intervalos(-20 -> 20) serão considerados como Zero.
          L = 0;                                    // Isso foi feito para desligar os dois motores quando a alavanda estiver na "posição 0",
          R = 0;                                    // visto que o sinal sofre variação de até -8 a 8 na posição inicial do "joystick".
          P1OUT &= ~(left_reverse + left_forward + right_forward + right_reverse);

        }else if(abs(coord_X) >= 20 || abs(coord_Y) >= 20){
          if(coord_X < 0 && coord_Y < 0 && coord_X >= coord_Y ){ // 3° quadrante(6°região no mapa).
            L = abs(coord_Y - coord_X);
            R = abs(coord_Y);
            P1OUT |= (right_reverse + left_reverse);
            P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X < 0 && coord_Y < 0 && coord_X < coord_Y){ // 3° quadrante(5° região do mapa).
            R = abs((-coord_X) + coord_Y);
            L = abs(coord_X - coord_Y);
            P1OUT |=(left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y >= - coord_X)){ // 2° quadrante (3° região do mapa).
            R = coord_Y;
            L = (coord_Y + coord_X);
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y < - coord_X)){ // 2° quadrante (4° região do mapa).
            R = (- coord_X);
            L = abs(coord_Y + coord_X);
            P1OUT |= (left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
           }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y >= coord_X)){ // 4° quadrante (7° região do mapa).
           R = abs(coord_Y  + coord_X);
           L = abs(coord_Y);
           P1OUT |= (right_reverse + left_reverse);
           P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y < coord_X)){ // 4° quadrante (8° região do mapa).
            R = abs((-coord_X) - coord_Y);
            L = (coord_X + coord_Y);
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }else if(coord_X > 0 && coord_Y > 0 && coord_Y  >= coord_X){ // 1° quadrante (2° região do mapa).
            R = (coord_Y - coord_X);
            L = coord_Y;
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X > 0 && coord_Y > 0 && (coord_Y < coord_X)){ // 1° quadrante (1° região do mapa).
            R = abs(coord_Y - coord_X);
            L = coord_X;
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }
        }

    }
}
