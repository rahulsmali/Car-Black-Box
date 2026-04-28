#include <xc.h>
#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "helper.h"
#include "digital_keypad.h"
#include "adc.h"
#include <string.h>
#include "external_eeprom.h"
#include "uart.h"


 unsigned char clock_reg[3];
 char time[7];  // "HH:MM:SS"
 unsigned char log[11];
 unsigned char pos = 0; //0 1 2 3 4 5 6 7 8 9
  unsigned char log_count = 0;
  
 unsigned char blink = 0;
  
 unsigned char return_time, secs ;
 
unsigned char user_pwd[4];
unsigned char svd_pwd[4];
 
 unsigned char *menu[] = {"view log", "clear log", "download log", "set time", "change pwd"};
 
 void read_time(void)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
      
    // HH -> 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
    
    // clcd_print(time, LINE2(0)); // HH:MM:SS 
}

void display_time(void)
{
    clcd_putch(time[0], LINE2(0));
    clcd_putch(time[1], LINE2(1));
    clcd_putch(':', LINE2(2));
  
    clcd_putch(time[2], LINE2(3));
    clcd_putch(time[3], LINE2(4));
    clcd_putch(':', LINE2(5));
    
    clcd_putch(time[4], LINE2(6));
    clcd_putch(time[5], LINE2(7));
    
}

void display_dashboard(unsigned char event[], unsigned char speed)
{
    clcd_print("TIME      EV  SP", LINE1(0));
    read_time();
    display_time();
    
    clcd_print(event, LINE2(10));
    
    clcd_putch(speed/10 + '0', LINE2(14));
    clcd_putch(speed%10 + '0', LINE2(15));
}

void log_event(unsigned char event[], unsigned char speed)
{
    unsigned char add;
    
    add = pos * 10;  //0, 10, 20, 30, 40,...90

    //put time, speed, event, into single string 
    read_time();
    
    strncpy(log, time, 6); //HHMMMSS
    strncpy(&log[6], event, 2);
    
    log[8] = speed/10 + '0';
    log[9] = speed%10 + '0';
    
    log[10] = '\0';
    
    write_external_eeprom_string(add, log); //addres. string with content

    pos++;
    
    if(pos == 10)//if the 11 th event occurs overwrite with older event
    {
        pos = 0;
    }

    if(log_count < 10)
    {
        log_count++;
    }
    
}

void load_password(unsigned char pwd[])
{
    unsigned char i;
    for(i=0; i<4; i++)
    {
       pwd[i] = read_external_eeprom(PASSWORD_ADDR + i);
    }
}

void save_password(unsigned char pwd[])
{
    unsigned char i;
    for(i=0; i<4; i++)
    {
       
        write_external_eeprom(PASSWORD_ADDR + i, pwd[i]);
        __delay_ms(10);
    }
    
}

 void init_password(void)
{
     unsigned char i;
     load_password(svd_pwd);
        
      if(svd_pwd[0] != '0' && svd_pwd[0] != '1')
      {
         svd_pwd[0] = '1';
         svd_pwd[1]= '1';
         svd_pwd[2]= '1';
         svd_pwd[3]= '1';
            
            save_password(svd_pwd);
      }
}
unsigned char check_password(unsigned char key, unsigned char reset_flag)
{
    
    static unsigned char attempt, i;
    
   
    if(reset_flag == RESET_PASSWORD)
    {
        attempt = 3;
        i=0;
        load_password(svd_pwd);
        
        return_time = 5;
        key = 0;
        
        clcd_write(LINE2(5), INST_MODE);
        
    }
    
    
    //read password from user check for timer return to dashboard
    if(key == SW4 && i<4)
    {
        user_pwd[i] = '1';
        //i++;
        clcd_putch('*', LINE2(i+5));
   
        i++;
        //clcd_write(LINE2(i+5), INST_MODE);
        return_time =5;
        
    }
    else if(key == SW5 && i<4)
    {
        user_pwd[i] = '0';
       // i++;
        clcd_putch('*', LINE2(i+5));

        i++;
        
       //clcd_write(LINE2(i+5), INST_MODE);
        return_time = 5;
       
        
    } 
    
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    
    if(i==4)
    {
        if(strncmp(user_pwd, svd_pwd, 4)==0)
        {
            clear_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, 0);
            clcd_print("login success", LINE1(0));
            __delay_ms(1500);
            TMR2ON = 0;
            return RETURN_SUCCESS;
        }
        
        else
        {
            attempt--;
            if(attempt == 0)  //if attempts are over lock the screen for 60 seconds
            {
               clear_screen();
               clcd_write(DISP_ON_AND_CURSOR_OFF, 0);
               clcd_print("Your are blocked", LINE1(0));
               clcd_print("wait for", LINE2(0));
               secs = 60;
               while(secs != 0)
               {
                   clcd_putch(secs/10 + '0', LINE2(9));
                   clcd_putch(secs%10 + '0', LINE2(10));
                   clcd_print("secs", LINE2(12));
               }
               
               attempt = 3;
            }
            
            else// re-enter the  password 
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, 0);
                clcd_print("wrong password", LINE1(0));
                clcd_print("Attempt left", LINE2(0));
                clcd_putch(attempt + '0', LINE2(13));
                __delay_ms(1500);
            }
            
            clear_screen();
            clcd_write(DISP_ON_AND_CURSOR_BLINK, INST_MODE);
            clcd_print("Enter password   ", LINE1(0));
            clcd_write(LINE2(5), INST_MODE);
            i=0;
            return_time = 5;
        }
    }
    
    return 0;
    
}

unsigned char menu_screen(unsigned char reset_flag, unsigned char key)
{
    static unsigned char menu_pos= 0;
    static unsigned char select_pos = 1;
    
    //based on key press scroll up and scroll down
    
    if(reset_flag == RESET_MENU)
    {
        clear_screen();
        menu_pos= 0; 
        select_pos = 1;     
    }
    
    if(key == SW4 && menu_pos < 4)
    {
        menu_pos++;
        clear_screen();
        if(select_pos < 2)
            select_pos++;
        
        return_time = 5;
    }
    
    else if(key == SW5 && menu_pos > 0)
    {
        menu_pos--;
        clear_screen();
        if(select_pos > 1)
            select_pos--;
        
        return_time = 5;
    }
  
   
    if(select_pos == 1)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        
        if(menu_pos < 4)
        clcd_print(menu[menu_pos+1], LINE2(2));
    }
    else 
    {
        clcd_putch('*', LINE2(0));
        if(menu_pos > 0)
        clcd_print(menu[menu_pos-1], LINE1(2));
        
        clcd_print(menu[menu_pos], LINE2(2));
    }
    
    return menu_pos;
    //return the pos of menu selected
}


void view_log(unsigned char key)
{
    static unsigned char first_time = 1;
    static unsigned char index = 0;
    
    unsigned char log[11];
    
    if(first_time)
    {
        clear_screen();
        
        if(log_count == 0)
        {
            clcd_print("No logs", LINE1(0));
            //__delay_ms(2000);
            return;
     
        }
        
        index = log_count - 1;
        
        first_time =0;
    }
    
    
    if(key == SW4)
    {
        index--;
        if(index == 0)
        {
            index = 9;
        }
        
    }
    
    else if(key == SW5)
    {
        index++;
        if(index == 10)
        {
            index =0;
        }
    }
    
    read_log(index, log);
    
    display_log(index, log);
}

void read_log(unsigned char index, unsigned char log[])
{
    unsigned char i;
    unsigned int addr;
    
    addr = index * 10;
    
    for(i=0; i<10; i++)
    {
       log[i] = read_external_eeprom(addr + i);
    }
    
    log[10] = '\0';
}

void display_log(unsigned char index, unsigned char log[])
{
    //clear_screen();
    
    clcd_putch('#', LINE1(0));
    clcd_print("TIME     E  SP", LINE1(2));
    
    //print index
    clcd_putch((index % 10) + '0', LINE2(0));
    
    //print time
    clcd_putch(log[0], LINE2(2));
    clcd_putch(log[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    
    clcd_putch(log[2], LINE2(5));
    clcd_putch(log[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    
    clcd_putch(log[4], LINE2(8));
    clcd_putch(log[5], LINE2(9));
    
    //print event
    clcd_putch(log[6], LINE2(11));
    clcd_putch(log[7], LINE2(12));
    
    //print speed 
    clcd_putch(log[8], LINE2(14));
    clcd_putch(log[9], LINE2(15));
        
    
} 

void clear_log(unsigned char key, unsigned char reset_flag)
{
    static unsigned char first_time = 1;
    if(reset_flag == RESET_CLEAR_LOG)
    {
        first_time = 1;
        
        
    }
    
    if(first_time)
    {
        clear_screen();
       log_count = 0;
       pos = 0;
    
        clcd_print("Clear Log", LINE1(0));
        clcd_print("Successful", LINE2(0));
        
        first_time = 0;
    }
    
    
    
    
}
void download_log(void)
{
    unsigned char i;
    unsigned char log[11];
    
    clear_screen();
    
    clcd_print("Downloading...", LINE1(0));
    
    if(log_count == 0)
    {
       puts("\r\nNo logs available\r\n");
        
        clcd_print("NO logs", LINE2(0));
        
       __delay_ms(2000);
       
       return;
    }
    
    puts("\r\n------------ LOG DATA ------------\r\n");
    
    puts("\r\nIndex  Time       Event  Speed\r\n");
    
    for(i =0; i<log_count; i++)
    {
        read_log(i, log);
        
        putchar('#');
        putchar((i / 10) + '0');
        putchar((i % 10) + '0');
        puts("    ");
       
        putchar(log[0]);
        putchar(log[1]);
        putchar(':');
        
        putchar(log[2]);
        putchar(log[3]);
        putchar(':');
        
        putchar(log[4]);
        putchar(log[5]);
        
        puts("   ");
        
        putchar(log[6]);
        putchar(log[7]);
        
        puts("     ");
        
        putchar(log[8]);
        putchar(log[9]);
        
        puts("  ");
        
        puts("\r\n");
    }
    
    puts("\n");
    puts("---------------------------------\r\n");

    clcd_print("Download Done", LINE2(0));

    __delay_ms(2000);
    
}
unsigned char set_time(unsigned char key)
{
    static unsigned char first_time = 1;
    static unsigned char field = 0;
    
    static unsigned char hour;
    static unsigned char min;
    static unsigned char sec;
    
    if(first_time)
    {
        clear_screen();
        
        clcd_print("TIME (HH:MM:SS)", LINE1(0));
        
        read_time();
        
        hour = (time[0] - '0') * 10 + (time[1] - '0');
        min = (time[2] - '0') * 10 + (time[3] - '0');
        sec = (time[4] - '0') * 10 + (time[5] - '0');
        
        field = 0;
        blink = 0;
        first_time = 0;
      
        
    }
    
    if(field == 2 && blink)
    {
        clcd_print("  ", LINE2(0));
    }
    else 
    {
       clcd_putch((hour / 10) + '0', LINE2(0));
       clcd_putch((hour % 10) + '0', LINE2(1));
    }
    
    clcd_putch( ':', LINE2(2));
    
    if(field == 1 && blink)
    {
      clcd_print("  ", LINE2(3));  
    }
    
    else
    {
      clcd_putch((min / 10) + '0', LINE2(3));
       clcd_putch((min % 10) + '0', LINE2(4));
    }
    
    clcd_putch( ':', LINE2(5));
    
    if(field == 0 && blink)
    {
       clcd_print("  ", LINE2(6)); 
    }
    
    else 
    {
      clcd_putch((sec/ 10) + '0', LINE2(6));
      clcd_putch((sec % 10) + '0', LINE2(7));
    }  
   
    
    
    if(key == SW4)
    {
        if(field == 0)
        {
            sec++;
            
            if(sec == 60)
            {
                sec = 0;
            }
        }
    
        else if(field == 1)
        {
           min++;
        
           if(min == 60)
           {
              min = 0;
           }
       }
    
        else if(field == 2)
        {
           hour++;
        
           if(hour == 24)
           {
               hour = 0;
           }
       }
        
    }
    
    
    if(key == SW5)
    {
        field++;
        
        if(field == 3)
        {
            field = 0;
        }
    }
    
    if(key == L_SW4)
    {
        unsigned char hr_bcd;
        unsigned  char min_bcd;
        unsigned char sec_bcd;
        
        hr_bcd = ((hour / 10) << 4) | (hour % 10);
        min_bcd = ((min / 10) << 4) | (min % 10);
        sec_bcd = ((sec / 10) << 4) | (sec % 10);
        
        write_ds1307(HOUR_ADDR, hr_bcd);
        write_ds1307(MIN_ADDR, min_bcd);
        write_ds1307(SEC_ADDR, sec_bcd);
        
        clear_screen();
        clcd_print("Time changed", LINE1(0));
        clcd_print("successfully", LINE2(0));
        __delay_ms(2000);
        first_time = 1;
        
        TMR2ON = 0;
        return 1;
    }
    
    return 0;
 
}
unsigned char change_password(unsigned char key)
{
    static unsigned char new_pwd[4];
    static unsigned char re_pwd[4];
    static unsigned char i = 0;
    static unsigned char stage = 0;
    static unsigned char first_time = 1;
    
    
    if(first_time)
    {
        clear_screen();
        clcd_print("Enter new pwd", LINE1(0));
        clcd_write(LINE2(5), INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_BLINK, INST_MODE);
        
        i=0;
        stage = 0;
        first_time = 0;
        
    }
    
 
    if(key == SW4 || key == SW5)
    {
        if(stage == 0)
        {
            if(key == SW4)
            {
                new_pwd[i] = '1';
            }
            
            else if(key == SW5)
            {
                new_pwd[i] = '0';
            }
            
            clcd_putch('*', LINE2(i+5));
            
            i++;
            
            if(i==4)
            {
                i=0;
                stage = 1;
                
                clear_screen();
                clcd_print("Re-enter Pwd", LINE1(0));
                clcd_write(LINE2(5), INST_MODE);
                clcd_write(DISP_ON_AND_CURSOR_BLINK, INST_MODE);
            }
        }
    
    
       else if(stage == 1)
       {
           if(key == SW4)
           {
               re_pwd[i] = '1';
           }
        
            else if(key == SW5)
            {
               re_pwd[i] = '0';
            }
        
           clcd_putch('*', LINE2(i+5));
        
            i++;
        
           if(i==4)
           {
                if(strncmp(new_pwd, re_pwd, 4) == 0)
                {
                   save_password(new_pwd);
                   load_password(new_pwd);
                 
                    clear_screen();
                     clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    clcd_print("Pwd updated", LINE1(0));
                    clcd_print("successfully", LINE2(0));
                    
                }
            
                else
                {
                   clear_screen();
                   clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                   clcd_print("Pwd mismatch", LINE1(0));

                }
               
               __delay_ms(2000);
            
               first_time = 1;
               i=0;
               stage = 0;
               
               return 1;
            }
        }
    }
    
}
