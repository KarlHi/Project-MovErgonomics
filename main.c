/* Including packages and files */ 
#include "gd32vf103.h"
#include "lcd.h"
#include "delay.h"
#include "gd32v_mpu6500_if.h"
#include <stdio.h>
#include "gd32vf103v_eval.h"

/* Define global variables */
#define GRAPH_HEIGHT 30
#define EI 1
#define DI 0

#define ARRAYNUM(arr_nanme)      (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define TRANSMIT_SIZE            (ARRAYNUM(txbuffer) - 1)

/* Create variables for transmit & recive */
uint8_t txbuffer[] = "brassi";
//uint8_t rxbuffer;
uint8_t tx_size = TRANSMIT_SIZE;
uint8_t rx_size = TRANSMIT_SIZE;
__IO uint8_t txcount = 0; 
__IO uint16_t rxcount = 0;

/* Create a package struct to transmit (transmit and recive data from gyro/acc, vectors) */
struct Package {
    float aX,aY,aZ,gX,gY,gZ;
};

/* Main function */

int main(void) {
	////////////////////////// Initialize LCD /////////////////////////////////////////
    struct Package package;
    package.aX = 1;
    package.aY = 2;
    package.aZ = 3;
    package.gX = 4;
    package.gY = 5;
    package.gZ = 6;
    
    struct Package package2;
    package2.aX = 7;
    package2.aY = 8;
    package2.aZ = 9;
    package2.gX = 10;
    package2.gY = 11;
    package2.gZ = 12;
    uint8_t * rxbuffer = (uint8_t*)&package2;
    uint8_t * bufferstruct = (uint8_t*)&package;
	Lcd_SetType(LCD_INVERTED);
    Lcd_Init();
    LCD_Clear(BLACK);
	/* int to write T/R characters on LCD screen */
	int x = 0;
	int y = 0;
	int x2 = 80;
	int y2 = 0;

	///////////////////////// Initialize Acc & Gyro ///////////////////////////////////

	/* The related data structure for the IMU, contains a vector of x, y, z floats*/
    mpu_vector_t Acc, vec_temp;
    mpu_vector_t Gyro, vec2_temp;
    /* for lcd */
    uint16_t line_color;

	/* Initialize pins for I2C */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C0);
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    /* Initialize the IMU (Notice that MPU6500 is referenced, this is due to the fact that ICM-20600
       ICM-20600 is mostly register compatible with MPU6500, if MPU6500 is used only thing that needs
       to change is MPU6500_WHO_AM_I_ID from 0x11 to 0x70. */
    mpu6500_install(I2C0);

	/* Creates temporary vectors to erase previously drwan lines */
    mpu6500_getAccel(&vec_temp);
    mpu6500_getGyro(&vec2_temp);

	//////////////////////////////////// Initialize Usart //////////////////////////////////////

	/* USART interrupt configuration */
    eclic_global_interrupt_enable();
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
    eclic_irq_enable(USART0_IRQn, 1, 0);
		
    /* configure COM0 */
    gd_eval_com_init(EVAL_COM0);                                // Startar hela USART systemet.
	
    
	/* enable USART TBE & RBNE interrupt */  
    usart_interrupt_enable(USART0, USART_INT_TBE);
    usart_interrupt_enable(USART0, USART_INT_RBNE);

    /////////////////////////////////// Initialize Math ///////////////////////////////////////// 

    /*  */




	/* Infinity while loop to always check gyro/acc & sedn/recive */
    
	while(1) {
        ///////////////////////////////////////////////// Accel & Gyro //////////////////////////////////////////////////////
		
        mpu6500_getGyroAccel(&Acc,&Gyro);
        // Calls on getAccel and getGyro at the same time
        package.aX = Acc.x;
        package.aY = Acc.y;
        package.aZ = Acc.z;
        package.gX = Gyro.x;
        package.gY = Gyro.y;
        package.gZ = Gyro.z;

        /* Skala värdena!!! */

        /////////////////////////////////////////////////// USART ////////////////////////////////////////////////////////////  

        while((txcount < sizeof(package) && rxcount<sizeof(package))) {
            if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE)){ 
                usart_data_transmit(USART0, bufferstruct[txcount++]);
                //LCD_ShowNum(50,10,txcount,2,BLUE);
                //LCD_Wait_On_Queue();
            }
            //delay_1ms(50);
            if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){ 
                rxbuffer[rxcount++] = usart_data_receive(USART0);
                //LCD_ShowNum(100,10,rxcount,2,YELLOW);
                //LCD_Wait_On_Queue();
            }
        }
        txcount =0;
        rxcount=0;

        //float num =  *((float*)buffer);
        /* Print out rxbuffer, the recived information to see that everything is there */
        LCD_ShowNum1(10,10,package2.aX,6,GREEN);
        LCD_Wait_On_Queue();
        LCD_ShowNum1(90,10,package2.gX,6,GREEN);
        LCD_Wait_On_Queue();
        LCD_ShowNum1(10,30,package2.aY,6,GREEN);
        LCD_Wait_On_Queue();
        LCD_ShowNum1(90,30,package2.gY,6,GREEN);
        LCD_Wait_On_Queue();
        LCD_ShowNum1(10,50,package2.aZ,6,GREEN);
        LCD_Wait_On_Queue();
        LCD_ShowNum1(90,50,package2.gZ,6,GREEN);
        LCD_Wait_On_Queue();
        
        delay_1ms(1000);
        LCD_Clear(BLACK);

        /////////////////////////////////////////////////// Math //////////////////////////////////////////////////////////// 
        if(package2.aX < 0){
            if(package2.aY < 0){
                LCD_Clear(GREEN);
            } else if(package2.aY > 0){
                LCD_Clear(GREEN);
            }
        } else if(package2.aX > 0){
            if(package2.aY < 0){
                LCD_Clear(GREEN);
            } else if(package2.aY > 0){
                LCD_Clear(GREEN);
            }
        }
        /* Skriv kod för att navigera i ett 2D rutnät */
		
    };
}