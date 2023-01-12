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
#include "pins.h"
#include "indicators.h"
#include "board/adc.h"
#include "board/exti.h"
#include "ultrasonic.h"

#define BUSY_WAIT 1

void init_extis() {
	SET(RCC_AHB2ENR, GPIOAEN);
	SET(RCC_AHB2ENR, GPIOCEN);

	SET_BITS(GPIOA->MODER, JOY_BTN * 2, INPUT_MODE, 2);
	SET_BITS(GPIOA->PUPDR, JOY_BTN * 2, PULLUP, 2);

	SET_BITS(GPIOC->MODER, BLUE_BTN * 2, INPUT_MODE, 2);
	SET_BITS(GPIOC->PUPDR, BLUE_BTN * 2, PULLDOWN, 2);

	EXTI->EXTISR[JOY_BTN] = 0;
	SET(EXTI->IMR1, JOY_BTN);
	SET(EXTI->FTSR1, JOY_BTN);
	SET(ISER0, 11 + JOY_BTN);

	EXTI->EXTISR[BLUE_BTN] = 2;
	SET(EXTI->IMR1, BLUE_BTN);
	SET(EXTI->RTSR1, BLUE_BTN);
	SET(ISER0, 11 + BLUE_BTN);
}
void init_TIM7() {
	SET(RCC_APB1ENR1, TIM7EN); //TIM6x_CLK is enabled, running at 4MHz
	TIM7->EGR |= 1; //enable UIF to generate an interrupt
	TIM7->PSC = 15999; //Set Prescaler
	TIM7->CR1 &= ~(1 << 1); //OVF will generate an event

	// TIM6 Interrupt Initialization
	TIM7->ARR = 5;
	TIM7->SR = 0; //clear UIF if it is set
	TIM7->DIER |= 1;
	ISER1 |= 1 << 18; //enable global signaling for TIM6 interrupt

	TIM7->CR1 |= 1; //TIM6_CNT is enabled (clocked)

	enable_interrupts();
}
void TIM7_IRQHandler(void) {
	SET(ADC1->CR, ADC_JADSTART);
	TIM7->SR = 0; //clear UIF
}
void ADC1_2_IRQHandler() {
	drive();
	SET(ADC1->ISR, ADC_JEOS);
}

//Depends on the value of JOY_BTN
void EXTI6_IRQHandler() {
	joystick_button_handler();
	SET(EXTI->FPR1, JOY_BTN);
}

//Depends on the value of BLUE_BTN
void EXTI13_IRQHandler() {
	static int auto_mode = 0;
	enable();
	if (auto_mode) {
		auto_mode = 0;
		set_mode(MANUAL);
	} else {
		init_mode(AUTO_WAIT);
		auto_mode = 1;
	}
	SET(EXTI->RPR1, BLUE_BTN);
}

int main(void) {
	init_mode(MANUAL);
	init_indicators();
	init_motors();
	init_extis();
	init_leds();
	set_led_direction(LED_STOP);
	initialize_adc();
	init_ultrasonic();
	if (BUSY_WAIT) {
		while (1) {
			for (int i = 0; i < 33300; i++)
				;
			SET(ADC1->CR, ADC_JADSTART);
		}
	} else {
		init_TIM7();
		while (1) {
			asm volatile("wfi");
		}
	}
}
