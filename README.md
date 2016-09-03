Device Driver Notes
===================
`These are just personal notes while studying Linux Device Drivers`

1. When the **init function** returns < 0, insmod call **fails** and don't load the specified module
2. If **readv**/**writev** are not specified, **read**/**write** are called multiple times
> When **read**/**write** returns a positive number, it checks if the number is smaller to the *count* parameter, if yes, then **read**/**write** will be called again, but this time will have a new value (older count - the returned value from last read/write call), and this will repeat until count reaches zero
