# Why is this solution not the right one

> Please have look at corresponding image 'direclty_in_main_measurement.png'

As you can see, in ../Src/main.c we setup tim6 config to the following:
```
Period: 71
Prescaller: 0
F CPU: 72 MHz
```

Frequency of tim6 interrupt is computed via this formula:	$$f_o = f_i / (prescaller + 1) / (period + 1)$$
After pasting values, our formula looks like this: $$f_o = 72 MHz / (0 + 1) / (71 + 1) = 1 MHz$$
However, in out measurement picture, we can see that
	1) Output frequency is approximately 295 kHz, which is both too slow for camera (we want about 1 MHz for precise timing)
	2) Output frequency is approximately 295 kHz, which does not match frequency configured for tim6 (1 MHz)
> **Note:** Probably, the code execution takes more time than one timer period, which causes refresh frequency being smaller
