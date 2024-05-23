target extended-remote :3333

monitor reset halt

load

break vAssertCalled
#break assert_failed
#break vApplicationMallocFailedHook
#break vApplicationStackOverflowHook
#break prvTraceAssertCreate
