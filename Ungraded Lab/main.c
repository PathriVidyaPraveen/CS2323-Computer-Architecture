#include<stdio.h>
#include<stdint.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>

// memory locationg given in question


#define DATA_SEGMENT_BASE 0x10000000ULL
#define EXPONENT_BITS_ADDRESS (DATA_SEGMENT_BASE + 0x0)
#define MANTISSA_BITS_ADDRESS (DATA_SEGMENT_BASE + 0x8)
#define INPUT_ADDRESS (DATA_SEGMENT_BASE + 0x10)
#define OUTPUT_ADDRESS (DATA_SEGMENT_BASE + 0x200)

// memory space

#define MEMORY_SIZE 0x400
uint8_t simulated_memory[MEMORY_SIZE];


// read 64 bit dword from simulated memory

uint64_t memReadDword(uint64_t address){


    if(address < DATA_SEGMENT_BASE || (address - DATA_SEGMENT_BASE) > MEMORY_SIZE - 8){
        fprintf(stderr,"Error: Memory read out of bounds at 0x%llx\n",(unsigned long long)address);
        exit(1);
    }


    uint64_t offset = address - DATA_SEGMENT_BASE;
    uint64_t value;
    memcpy(&value,&simulated_memory[offset],sizeof(uint64_t));
    return value;
}


// write 64 bit dword to simulated memory

void memWriteDword(uint64_t address, uint64_t value){
    if(address < DATA_SEGMENT_BASE || (address-DATA_SEGMENT_BASE) > MEMORY_SIZE-8){
        fprintf(stderr,"Error: Memory write out of bounds at 0x%llx\n ",(unsigned long long)address);
        exit(1);
    }
    uint64_t offset = address - DATA_SEGMENT_BASE;
    memcpy(&simulated_memory[offset],&value,sizeof(uint64_t));
    return;
}


int EXPONENT_BITS = 11;
int MANTISSA_BITS = 52;
int BIAS = 1023;


// now split 64 bit double into sign , true exponent and 53 bit normalized mantissa with implicit 1

void decompose(uint64_t fp_val, int* sign, int64_t *exponent, uint64_t *mantissa){
    *sign = (fp_val >> 63) & 1;
    *exponent = (fp_val >> MANTISSA_BITS) & ((1ULL << EXPONENT_BITS)-1);
    *mantissa = fp_val & ((1ULL << MANTISSA_BITS) - 1);


    int max_exponent = (1 << EXPONENT_BITS) - 1;

    if(*exponent == 0 && *mantissa == 0){
        *exponent = 0;
    }else if(*exponent == max_exponent){

    }else{
        *mantissa = *mantissa | (1ULL << MANTISSA_BITS);
        *exponent = *exponent - BIAS;
    }


    return;
}

// merge parts back into 64 bit double after handling normalization , rounding and overflow or underflow

uint64_t recompose(int sign , int64_t exponent , uint64_t mantissa){


int max_exponent = (1 << EXPONENT_BITS)-1;

    uint64_t exponent_mask = (1ULL << EXPONENT_BITS)-1;

    if((exponent & exponent_mask) == exponent_mask){
        return ( (uint64_t)sign << 63) | (exponent_mask << MANTISSA_BITS) | (mantissa & ((1ULL << MANTISSA_BITS) - 1));

    }


    int64_t biased_exponent = exponent + BIAS;
    uint64_t final_mantissa = mantissa & ((1ULL << MANTISSA_BITS)-1);


    if(biased_exponent  >= max_exponent){
        // exponent overflow so plud or minus infinity

        return ((uint64_t)sign << 63) | (exponent_mask << MANTISSA_BITS);

    }else if(biased_exponent <= 0){
        // exponent underflow so 0
        return ((uint64_t)sign << 63);
    }else{
        return ((uint64_t)sign << 63)| ((uint64_t)biased_exponent << MANTISSA_BITS) |final_mantissa;
    }


    return ((uint64_t)sign << 63)| ((uint64_t)biased_exponent << MANTISSA_BITS) |final_mantissa;

}


int isNaN(uint64_t fp_val){
    int exponent_field = (fp_val >> MANTISSA_BITS) & ((1ULL << EXPONENT_BITS)-1);
    uint64_t mantissa_field = fp_val & ((1ULL << MANTISSA_BITS)-1);
    return (exponent_field == (1 << EXPONENT_BITS)-1) && (mantissa_field != 0);
}


int is_infinity(uint64_t fp_val){
    int exponent_field = (fp_val >> MANTISSA_BITS) & ((1ULL << EXPONENT_BITS)-1);
    uint64_t mantissa_field = fp_val & ((1ULL << MANTISSA_BITS)-1);
    return (exponent_field == (1 << EXPONENT_BITS)-1) && (mantissa_field == 0);
}

int is_zero(uint64_t fp_val){
    int exponent_field = (fp_val >> MANTISSA_BITS) & ((1ULL << EXPONENT_BITS)-1);
    uint64_t mantissa_field = fp_val & ((1ULL << MANTISSA_BITS)-1);
    return (exponent_field == 0) && (mantissa_field == 0);
}


// takes 2 64-bit integers and do multiplication and produce 128 bit result then normalizatiion


uint64_t dmult_int(uint64_t a , uint64_t b){
    int sign_a , sign_b;
    int64_t exponent_a , exponent_b;
    uint64_t mantissa_a , mantissa_b;

    decompose(a,&sign_a,&exponent_a,&mantissa_a);
    decompose(b,&sign_b,&exponent_b,&mantissa_b);
    int sign_result = sign_a ^ sign_b;

    int64_t exponent_result = exponent_a + exponent_b;


    
    __uint128_t mantissa_result_128 = (__uint128_t)mantissa_a * mantissa_b;
    uint64_t mantissa_result_high = (uint64_t)(mantissa_result_128 >> 64);
    uint64_t mantissa_result_low = (uint64_t)mantissa_result_128;


    // normalization

    uint64_t new_mantissa;
    int shift_amount;

    if(mantissa_result_128 >> 105){
        shift_amount = 105 - MANTISSA_BITS;
        exponent_result ++;
    }else{
        shift_amount = 104 - MANTISSA_BITS;
    }

    __uint128_t rounding_bit =(__uint128_t)1 << (shift_amount - 1);
        mantissa_result_128 = mantissa_result_128 + rounding_bit;
    new_mantissa = (uint64_t)(mantissa_result_128 >> shift_amount);




    return recompose(sign_result,exponent_result,new_mantissa);


}


// simulate double precision floating point numbers using integeers



// fix this function for precision correctness later!!

uint64_t fp64add_int(uint64_t a,uint64_t b){
    int sign_a , sign_b;
    int64_t exponent_a , exponent_b;
    uint64_t mantissa_a , mantissa_b;

    decompose(a,&sign_a,&exponent_a,&mantissa_a);
    decompose(b,&sign_b,&exponent_b,&mantissa_b);


    if(exponent_a < exponent_b){
        // swap values


        uint64_t temp_mantissa = mantissa_a;
        mantissa_a = mantissa_b;
        mantissa_b = temp_mantissa;

        int64_t temp_exponent = exponent_a;
        exponent_a = exponent_b;
        exponent_b = temp_exponent;

        int temp_sign = sign_a;
        sign_a = sign_b;
        sign_b = temp_sign;

    }

    int64_t exponent_result = exponent_a;
    int64_t exponent_difference = exponent_a - exponent_b; // shift amount


    uint64_t rounding_bit = 0;

    // if(exponent_difference > 0){
    //     mantissa_b = 0;
    // }else{
    //     mantissa_b = mantissa_b >> exponent_difference;
    // }


//     if(exponent_difference > 0){
//         mantissa_b = mantissa_b>>exponent_difference;
//     }


if(exponent_difference > 0){
    if(exponent_difference > 63){
        mantissa_b = 0;
    }else{



        if(mantissa_b &(1ULL << (exponent_difference-1))){
            rounding_bit = 1;
        }
        mantissa_b = mantissa_b >> exponent_difference;
    }
}

    uint64_t mantissa_result;
    int sign_result;

    if(sign_a == sign_b){
        mantissa_result = mantissa_a + mantissa_b;
        sign_result = sign_a;


        // fix this later

        if(rounding_bit){
            mantissa_result++;
        }

        if(mantissa_result &(1ULL << (MANTISSA_BITS+1))){
            mantissa_result = mantissa_result >> 1;
            exponent_result++;
        }


        // if(mantissa_result & (1ULL << (MANTISSA_BITS+1))){
        //     mantissa_result = mantissa_result >> 1;
        //     exponent_result++;
        // }


        // if(exponent_difference > 0){
        //     mantissa_result++;

        // }
        //             if (mantissa_result &(1ULL<<(MANTISSA_BITS + 1))) {
        //         mantissa_result = mantissa_result >> 1;
        //         exponent_result++;
        //     }


    }else{
        mantissa_result = mantissa_a - mantissa_b;
        sign_result = sign_a;


        if(!(mantissa_result & (1ULL << MANTISSA_BITS))    && mantissa_result != 0){
            // uint64_t temp = mantissa_result;
            // int shift_count = 0;


            // while((temp << shift_count) < (1ULL << 63) && shift_count < 64){
            //     shift_count++;
            // }

            // int leading_zeros = 64 - shift_count;
            // int normalization_shift = MANTISSA_BITS + leading_zeros - 63;

            // mantissa_result = mantissa_result << normalization_shift;
            // exponent_result = exponent_result - normalization_shift;

            int normalization_shift = 0;
            uint64_t temp = mantissa_result;

            for(int k=MANTISSA_BITS;k>=0;k--){
                if(temp &(1ULL << k)){
                    normalization_shift = MANTISSA_BITS - k;
                    break;
                }
            }
            if(normalization_shift > 0){
                mantissa_result = mantissa_result << normalization_shift;
                exponent_result = exponent_result - normalization_shift;

            }
        }
    }

// mantissa_result ++;
// if(mantissa_result & (1ULL << (MANTISSA_BITS+1))){
//     mantissa_result = mantissa_result >> 1;
//     exponent_result++;
// }



// uint64_t mantissa_result;
// int sign_result;



     return recompose(sign_result,exponent_result,mantissa_result);
    
}

// main rountine for double precision floating point addition

uint64_t fp64add(uint64_t a , uint64_t b){


    // check for nan

    uint64_t exponent_mask = (1ULL << EXPONENT_BITS)-1;
    uint64_t nan_value = ((uint64_t)1 << 63)| (exponent_mask << MANTISSA_BITS)| (1ULL << (MANTISSA_BITS - 1));

    if(isNaN(a) || isNaN(b)){
        return nan_value;
    }

    // special cases of infinity

    int a_inf = is_infinity(a);
    int b_inf = is_infinity(b);
    int sign_a = (a >> 63)&1;
    int sign_b = (b >> 63)&1;

    if(a_inf && b_inf){
        if(sign_a != sign_b){
            return 0;
        }else{
            return a;
        }

    }else if(a_inf){
        return a;
    }else if(b_inf){
        return b;
    }


    // do similar to integer division
    return fp64add_int(a,b);


}


// main routine for double precision floating point multiplication

uint64_t fp64mul(uint64_t a,uint64_t b){
    int sign_a = (a >> 63)&1;
    int sign_b = (b >> 63)&1;
    int sign_result = sign_a ^ sign_b;
    uint64_t exponent_mask = (1ULL << EXPONENT_BITS)-1;
    uint64_t inf_value =((uint64_t)sign_result << 63)|(exponent_mask << MANTISSA_BITS);
    uint64_t nan_value =((uint64_t)1 << 63)|(exponent_mask << MANTISSA_BITS)|(1ULL << (MANTISSA_BITS- 1));

    // check for nan

    if(isNaN(a) || isNaN(b)){
        return nan_value;
    }

    // special cases

    int a_inf = is_infinity(a);
    int b_inf = is_infinity(b);
    int a_0 = is_zero(a);
    int b_0 = is_zero(b);


    if(a_inf || b_inf){
        if(a_0 || b_0){
            return nan_value;
        }

        return inf_value;
    }else if(a_0 || b_0){
        return ((uint64_t)sign_result << 63);
    }




    return dmult_int(a,b);


}

// run simulation and entire testing part is done below

void runSimulation(){
    EXPONENT_BITS = (int)memReadDword(EXPONENT_BITS_ADDRESS);
    MANTISSA_BITS = (int)memReadDword(MANTISSA_BITS_ADDRESS);
    BIAS = (1 << (EXPONENT_BITS-1))-1;

    if(EXPONENT_BITS + MANTISSA_BITS > 63){
        fprintf(stderr,"Error: Exponent and mantissa bits sum exceeds 63\n");
        return;
    }

    int count = (int)memReadDword(INPUT_ADDRESS);

    uint64_t input_pair_address = INPUT_ADDRESS+8;
    uint64_t output_address = OUTPUT_ADDRESS;

    for(int i=0;i<count;i++){
        uint64_t a = memReadDword(input_pair_address);
        uint64_t b = memReadDword(input_pair_address+8);

        uint64_t sum = fp64add(a,b);
        uint64_t product = fp64mul(a,b);
        memWriteDword(output_address,sum);
        memWriteDword(output_address+8,product);

        printf("Pair %d (A: 0x%llX, B: 0x%llX)\n", i+1,(unsigned long long)a,(unsigned long long)b);
        printf("Sum (0x%llX): 0x%llX\n",(unsigned long long)output_address,(unsigned long long)sum);
        printf("Product (0x%llX): 0x%llX\n",(unsigned long long)output_address+8,(unsigned long long)product);

        input_pair_address += 16;
        output_address += 16;

    }

    printf("Simulation done!\n");


}

void initialize1(){
    memset(simulated_memory,0,MEMORY_SIZE);
    memWriteDword(EXPONENT_BITS_ADDRESS,11);
    memWriteDword(MANTISSA_BITS_ADDRESS,52);
    memWriteDword(INPUT_ADDRESS,1);
    memWriteDword(INPUT_ADDRESS+8,0x3fd2c00000000000ULL);   


    memWriteDword(INPUT_ADDRESS+16,0x40a22676f31205e7ULL);


    return;
}

void initialize2(){
    memset(simulated_memory,0,MEMORY_SIZE);
    memWriteDword(EXPONENT_BITS_ADDRESS,11);
    memWriteDword(MANTISSA_BITS_ADDRESS,52);





    memWriteDword(INPUT_ADDRESS,2);  

    memWriteDword(INPUT_ADDRESS+8,0x409fa0cf5c28f5c3ULL);
    memWriteDword(INPUT_ADDRESS+16,0x4193aa8fc4f0a3d7ULL);
    memWriteDword(INPUT_ADDRESS+24,0x40e699d2003eea21ULL);
    memWriteDword(INPUT_ADDRESS+32,0x420e674bcb5a1877ULL);

    return;
}


int main(){

    initialize1();
    runSimulation();

    printf("Sample1 : \n");
    printf("Expected Sum: 0x40a2270cf31205e7 , Actual Sum: 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS));
    printf("Expected Product: 0x4085451364d91eeb , Actual Product: 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS+8));


    initialize2();
    runSimulation();

    printf("Sample2: \n");
    printf("Pair 1 Sum (0x10000200): Expected- 0x4193aaaf65c00000 , Actual- 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS));
    printf("Pair 1 Product (0x10000208): Expected- 0x4243700f85975d74 , Actual- 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS + 8));
    printf("Pair 2 Sum (0x10000210): Expected- 0x420e675171ce9887 , Actual- 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS + 16));
    printf("Pair 2 Product (0x10000218) : Expected- 0x43057929844f64ac , Actual- 0x%llX\n",(unsigned long long)memReadDword(OUTPUT_ADDRESS + 24));





    return 0;
}






