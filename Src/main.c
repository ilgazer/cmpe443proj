/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "board/timer.h"
#include "board/rcc.h"
#include "board/gpio.h"
#include "board/iser.h"
#include "utils.h"
#include "motors.h"
#include "joystick.h"
#include "leds.h"
#include "drive.h"
#include "board/adc.h"
#include "board/exti.h"



void ADC1_2_IRQHandler() {

	drive();
	SET(ADC1->ISR, ADC_JEOS);
}


void EXTI15_IRQHandler() {
	driver_stop();
	SET(EXTI->FPR1, 15);
}

int main(void) {
	init_mode(AUTO_WAIT);
	init_motors();

	init_leds();
	set_led_direction(LED_STOP);
	initialize_adc();
	while (1) {
//		set_led_direction(LED_STOP);
	    for(int i=0; i<=3330; i++);
		SET(ADC1->CR, ADC_JADSTART);
	}
}
