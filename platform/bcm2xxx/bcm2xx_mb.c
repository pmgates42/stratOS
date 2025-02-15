/**********************************************************
 *
 *  bcm2xxx_mb.h
 *
 *  DESCRIPTION:
 *      BCM 283x Mailbox interface definitions
 *
 */

#include "generic.h"
#include "bcm2xxx_mb.h"
#include "peripherals/base.h"
#include "printf.h"
#include "utils.h"
#include "../../drivers/usb/dwc2/dwc2.h"

/* Coherent memory region used in mailbox communication */
#define MEM_COHERENT_REGION	0x400000

#define MB_FULL   0x80000000
#define MB_EMPTY  0x40000000

#define GPU_MEM_BASE	0xC0000000
#define BUS_ADDRESS(phys)	(((phys) & ~0xC0000000) | GPU_MEM_BASE)

/**********************************************************
 *
 *  VideoCore (VC) GPU tag interface types
 *
 *      The BCM 283x Mailbox's main purpose is to faciliate
 *      communication from the ARM part of the CPU to the
 *      VC GPU. This interface includes requests that the
 *      ARM code can make such as powering on various
 *      peripherals.
 *
 *      tag_code_t (uint32_t)
 *
 *          Tag code describes the request and response codes
 *          to/from the GPU. We make a request and it tells us
 *          whether the request was successful or not.
 *
 *      vc_tag_buff_data_t
 *
 *          Tag buffer descriptor type. Contains information
 *          such as the size of the data we are sending to VC
 *          and the tag code.
 */

typedef uint32_t vc_end_tag_t32;

#define TAG_RQUST_TYPE_SET_POWER_STATE 0x00028001
#define TAG_RQUST_END_TAG 0x00000000
#define BCM_MAILBOX_PROP_OUT	8


typedef uint32_t tag_code_t;
enum
{
    TAG_CODE_REQUEST = 0x00000000,
    TAG_CODE_RESP_SUCCESS = 0x80000000,
    TAG_CODE_RESP_FAILURE = 0x80000001,
};

typedef uint32_t vc_device_id;
enum
{
    VC_DEVICE_ID_USB_HCD = 3,
};

typedef uint32_t vc_power_state_t;
enum
{
    /* Power on requests */
    VC_POWER_STATE_OFF      = ( 0 << 0 ),
    VC_POWER_STATE_ON       = ( 1 << 0 ),
    VC_POWER_STATE_WAIT     = ( 1 << 1 ),

    /* Power on respones */
    VC_POWER_STATE_NO_DEVICE= ( 1 << 1 ),
};

typedef struct
{
    uint32_t size;      /* Size of buffer */
    tag_code_t code;    /* Tag comm type code */
    uint8_t data[0];    /* Variably sized tag data */
} vc_tag_buff_data_t;
_Static_assert(sizeof(vc_tag_buff_data_t) == 8, "vc_tag_buff_data_t size is not 8 bytes");

typedef struct
{
uint32_t tag_id;            /* Describes operation */
uint32_t value_buf_size;    /* Size of value buffer (4-byte aligned) */
uint32_t value_len;         /* Actual length ofthe vaue */
} vc_tag_header_t;

typedef struct
{
    vc_tag_header_t tag;    /* Request tag */
    vc_device_id device_id; /* Device id */
    vc_power_state_t
                power_state;/* Power state */
} vc_power_request_t;
// _Static_assert(sizeof(vc_tag_buff_data_t) == 20, "vc_power_request_t size is not 20 bytes");//TODO


/**********************************************************
 *
 *  BCM 283x Mailbox types
 *
 *      mb_channel_t
 *
 *          Mailbox channels used for various communications
 *          See other types in this file which describe the
 *          various interfaces that use the BCM's mailbox
 *
 */

typedef uint32_t mb_channel_t;
enum
{
    POWER_CHNL = 0,
    VC_PROPERTY_TAG_CHNL = 8,
};

/* Register defines */

#define REG_MB_BASE ( PBASE + 0xB880 )
#define REG_MB_1_STATUS *( ( volatile uint32_t * )( REG_MB_BASE + 0x38 ) )
#define REG_MB_0_STATUS *( ( volatile uint32_t * )( REG_MB_BASE + 0x18 ) )
#define REG_MB_1_WRITE_1  *( ( volatile uint32_t * )( REG_MB_BASE + 0x20 ) ) 
#define REG_MB_0_READ *( ( volatile uint32_t * )( REG_MB_BASE + 0x00 ) ) 
#define REG_MB_CHNL_MASK  0xF

/**********************************************************
 *
 *  VideoCore (VC) GPU tag variables
 *
 *      tag_buff_data
 *
 *          Tag buffer data
 *
 */

static vc_tag_buff_data_t * tag_buff_data = ( ( vc_tag_buff_data_t * ) MEM_COHERENT_REGION );

/* Forward declares */
static void write_to_mb(mb_channel_t chnl, uint32_t data);
static uint32_t read_from_mb(mb_channel_t chnl);
static void flush_mb(void);


/**********************************************************
 *
 * process_vc_request()
 *
 *  DESCRIPTION:
 *      Process VideoCore (VC) GPU tag interface requests.
 *
 *  NOTES:
 *      Only processes power on requests at the moment.
 */

static boolean process_vc_request(void)
{
// TODO for now this processes the only request we care about,
// TODO which is powering on the USB Controller.e
uint32_t buffer_size;       /* Size of buff we are sending to VC */
vc_tag_header_t tag_header; /* Tag header */
vc_power_request_t power_state;
                            /* Power state request data */

/* Setup the buffer
 *
 * Size:
 * +---------------------------+
 * | TAG BUFFER INFO (8 B)     |
 * |----------------------------
 * | POWER ON REQUEST (20 B)   |
 * |----------------------------
 * | END TAG (1 B)             |
 * +---------------------------+
 */

buffer_size = sizeof( vc_tag_buff_data_t ) +
              sizeof( vc_power_request_t ) +
              sizeof( vc_end_tag_t32 );

/* Setup header */
tag_header.tag_id = TAG_RQUST_TYPE_SET_POWER_STATE;
tag_header.value_buf_size = sizeof( vc_power_request_t ) - sizeof( vc_tag_header_t );
tag_header.value_len = 8;   // TODO USB/power on specific

/* Set up power state portion of request */
power_state.device_id = 0x3;    // TODO USB only, update this to be more generic
power_state.power_state = ( VC_POWER_STATE_ON | VC_POWER_STATE_WAIT );
power_state.tag = tag_header;

/* Build the buffer */
tag_buff_data->code = TAG_CODE_REQUEST;
tag_buff_data->size = buffer_size;
memcpy( tag_buff_data->data, &power_state, sizeof( vc_power_request_t ) );

/* Set the end tag which lets VC know where end
 * of data is */
vc_end_tag_t32 * end_tag_ptr = ( vc_end_tag_t32 * )( tag_buff_data->data + sizeof( vc_power_request_t ) );
*end_tag_ptr = TAG_RQUST_END_TAG;

/* Get buffer bus address */
uint32_t buff_address = BUS_ADDRESS( ( uint32_t )tag_buff_data );

/* Write the buffer address to the mailbox */
flush_mb();
write_to_mb( VC_PROPERTY_TAG_CHNL, buff_address );

/* Read GPU response. In this case, we would expect the
 * buffer address to be returned.
 */
if( ( read_from_mb( VC_PROPERTY_TAG_CHNL ) != buff_address ) ||
  ( tag_buff_data->code != TAG_CODE_RESP_SUCCESS ) )
{
    printf("\nFailed request!");
    return FALSE;
}

/* Verify power on was succesful */
power_state = *( ( vc_power_request_t * ) tag_buff_data->data );
if( 0 == ( power_state.power_state & VC_POWER_STATE_ON ) || ( power_state.power_state &  VC_POWER_STATE_NO_DEVICE ) )
{
    printf("\nFaied to power on device!");
    return FALSE;
}

return TRUE;
}

static void write_to_mb(mb_channel_t chnl, uint32_t data)
{
    /* Block until mailbox is no longer full */
    while(REG_MB_1_STATUS & MB_FULL)
        ;

    REG_MB_1_WRITE_1 = data | chnl;
}

static uint32_t read_from_mb(mb_channel_t chnl)
{
	uint32_t ret_data;
	
	do
	{
		while ( REG_MB_0_READ & MB_EMPTY )
            ;

		ret_data = REG_MB_0_READ;
	}
	while ( ( ret_data & REG_MB_CHNL_MASK ) != chnl );

    printf("\nRead data from mb %d ", ret_data & ~( REG_MB_CHNL_MASK ));
	return ret_data & ~( REG_MB_CHNL_MASK );
}

static void flush_mb(void)
{
	while ( !( REG_MB_0_STATUS & MB_EMPTY ) )
	{
        printf("\ndata=%d", REG_MB_0_READ);

        delay_ms( 25 );
	}
}

/**
 * bcm2xxx_power_on
 *
 * @brief Power on hardware componen, i.e,. USB, SD, etc.
 *
 * @param type of power on to perform
 */
void bcm2xxx_power_on(bcm2xxx_power_on_t type)
{
    process_vc_request();

    //TODO
    // /* block for now. put this in a task */
    // while(REG_MB[6] & MB_FULL)
    //     ;

    // current_mb_val |= type;
    // REG_MB[8] = ( current_mb_val & ~( 0x0F ) ) | POWER_CHNL;

    // delay(600);

    // #define REG_VBUS *( ( volatile uint32_t * )HPRT0 )
    // uint32_t hprt0 = REG_VBUS;
    // hprt0 &= ~(HPRT0_ENA | HPRT0_CONNDET | HPRT0_ENACHG | HPRT0_OVRCURRCHG);
    // if( !(hprt0 & HPRT0_PWR) )
    // {
    //     printf("\nvbus NOT on. Turning on vbus");
    //     hprt0 |= HPRT0_PWR;
    //     REG_VBUS = hprt0;
    // }
    // else
    // {
    //     printf("\nvbus on");
    // }

    // printf("Successful power on of component %d", type);
}
