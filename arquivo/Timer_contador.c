#include <msp430.h> 

/*
     This is a code designed to help engineers to
 calculate the time execution of a function,  ba-
 sed on the Capture event of timerA. Based on the
 datasheet of MSP340G2553 and the user guide pro-
 vided by Texas Instruments.

 Author: Daniel Carvalho de Sousa
 Date: 20/08/2017

 */
long int x0,y0;
int y = 100;
int x = 500;
unsigned int T0 = 0,enable = 0;
float Temp_exec = 0;

long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;  // select 1Mhz clock.
    DCOCTL = CALDCO_1MHZ;   // select the DCO clock.
	TA0CCTL0 = CAP + SCS + CCIS1 + CM_3 + CCIE; // both edges capture + sync with the clock + interrupt flag.
    __bis_SR_register(GIE); // set the global interruptions
	TA0CTL = TASSEL_2 + MC_2; // timer´s clock is system master clock, whithout division.
	TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
	TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer

	//---- Insert the code test here ! -----

	x0 = map(x, 272, 482, -255, 255);
	y0 = map(y, 272, 482, -255, 255);

	//------------Controle de coordenadas-------------
	// O funcionamento desse sistema de coordenadas, assemelha-s
/*	if(abs(coord_X) < 20 && abs(coord_Y) < 20){ // --> Nessa configuração, valores entre os intervalos(-20 -> 20) serão considerados como Zero.
	  L = 0;                                    // Isso foi feito para desligar os dois motores quando a alavanda estiver na "posição 0",
	  R = 0;                                    // visto que o sinal sofre variação de até -8 a 8 na posição inicial do "joystick".
	  digitalWrite(left_reverse,LOW);
	  digitalWrite(left_forward,LOW);
	  digitalWrite(right_reverse,LOW);
	  digitalWrite(right_forward,LOW);
	  digitalWrite(motor_r,LOW);
	  digitalWrite(motor_l,LOW);
	}else{
	  if(coord_X < 0 && coord_Y < 0 && coord_X >= coord_Y ){ // 3° quadrante(6°região no mapa).
	    analogWrite(motor_l, abs(coord_Y - coord_X));
	    analogWrite(motor_r, abs(coord_Y));

	    digitalWrite(right_reverse,HIGH);
	    digitalWrite(left_reverse,HIGH);
	    digitalWrite(right_forward,LOW);
	    digitalWrite(left_forward,LOW);
	  }else if(coord_X < 0 && coord_Y < 0 && coord_X < coord_Y){ // 3° quadrante(5° região do mapa).
	    analogWrite(motor_r, abs((-coord_X) + coord_Y));
	    analogWrite(motor_l, abs(coord_X - coord_Y));

	    digitalWrite(left_reverse,HIGH);
	    digitalWrite(right_forward,HIGH);
	    digitalWrite(left_forward,LOW);
	    digitalWrite(right_reverse,LOW);
	  }else if(coord_X < 0 && coord_Y > 0 && (coord_Y >= - coord_X)){ // 2° quadrante (3° região do mapa).
	    analogWrite(motor_r, coord_Y);
	    analogWrite(motor_l,(coord_Y + coord_X));

	    digitalWrite(right_forward,HIGH);
	    digitalWrite(left_forward,HIGH);
	    digitalWrite(right_reverse,LOW);
	    digitalWrite(left_reverse,LOW);
	  }else if(coord_X < 0 && coord_Y > 0 && (coord_Y < - coord_X)){ // 2° quadrante (4° região do mapa).
	    analogWrite(motor_r,(- coord_X));
	    analogWrite(motor_l, abs(coord_Y + coord_X));

	    digitalWrite(left_reverse,HIGH);
	    digitalWrite(right_forward,HIGH);
	    digitalWrite(left_forward,LOW);
	    digitalWrite(right_reverse,LOW);
	  }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y >= coord_X)){ // 4° quadrante (7° região do mapa).
	    analogWrite(motor_r, abs(coord_Y  + coord_X));
	    analogWrite(motor_l, abs(coord_Y));

	    digitalWrite(right_reverse,HIGH);
	    digitalWrite(left_reverse,HIGH);
	    digitalWrite(right_forward,LOW);
	    digitalWrite(left_forward,LOW);
	  }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y < coord_X)){ // 4° quadrante (8° região do mapa).
	    analogWrite(motor_r, abs((-coord_X) - coord_Y));
	    analogWrite(motor_l, (coord_X + coord_Y));

	    digitalWrite(left_forward,HIGH);
	    digitalWrite(right_reverse,HIGH);
	    digitalWrite(left_reverse,LOW);
	    digitalWrite(right_forward,LOW);
	  }else if(coord_X > 0 && coord_Y > 0 && coord_Y  >= coord_X){ // 1° quadrante (2° região do mapa).
	    analogWrite(motor_r, (coord_Y - coord_X));
	    analogWrite(motor_l, coord_Y);

	    digitalWrite(right_forward,HIGH);
	    digitalWrite(left_forward,HIGH);
	    digitalWrite(right_reverse,LOW);
	    digitalWrite(left_reverse,LOW);
	  }else if(coord_X > 0 && coord_Y > 0 && (coord_Y < coord_X)){ // 1° quadrante (1° região do mapa).
	    analogWrite(motor_r,abs(coord_Y - coord_X));
	    analogWrite(motor_l, coord_X);

	    digitalWrite(left_forward,HIGH);
	    digitalWrite(right_reverse,HIGH);
	    digitalWrite(left_reverse,LOW);
	    digitalWrite(right_forward,LOW);
	  }
	}
	*/
	//--------------------------------------
	TA0CCTL0 ^= CCIS0;  // Last interrupt request (final capture condition)
	Temp_exec *= 0.001;
	//__bis_SR_register(LPM4_bits);
	return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    TA0CTL &= ~MC_3;    // Stop the timer (holds its value until the start)
    if(enable == 0){
        enable ++;
        T0 = TA0CCR0 + 9; //reason: calling an interrupt and "stop timer command" demand 9 clock cycles.
    }else{
        enable = 0;
        Temp_exec = TA0CCR0 - T0 - 4; // 4 clocks demanded of the interrupt function.
    }
    TA0CCTL0 &= ~CCIFG;
}
