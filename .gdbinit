target extended-remote :3333

monitor reset halt

load

break vApplicationMallocFailedHook
break vApplicationStackOverflowHook
break vAssertCalled
