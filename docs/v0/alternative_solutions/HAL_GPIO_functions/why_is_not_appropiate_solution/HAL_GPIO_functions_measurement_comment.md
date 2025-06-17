# Why is this solution not the right one

Please have look at corresponding image 'HAL_GPIO_functions_measurement.png'

This time, the biggest error in our LED signal is caused by HAL function HAL_GPIO_WritePin:
- In ../Src/main.c, the followind code is executed
```
file: ../Src/main.c, line 107
			// turn off old LED
			HAL_GPIO_WritePin(LED_PORT, puiGpioLedPins[iCurrentLed], GPIO_PIN_RESET);

			// increment current led index
			if (++iCurrentLed == 2)
				iCurrentLed = 0;

			// turn on new LED
			HAL_GPIO_WritePin(LED_PORT, puiGpioLedPins[iCurrentLed], GPIO_PIN_SET);
```
- In other words, this code produces following:
	1) reset old pin
	2) increment LED
	3) set new pin
- As you can see, incrementing LED index takes time. Not only that, switching from the last LED to the first LED consumes extra time, which causes signal being not perfect . That causes setting and resetting pins being done in different time, which is not good.
	> **Note:** Maybe we could compute this in cameras, but that is not needed, as writing to ODR register (via DMA or manually) causes practically immediate switch of LED states practically simultaneously

- Signal is not fast enough
> 	We need LED frequency around 1 MHz, 660 kHz is not enough
> 
> 	HAL_GPIO_WritePin checks if PinState is set or reset state. This operation takes extra time. When using DMA, this computation is done automatically (by hardware) and much faster.