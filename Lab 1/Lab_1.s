# Takes x5 and x6 content from the memory addresses
li x5 , 0x10000000
ld x7 , 0(x5)

li x6 , 0x10000008
ld x8 , 0(x6)


# Extract the sign of both operands and then try to find out sign of final result

srai x12 , x7 , 63 # Sign for x7 ( 0 or -1 )
srai x13 , x8 , 63 # Sign for x8 ( 0 or -1 )
xor x11 , x12 , x13
andi x11 , x11 , 1
# This x11 value is 1 if result is negative and 0 if result is positive

# Now we are finding absolute value of both operands for unsigned multiplication

xor x5 , x7 , x12
sub x5 , x5 , x12

xor x6 , x8 , x13
sub x6 , x6 , x13

# Unsigned integer multiplication of x5 and x6 (long multiplication method ) 
# ( similar to what we do for decimal numbers )

# Initialize result to 0

add x9 , x0 , x0

loop:
    andi x15 , x6 , 1 # Check LSB of x6
    beq x15 , x0 , skip_addition # If LSB = 0 , goes on to bit shifts
    add x9 , x9 , x5 # If LSB = 1 , add x5 one time to the result

skip_addition:
    
    slli x5 , x5 , 1 # Make  x5 << 1
    srli x6 , x6 , 1 # Make x6 >> 1 (logical right shift)
    bne x6 , x0 , loop # Moves to next iteration if x6 != 0
    
# Final step : Apply sign to result 

beq x11 , x0 , no_change_of_sign
sub x9 , x0 , x9

no_change_of_sign:
    
       
    
# Stores x9 to the givem memory address

li x10 , 0x10000050
sd x9 , 0(x10)


