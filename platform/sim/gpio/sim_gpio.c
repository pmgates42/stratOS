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
#include "fs.h"
#include "debug.h"

#define NUMBER_OF_GPIO_PINS 50

#define PIN_FILE_FNAME "build/sim/sim_gpio_pin_states.state"

enum
{
    UNCONFIGURED_PIN,   /* Must always be first */
    OUTPUT_PIN,
    INPUT_PIN
};

typedef struct
{
    uint8_t io_state;
    uint8_t value : 1;  /* digital IO pin can only be 0 or 1 */
    uint8_t function;   /* IO function */
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

static void write_out_pins(void);
static void read_in_pins(void);

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

}

/**********************************************************
 * 
 *  write_out_sim_gpio_data
 * 
 * 
 *  DESCRIPTION:
 *      Write out the simulated GPIO data
 *
 */

static void write_out_sim_gpio_data()
{
    write_out_pins();
}


void gpio_maintenance_task()
{
static boolean init = TRUE; /* initialize task? */
if( init )
    {
    write_out_sim_gpio_data();
    init = FALSE;
    }

    read_in_pins(); 
}

static void write_out_pins(void)
{
    uint8_t       i;
    file_handle_t fhandle;
    // clr_mem( &fhandle, sizeof fhandle ); TODO

    fs_open_file( PIN_FILE_FNAME, &fhandle, FILE_OPEN_FLAGS_WRITE | FILE_OPEN_FLAGS_CREATE | FILE_OPEN_FLAGS_OVRWRT );

    for( i = 0; i < NUMBER_OF_GPIO_PINS; i++ )
    {
        fs_writef( fhandle, "%d %d\r\n", simulated_pins[i].io_state, simulated_pins[i].value );
    }
    fs_close_file(&fhandle);
}

static void read_in_pins(void)
{
    #define LINE_SZ_BYTES   5

    typedef struct
    {
    uint8_t io_state;
    uint8_t dont_care_0[1];
    uint8_t value;
    uint8_t dont_care_1[2];
    } state_file_line_type;

    static char buf[NUMBER_OF_GPIO_PINS * LINE_SZ_BYTES]; 

    uint8_t           i;
    file_handle_t     fhandle;
    int               value;
    state_file_line_type
                    * temp_line;
    const uint8_t   * buf_ptr;

    fs_open_file(PIN_FILE_FNAME, &fhandle, FILE_OPEN_FLAGS_READ);
    if( fs_read(fhandle, buf, sizeof buf) != FILE_ERR_NONE)
        return;

    buf_ptr = buf;

    for (i = 0; i < 0; i++)
    {
        temp_line = (state_file_line_type *)buf_ptr;

        simulated_pins[i].io_state = temp_line->io_state;
        simulated_pins[i].value    = temp_line->value;

        buf_ptr += LINE_SZ_BYTES;
    }

    fs_close_file(&fhandle);
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
   simulated_pins[ pin ].value = 0x1;
   write_out_pins();
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
    simulated_pins[ pin ].value = 0x0;
    write_out_pins();
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
    read_in_pins();
    return simulated_pins[ pin ].value;
}

void gpio_pin_set_func(uint32_t pin, uint8_t fnc)
{
    simulated_pins[ pin ].function = fnc;
}
