#  Group    SubG     ID      Value
#--------------------------------------------
#Slave Masks
#--------------------------------------------
#Desired MASK (0), MSBytes (Defalut 0x00FF)
     0        1       0      0x00FF	
#Desired MASK (0), LSBytes (Default 0xFFFF)
     0        1       1      0xFFFF  
#Group MASK (1),   MSBytes (Defalut 0x0000)
     0        1       2      0x0000
#Group MASK (1),   LSBytes (Default 0x003F)
     0        1       3      0x003F
#Group MASK (2),   MSBytes (Defalut 0x0000)
     0        1       4      0x0000
#Group MASK (2),   LSBytes (Default 0x0FC0)
     0        1       5      0x0FC0
#Group MASK (3),   MSBytes (Defalut 0x0003)
     0        1       6      0x0003
#Group MASK (3),   LSBytes (Default 0xF000)
     0        1       7      0xF000
#Group MASK (4),   MSBytes (Default 0x00FC)
     0        1       8      0x00FC
#Group MASK (4),   LSBytes (Default 0x0000)
     0        1       9      0x0000
#--------------------------------------------
#JINF Internal registers
#--------------------------------------------
#SSF State                 (Default 0x0001)
     1        1       0      0x0000
#Busy_High Mask            (Default 0x0000)
     1        1       1      0x0000
#Busy_Low Mask             (Default 0x0000)
     1        1       2      0x0000
#S/H Time                  (Default 0x0080)
     1        1       3      0x0080
