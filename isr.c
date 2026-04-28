#include <xc.h>
#include "timers.h"

extern unsigned char return_time;
extern unsigned char secs;
extern unsigned char blink;

void __interrupt() isr(void)
{
    static unsigned int count = 0;
    
    if (TMR2IF == 1)
    {
        count++;
        
        if(count == 10000)
        {
            blink = 1;
        }
        if (count == 20000)
        {
            count = 0;
            
            blink = 0;
            
           if(return_time > 0)
               return_time--;
           
            if(secs > 0)
                secs--;
        }
        
        TMR2IF = 0;
    }
}