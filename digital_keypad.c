#include <xc.h>
#include "digital_keypad.h"
#include "clcd.h"

void init_digital_keypad(void)
{
    /* Set Keypad Port as input */
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES;
}

unsigned char read_digital_keypad(unsigned char mode)
{
    static unsigned char once = 1;
    static unsigned char long_press = 0;
    static unsigned char prev_key = ALL_RELEASED;
    
    unsigned char key;

    if (mode == LEVEL)
    {
        return KEYPAD_PORT & INPUT_LINES;
    }
    else
    {
        //if key is pressed for long time  L_SW4 = SW4 | 0x80, L_SW5 = SW5 | 0x80
        key = KEYPAD_PORT & INPUT_LINES;
        if((key != ALL_RELEASED) && once)
        {
            once = 0;
            prev_key = key;
            long_press = 0;
        }
        
        else if(key != ALL_RELEASED)
        {
             if(prev_key != key)
              {
                 prev_key = key;
                 long_press = 0;
              }
            if(long_press < 30)
            {
                __delay_ms(20);
                long_press++;
            }

            if(long_press == 30)
            {
                long_press++;   // prevent repeat
                return prev_key | 0x80;   // Long press
            }
        }

        else if(key == ALL_RELEASED && once == 0)
        {
            once = 1;

            if(long_press < 30)
            {
                return prev_key;   // Short press
            }

            prev_key = ALL_RELEASED;
        }
    } 
    
    return ALL_RELEASED;
}
