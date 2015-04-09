# MDM_OffsetAdder
This program add an offset to your measures on IC_CAP software (MDM files)

If you want, for exemple, to realize a measure between 0.3V to 3.6V. 
For the extraction, IC-CAP needs a 0V default value... so you can add an offset : your Vds, Vgs [...] stay the same.

To choose the offset value, change the define value of V_OFFSET (3.3 by default).

This program needs [N times] two arguments :
  - the transistor type : NMOS or PMOS
  - the name of the file to modifiy (.mdm)

Notice that a backup file, with the original file, is created.

How to use :
1 _ Compile .c file (on linux : > gcc main.c -o MDM_OffsetAdder)
2 _ Execute :
    a) on linux : the .sh file with the all list of MDM files you want to modify
    b) on others/windows : execute directely the MDM_OffsetAdder(.exe) exceutable (with only the two arguments).
    
