
#include <xc.h>
#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "helper.h"
#include "digital_keypad.h"
#include "adc.h"
#include <string.h>
#include "timers.h"
#include "external_eeprom.h"
#include "uart.h"

#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)

unsigned char *gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
extern unsigned char return_time;
static unsigned char first_entry;

static void init_config(void) 
{
    init_i2c(100000); //100K 
    init_ds1307();
    init_adc();
    init_digital_keypad();      
    init_timer2();
    init_clcd();
    init_uart(9600);
    
    PEIE =1;
    GIE = 1;
}

void main(void)
{
    init_config();
    init_password();
    unsigned char screen_flag = DASH_BOARD;
    unsigned char reset_flag = RESET_NOTHING;
    unsigned char speed = 0;
    unsigned char event[3] = "ON"; 
    unsigned char key;
    unsigned char gr = 0;
    unsigned char menu_pos=0;
      static unsigned char first_time;

  
    while(1)
    {
       
        //read speed
        speed = (unsigned char) (read_adc(CHANNEL0) / 10);
        if(speed > 99)
        {
            speed = 99;
        }
     
        //detect events based on  key press
        key = read_digital_keypad(STATE);
        //__delay_us(100);
        
        if(key == SW1)
        {
            strcpy(event, "C ");
            //store the event, speed and time into external eeprom
            log_event(event, speed);
        }
        
        if(key == SW2)
        {
            if(gr < 5)  
            {
               gr++;
               strcpy( event,gear[gr]);           
               log_event(event, speed);
            
            }
        }
        
        else if(key == SW3)
        {
            if(gr > 0)
            {
                gr--;
               strcpy( event,gear[gr]);
               log_event(event, speed);
            }
            
        }
        
        else if((key == SW5 || key == SW4) && screen_flag == DASH_BOARD)
        {
            screen_flag = PASSWORD_SCREEN;
            reset_flag = RESET_PASSWORD;
            clear_screen();
            clcd_print("Enter password   ", LINE1(0));
            clcd_write(DISP_ON_AND_CURSOR_BLINK, INST_MODE);
             TMR2ON = 1;
        }
        

        //based on screen flag display particular screen 
        
        switch(screen_flag)
        {
           
            case DASH_BOARD:
                      display_dashboard(event, speed);
                      break;
                
            case PASSWORD_SCREEN:
                         switch(check_password(key, reset_flag))
                         {
                              case RETURN_BACK:
                                         screen_flag = DASH_BOARD;
                                         clear_screen();
                                         clcd_write(DISP_ON_AND_CURSOR_OFF, 0);
                                         TMR2ON = 0;
                                         break;
                        
                              case RETURN_SUCCESS:
                                          screen_flag = MENU_FLAG;
                                          reset_flag = RESET_MENU;
                                          first_entry = 1;
                                          clear_screen();
                                          TMR2ON= 0;
                                          break;              
                               break;
                          }
                        break;
                        
            case MENU_FLAG:

                       if(reset_flag == RESET_MENU || first_entry)
                        {
                            TMR2ON = 1;
                            return_time = 5;
                            clear_screen();
                            first_entry = 0;
                            reset_flag = RESET_NOTHING;
                        }

                       menu_pos = menu_screen(reset_flag, key);   
                       
                         if(read_digital_keypad(LEVEL) != ALL_RELEASED)
                          {
                             return_time = 5;
                         }
                       //if menu screen, long press of the switch update the screen
                      if(screen_flag == MENU_FLAG && (key == L_SW4 || key == L_SW5))
                      {
                          TMR2ON = 0;
                          first_entry = 1;
                          return_time = 5;
                          clear_screen();
                 
                          while(read_digital_keypad(STATE) != ALL_RELEASED);
                          
                          switch(menu_pos)
                          {
                              
                              case 0:
                                  screen_flag = VIEW_LOG;
                                 first_time=1;
                                  break;
                    
                               case 1:
                                  screen_flag = CLEAR_LOG;
                                  reset_flag = RESET_CLEAR_LOG;
                                  break;
                     
                               case 2:
                                  screen_flag = DOWNLOAD_LOG;
                                 reset_flag =RESET_DOWNLOAD;
                                 break;
                               case 3:
                                  screen_flag = SET_TIME;
                                 // reset_flag = RESET_SET_TIME;
                                 break;
                    
                               case 4:
                                   screen_flag = CHANGE_PWD;
                                   //reset_flag = RESET_CHANGE_PWD;
                                   break;
                           }
                          
                          break;
           
                        }
                       
                       if(return_time == 0)
                       {
                          screen_flag = DASH_BOARD;
                          clear_screen();
                          first_entry = 1; 
                          TMR2ON = 0;
                          break;
                      }
                     break;
                     
            case VIEW_LOG:
                view_log(key);
                if(key == L_SW4)
                {
                     while(read_digital_keypad(STATE) != ALL_RELEASED);

                     screen_flag = MENU_FLAG;
                     reset_flag  = RESET_MENU;
                     first_entry = 1;
                     first_time  = 1;
        
                      clear_screen();
                }
                

               /* Long press DOWN ? Dashboard */
               else if(key == L_SW5)
               {
                   while(read_digital_keypad(STATE) != ALL_RELEASED);

                   screen_flag = DASH_BOARD;
                   first_time  = 1;
 
                  clear_screen();
               }
                
                break;
                
            case CLEAR_LOG:
                //unsigned char ret;
                clear_log(key, reset_flag);

               if(key == L_SW4)
               {
                    while(read_digital_keypad(STATE) != ALL_RELEASED);
                    //first_time = 1;
                   clear_screen();
                   screen_flag = MENU_FLAG;
                   reset_flag = RESET_MENU;
               }
    
                else if(key == L_SW5)
               {
                   while(read_digital_keypad(STATE) != ALL_RELEASED);
                  clear_screen();
                  //first_time = 1;
                  screen_flag = DASH_BOARD;
               }
               break;    
                
            case DOWNLOAD_LOG:
                if(reset_flag == RESET_DOWNLOAD)
                  {
                        download_log();
                  }
                clear_screen();
                screen_flag = MENU_FLAG;
                screen_flag = MENU_FLAG;;
                break;
                
            case SET_TIME:
                   TMR2ON = 1;
                  if(set_time(key))
                  {
                      screen_flag = MENU_FLAG;
                      reset_flag = RESET_MENU;
                      first_entry = 1;
                      
                      clear_screen();
                  }
                  break;
                
            case CHANGE_PWD:
                     if(change_password(key) == 1)
                        {
                            screen_flag = MENU_FLAG;
                            reset_flag  = RESET_MENU;
                            first_entry = 1;
                           
                            clear_screen();
                       }
                   break;

        }
        
       reset_flag = RESET_NOTHING;
      
    }
    
    return;
}
