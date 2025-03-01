/**********************************************************
 * 
 *  sim_gpio.c
 * 
 *  DESCRIPTION:
 *      Simulated GPIO module. Reads and writes to simulated
 *      GPIO files.
 *
 *      The simulator is not modeled after any particular
 *      hardware so values defined here are relatively
 *      arbitraty, that being said there may be a UI that
 *      displays simulated GPIO data in the future so be
 *      careful when making changes.
 * 
 */

#include "generic.h"

#define NUMBER_OF_GPIO_PINS 50

#define PIN_FILE_FNAME "sim_gpio_pin_states"

enum
{
    UNCONFIGURED_PIN,
    OUTPUT_PIN,
    INPUT_PIN
};

typedef struct
{
    uint8_t io_state;
    uint8_t value : 1;  /* digital IO pin can only be 0 or 1 */

} simulated_gpio_pin_type;

/*
  No initialization on this type is required
  C99 Standard (ISO/IEC 9899:1999) Section 6.7.8, Paragraph 10 (Initialization)

  However, even if we wanted to handle the case of task reset, we would not
  want to clear this memory since this simulates a hardware state (not a software)
  state

  This is a reminder to myself to not mess with this when I decide to make
  the OS more "reset-able".
 */
static simulated_gpio_pin_type simulated_pins[ NUMBER_OF_GPIO_PINS ];

static boolean state_changed_flag = FALSE;

/**********************************************************
 * 
 *  gpio_pin_setas_outp
 * 
 * 
 *  DESCRIPTION:
 *      Enable output on the pin
 *
 */

void gpio_pin_setas_outp(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return;
    }
    simulated_pins[ pin ].io_state = OUTPUT_PIN;
    state_changed_flag = TRUE;
}

/**********************************************************
 * 
 *  gpio_pin_setas_inp
 * 
 * 
 *  DESCRIPTION:
 *      Enable input on the pin
 *
 */

void gpio_pin_setas_inp(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return;
    }
    simulated_pins[ pin ].io_state = INPUT_PIN;
    state_changed_flag = TRUE;

}

/**********************************************************
 * 
 *  gpio_maintenance_task
 * 
 * 
 *  DESCRIPTION:
 *      Read and write static simulated GPIO data to a
 *      local file.
 *
 */

void gpio_maintenance_task()
{
    if( state_changed_flag )
    {
    state_changed_flag = FALSE;
    }
    else
    {
    
    }
}


/**********************************************************
 * 
 *  gpio_pin_enable
 * 
 * 
 *  DESCRIPTION:
 *      Enable the pin for GPIO
 *
 */

void gpio_pin_enable(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return;
    }

}


/**********************************************************
 * 
 *  gpio_set
 * 
 * 
 *  DESCRIPTION:
 *      Set the pin
 *
 */

void gpio_set(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return;
    }
   simulated_pins[ pin ].io_state = 0x1;
}

/**********************************************************
 * 
 *  gpio_clr
 * 
 * 
 *  DESCRIPTION:
 *      Clear the pin
 *
 */

void gpio_clr(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return;
    }
    simulated_pins[ pin ].io_state = 0x0;
}

/**********************************************************
 * 
 *  gpio_get
 * 
 * 
 *  DESCRIPTION:
 *      Get current pin value
 *
 */

boolean gpio_get(uint32_t pin)
{
    if( pin >= NUMBER_OF_GPIO_PINS )
    {
        return 0x0;
    }
    return simulated_pins[ pin ].value;
}