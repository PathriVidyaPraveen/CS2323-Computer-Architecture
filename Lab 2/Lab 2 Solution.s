.data
.dword 3, 12, 3, 125 , 50 , 32 , 16 # sample input

.text

#The following line initializes register x3 with 0x10000000 
#so that you can use x3 for referencing various memory locations. 
lui x3, 0x10000
#your code starts here

# load number of gcd computations in x4
ld x4 , 0(x3)
addi x5 , x3 , 8 # Starts taking inputs
li x6 , 0x10000200
add x7 , x0 , x0 # Stores number of computations completed so that we can break out of loop


gcd_loop:
    
       # break out if all gcds computed
       beq x7 , x4 , end_loop
       ld x8 , 0(x5) # first number 
       ld x9 , 8(x5) # second number
       # if atleast one is zero we store gcd as 0 and move on to next pair
       beq x8 , x0 , make_gcd_zero
       beq x9 , x0 , make_gcd_zero
       
compute_gcd:
    
       # if both numbers are equal we store that result as gcd and move on to next pair
       
       beq x8 , x9 , store_result_as_gcd
       bltu x9 , x8 , operation_if_a_is_greater
       sub x9 , x9 , x8 # b = b - a
       jal x0 , compute_gcd
       
operation_if_a_is_greater:
       sub x8 , x8 , x9 # a = a - b
       jal x0 , compute_gcd
       
       
       
store_result_as_gcd:
    
    sd x8 , 0(x6)
    addi x5 , x5 , 16 # for next gcd computation inputs
    addi x6 , x6 , 8 # for storing gcd of next computation
    addi x7 , x7 , 1 # increment number of completed gcds
    jal x0 , gcd_loop
       
make_gcd_zero:
    
    sd x0 , 0(x6)
    addi x5 , x5 , 16 # for next gcd computation inputs
    addi x6 , x6 , 8 # for storing gcd of next computation
    addi x7 , x7 , 1 # increment number of completed gcds
    jal x0 , gcd_loop
    
       
       
       
       
       
end_loop:
    
    jal x0 , end_loop
     
       
     
#The final result should be in memory starting from address 0x10000200
#The first dword location at 0x10000200 contains gcd of input11, input12
#The second dword location from 0x10000200 contains gcd of input21, input22, and so on.