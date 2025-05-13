#include <ti/devices/msp/msp.h>
#include <elec327.h>
#include <stdio.h>

//Libraries for I2C
#include "ti_msp_dl_config.h"
#include "lcd_i2c.h"

#define POWER_STARTUP_DELAY (16)

/* Maximum size of TX packet */
#define I2C_TX_MAX_PACKET_SIZE (16)

/* Number of bytes to send to target device */
#define I2C_TX_PACKET_SIZE (16)

/* Maximum size of RX packet */
#define I2C_RX_MAX_PACKET_SIZE (16)

/* Number of bytes to received from target */
#define I2C_RX_PACKET_SIZE (16)

/* I2C Target address */
#define I2C_TARGET_ADDRESS (0x27)

/* Data sent to the Target */
uint8_t gTxPacket[I2C_TX_MAX_PACKET_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

/* Counters for TX length and bytes sent */
uint32_t gTxLen, gTxCount;
uint8_t gRxPacket[I2C_RX_MAX_PACKET_SIZE];
/* Counters for TX length and bytes sent */
uint32_t gRxLen, gRxCount;
/* Indicates status of I2C */
enum I2cControllerStatus {
    I2C_STATUS_IDLE = 0,
    I2C_STATUS_TX_STARTED,
    I2C_STATUS_TX_INPROGRESS,
    I2C_STATUS_TX_COMPLETE,
    I2C_STATUS_RX_STARTED,
    I2C_STATUS_RX_INPROGRESS,
    I2C_STATUS_RX_COMPLETE,
    I2C_STATUS_ERROR,
} gI2cControllerStatus;

enum current_state_enum {
    SENSOR_STATE,
    LCD_STATE
}; // 0: sensor state, 1: lcd state




#define TRIG_PIN   (0x1u << 26)  // GPIOB_DIO26
#define ECHO_PIN   (0x1u << 22)  // GPIOB_DIO22

#define BTN (0x1u << 17)

// Define the pins for SDA and SCL
#define SDA_PIN    (0x1u << 3)  // PB2
#define SCL_PIN    (0x1u << 2)  // PB3


#define MAX_ECHO_TIME_US 881600
#define rep_threshold 200000 // Change this parameter to change the distance from bar to floor to count boundary

// Rolling average filter window size
#define WINDOW_SIZE 5
float pulse_width_window[WINDOW_SIZE] = {0};
int window_index = 0;

// Rep counting
int num_reps = 0;
int temp_num_reps = 0;
char rep_state = 'd'; // 'u' = up, 'd' = down

// Thresholds
#define TOP_THRESHOLD 140000  // bar lowered (pulse width increases)
#define BOTT_THRESHOLD   120000  // bar lifted (pulse width decreases)


// Define macros for GPIO toggle
#define LED_ON()  (GPIOB->DOUTSET31_0 = TRIG_PIN)
#define LED_OFF() (GPIOB->DOUTCLR31_0 = TRIG_PIN)


float get_filtered_pulse_width(float new_reading) {
    // Reject outlier
    if (new_reading > 100 && new_reading < 400000) {
        pulse_width_window[window_index] = new_reading;
        window_index = (window_index + 1) % WINDOW_SIZE;
    }

    // Compute average
    float sum = 0;
    int valid_samples = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        if (pulse_width_window[i] != 0) { // ignore zeroed slots initially
            sum += pulse_width_window[i];
            valid_samples++;
        }
    }
    if (valid_samples == 0) return new_reading; // fallback
    return sum / valid_samples;
}


uint32_t micros(void) {
    return (~SysTick->VAL) & 0x00FFFFFF;
}

void InitializeCLKOut(void) {
    // This function initializes the GPIO so that the system clock is outputed to pin PA31
    // !!!LOOK-OUT!!! GPIOA POWER CYCLING!!! (change if using GPIOA!)
    GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
    GPIOA->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);
    delay_cycles(POWER_STARTUP_DELAY); // delay to enable GPIO to turn on and reset
    IOMUX->SECCFG.PINCM[(IOMUX_PINCM6)] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM6_PF_SYSCTL_CLK_OUT);
    GPIOA->DOESET31_0 = 0x1u<<31; // PA31 is our output pin for the clock
}

void InitializeProcessor(void) {
    SYSCTL->SOCLOCK.BORTHRESHOLD = SYSCTL_SYSSTATUS_BORCURTHRESHOLD_BORMIN; // Brownout generates a reset.

    update_reg(&SYSCTL->SOCLOCK.MCLKCFG, (uint32_t) SYSCTL_MCLKCFG_UDIV_NODIVIDE, SYSCTL_MCLKCFG_UDIV_MASK); // Set UPCLK divider
    update_reg(&SYSCTL->SOCLOCK.SYSOSCCFG, SYSCTL_SYSOSCCFG_FREQ_SYSOSCBASE, SYSCTL_SYSOSCCFG_FREQ_MASK); // Set SYSOSC to 32 MHz

    // Disable MCLK Divider
    update_reg(&SYSCTL->SOCLOCK.MCLKCFG, (uint32_t) 0x0, SYSCTL_MCLKCFG_MDIV_MASK);

    // Enable external clock out
    update_reg(&SYSCTL->SOCLOCK.GENCLKCFG,
        (uint32_t) SYSCTL_GENCLKCFG_EXCLKDIVEN_PASSTHRU | (uint32_t) SYSCTL_GENCLKCFG_EXCLKSRC_LFCLK,
        SYSCTL_GENCLKCFG_EXCLKDIVEN_MASK | SYSCTL_GENCLKCFG_EXCLKDIVVAL_MASK |
            SYSCTL_GENCLKCFG_EXCLKSRC_MASK);
    SYSCTL->SOCLOCK.GENCLKEN |= SYSCTL_GENCLKEN_EXCLKEN_ENABLE;


    // Enable the SysTick counter. Systick is a 24-bit counter.
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk; // Disable Systick for configuration
    SysTick->LOAD = 0x00FFFFFF;                 // Set the value that the Systick should count down to to the maximum for 24 bits
    SysTick->VAL  = 0;                          // Set the current value of the counter to zero. When we execute the next line, it will loop around back to LOAD
    SysTick->CTRL |= (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk); // Re-enable systick
}



float pulse_width;
float inches;
float cm;

int main(void)
{


    enum current_state_enum next_state;
    next_state = SENSOR_STATE;

    // Functional
    while (1) {
       switch (next_state) {
       case SENSOR_STATE:
           InitializeProcessor();
           InitializeCLKOut();

           /* Code to initialize GPIO PORT */

           // 1. Reset GPIO port (the one(s) for pins that you will use)
           GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
           GPIOB->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);

           // 2. Enable power on LED GPIO port
           GPIOB->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);

           delay_cycles(POWER_STARTUP_DELAY); // delay to enable GPIO to turn on and reset

           uint32_t input_configuration =
               IOMUX_PINCM_PC_CONNECTED   |  // connect pad to IOMUX
               IOMUX_PINCM_INENA_ENABLE   |  // enable the input buffer
               ((uint32_t)0x00000001)      |  // select MUX entry 1 = GPIO
               IOMUX_PINCM_INV_DISABLE    |  // no signal inversion
               IOMUX_PINCM_PIPU_DISABLE   |  // turn OFF internal pull-up
               IOMUX_PINCM_PIPD_ENABLE    |  // turn ON internal pull-down
               IOMUX_PINCM_HYSTEN_DISABLE |  // (optional) no hysteresis
               IOMUX_PINCM_WUEN_DISABLE;     // (optional) no wake-on-edge

           /* Code to initialize specific GPIO PINS */
           // 3. Initialize the appropriate pin(s) as digital outputs (Configure the Pinmux!)
           IOMUX->SECCFG.PINCM[(IOMUX_PINCM50)] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM_INENA_ENABLE | ((uint32_t) 0x00000001)); // IOMUX_PINCM50_PF_GPIOB_DIO22
           IOMUX->SECCFG.PINCM[(IOMUX_PINCM57)] = (IOMUX_PINCM_PC_CONNECTED | ((uint32_t) 0x00000001)); // IOMUX_PINCM50_PF_GPIOB_DIO22

           //IOMUX->SECCFG.PINCM[(IOMUX_PINCM43)] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM_INENA_ENABLE | ((uint32_t) 0x00000001)); // BTN input for reset

           IOMUX->SECCFG.PINCM[(IOMUX_PINCM43)] = input_configuration; // BTN input for reset

           // 4. Clear pins (set values to 0)
           GPIOB->DOUTCLR31_0 = TRIG_PIN;

           // 5. Enable GPIO output
           GPIOB->DOESET31_0 = TRIG_PIN;

           LED_ON();
           delay_cycles(30000);
           LED_OFF();

           if (GPIOB->DIN31_0 & BTN) {  // button reset
               num_reps = 0;
           }

           // Wait for ECHO to go HIGH
           while (!(GPIOB->DIN31_0 & ECHO_PIN));
           uint32_t start = micros();

           // Wait for ECHO to go LOW
           while (GPIOB->DIN31_0 & ECHO_PIN);
           uint32_t end = micros();

           pulse_width = (end - start);


           if (pulse_width > MAX_ECHO_TIME_US) {
               printf("Out of range (pulse_width = %f us)\n", pulse_width);
               fflush(stdout);
           } else {
               cm = pulse_width / 58.0f;
               inches = pulse_width / 148.0f;

               float filtered_pulse_width = get_filtered_pulse_width(pulse_width);

               // Convert to cm and inches
               cm = filtered_pulse_width / 58.0f;
               inches = filtered_pulse_width / 148.0f;

               printf("pulse_width: %f us, filtered: %f us, cm: %.2f, inches: %.2f, rep_state: %c, reps: %d\n",
                      pulse_width, filtered_pulse_width, cm, inches, rep_state, num_reps);

               fflush(stdout);

               temp_num_reps = num_reps; // set temp reps before incrementing reps

               if (rep_state == 'd' && filtered_pulse_width < BOTT_THRESHOLD) {
                   rep_state = 'u'; // bar lifted
               } else if (rep_state == 'u' && filtered_pulse_width > TOP_THRESHOLD) {
                   rep_state = 'd'; // bar lifted
                   num_reps++;      // count a rep
               }

           }

           //       delay_cycles(3000000);
                   delay_cycles(300000);
                   next_state = LCD_STATE;

                   break;


        case LCD_STATE:
            SYSCFG_DL_init();

            NVIC_EnableIRQ(I2C_INST_INT_IRQN);
            DL_SYSCTL_disableSleepOnExit();

            lcd_init();
            lcd_set_cursor(0, 0);
            char buf[16];
            snprintf(buf, sizeof(buf), "REPS:%d", num_reps);
            lcd_print(buf);


            printf("FLAG");
            fflush(stdout);
            next_state = SENSOR_STATE;

        default:
            break;
        }


    }
}


void I2C_INST_IRQHandler(void)
{
    switch (DL_I2C_getPendingInterrupt(I2C_INST)) {
        case DL_I2C_IIDX_CONTROLLER_RX_DONE:
            gI2cControllerStatus = I2C_STATUS_RX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_TX_DONE:
            DL_I2C_disableInterrupt(
                I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
            gI2cControllerStatus = I2C_STATUS_TX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_RX_INPROGRESS;
            /* Receive all bytes from target */
            while (DL_I2C_isControllerRXFIFOEmpty(I2C_INST) != true) {
                if (gRxCount < gRxLen) {
                    gRxPacket[gRxCount++] =
                        DL_I2C_receiveControllerData(I2C_INST);
                } else {
                    /* Ignore and remove from FIFO if the buffer is full */
                    DL_I2C_receiveControllerData(I2C_INST);
                }
            }
            break;
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_TX_INPROGRESS;
            /* Fill TX FIFO with next bytes to send */
            if (gTxCount < gTxLen) {
                gTxCount += DL_I2C_fillControllerTXFIFO(
                    I2C_INST, &gTxPacket[gTxCount], gTxLen - gTxCount);
            }
            break;
            /* Not used for this example */
        case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
        case DL_I2C_IIDX_CONTROLLER_NACK:
            if ((gI2cControllerStatus == I2C_STATUS_RX_STARTED) ||
                (gI2cControllerStatus == I2C_STATUS_TX_STARTED)) {
                /* NACK interrupt if I2C Target is disconnected */
                gI2cControllerStatus = I2C_STATUS_ERROR;
            }
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_EMPTY:
        case DL_I2C_IIDX_CONTROLLER_START:
        case DL_I2C_IIDX_CONTROLLER_STOP:
        case DL_I2C_IIDX_CONTROLLER_EVENT1_DMA_DONE:
        case DL_I2C_IIDX_CONTROLLER_EVENT2_DMA_DONE:
        default:
            break;
    }
}


