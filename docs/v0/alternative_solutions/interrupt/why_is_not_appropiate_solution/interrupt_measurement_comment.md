# Why is this solution not the right one

Please have look at corresponding image interrupt_measurement.png

As you can see, in ../Src/main.c we setup tim6 config to the following:
```
Period: 71
Prescaller: 0
F CPU: 72 MHz
```

Frequency of tim6 interrupt is computed via this formula: $$f_o = f_i / (prescaller + 1) / (period + 1)$$
After pasting values, our formula looks like this: $$f_o = 72 MHz / (0 + 1) / (71 + 1) = 1 MHz$$
## However, in out measurement picture, we can see both that
1) Output frequency is cca 295 kHz, which is both
		- too slow for camera (we want about 1 MHz)
		- it does not match frequency configured for tim6 (1 MHz)
>		**Note:** probably, the callback code execution takes more time than one timer period, which causes interrupt frequency being smaller
2) Signal is in logic state 0 less time than in logic time 1 (However, this is not as big problem as the first one)
		This is due to following code:
```
		file: ../Src/main.c; line: 74, 75
		
		if (++iCurrentLed == 2)
			iCurrentLed = 0;
```
Experienced developers (or developers using Debug mode :-) ) can see that this code produces following assembly:
```
 74       		if (++iCurrentLed == 2)
 ...
08002ffc:   adds    r3, #1			// <- increment register r3 value
 ...
08003006:   cmp     r3, #2			// <- compare r3 with value 2 (result stored in bit Z in core registers)
08003008:   bne.n   0x8003010 <HAL_TIM_PeriodElapsedCallback+60>	// jump to address 0x8003010 if Z bit in core registers points out that cmp comparison was NOT equal
 ...
 75       			iCurrentLed = 0;
 ...
0800300c:   movs    r2, #0			// <- load 0 to register r2
0800300e:   str     r2, [r3, #0]	// <- stores value in register r2 to RAM
 ...
```
 
Did you catch that? The HAL_TIM_PeriodElapsedCallback function takes more time to execute if iCurrentLed equals to 1 (before interrupt arriving). Let me simplify that assembly instructions:
			1) Increment iCurrentLed
			2) Compare iCurrentLed with 2
				equal => jump to some address and skip instructions
				different => execute more instructions
