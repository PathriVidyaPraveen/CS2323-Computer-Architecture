.section .data

L1: .word 200000, 80000
    .section .text
    .global main

main:



    la x3, L1 
    lw x4,0(x3) 
    lw x5, 4(x3)   

    
    lui x6, 0x10012  
    sw x0, 4(x6)  
    li x7, 0x20 
    sw x7, 8(x6)  


blink_loop:
   
    sw x7, 12(x6)     
    add x10, x4, x0
    jal x1, delay 

    sw x0, 12(x6)     
    add x10, x5, x0 
    jal x1, delay


    

    jal x0, blink_loop


delay:




delay_loop:
    addi x10, x10, -1  
    bne x10, x0, delay_loop
    jalr x0, 0(x1)     




