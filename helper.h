
#ifndef HELPER_H
#define	HELPER_H

#define DASH_BOARD         0x01
#define MENU_FLAG          0x02
#define PASSWORD_SCREEN    0x03
#define VIEW_LOG           0x04
#define CLEAR_LOG          0x05
#define DOWNLOAD_LOG       0x06
#define SET_TIME           0x07
#define CHANGE_PWD         0x08

#define RESET_PASSWORD     0x13
#define RESET_MENU         0x12
#define RESET_NOTHING      0x00
#define RESET_VIEW_LOG     0x14
#define RESET_DOWNLOAD     0x15
#define RESET_CLEAR_LOG    0x16

#define RETURN_SUCCESS     0xA0
#define RETURN_BACK        0xA1

#define PASSWORD_ADDR     100

void display_dashboard(unsigned char event[], unsigned char speed);
void log_event(unsigned char event[], unsigned char speed);
unsigned char check_password(unsigned char key, unsigned char reset_flag);

unsigned char menu_screen(unsigned char reset_flag, unsigned char key);
void view_log(unsigned char key);

void display_log(unsigned char index, unsigned char log[]);
void read_log(unsigned char index, unsigned char log[]);

unsigned char change_password(unsigned char key);

void init_password(void);

unsigned char set_time(unsigned char key);

void download_log(void);

void clear_log(unsigned char key, unsigned char reset_flag);



#endif	/* HELPER_H */

