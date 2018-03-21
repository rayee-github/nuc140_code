#ifndef __LCD_Driver_H__
#define __LCD_Driver_H__
     
extern void SysTimerDelay(uint32_t us);
extern void Initial_panel(void);
extern void Disable_Buzzer(void);

extern void Show_Word(unsigned char x, unsigned char y,unsigned char ascii_word);

extern void print_lcd(unsigned char line, char *str);


extern void clr_all_panel(void);

extern void draw_LCD(unsigned char *buffer);

#endif
