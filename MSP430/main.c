#include <msp430.h> 
#include <stdlib.h>
#include <string.h>


// uC GPIO Port assignment
#define UC_PORT     P1OUT
#define UC_PORT_DIR P1DIR

// Peripheral pin assignments
#define US_TRIG         BIT0
#define US_ECHO         BIT1
#define LCD_EN          BIT2
#define LCD_RS          BIT3
#define LCD_DATA        BIT4 | BIT5 | BIT6 | BIT7
#define LCD_D0_OFFSET   4   // D0 at BIT4, so it is 4
#define US_MASK         US_TRIG | US_ECHO
#define LCD_MASK        LCD_EN | LCD_RS | LCD_DATA

unsigned int up_counter;

void initialize_port2(){
    P2DIR &= (~BIT0);
    P2DIR &= (~BIT1);
    P2DIR &= (~BIT2);
}

void lcd_reset()
{
    UC_PORT = 0x00;
    __delay_cycles(20000);

    UC_PORT = (0x03 << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;
    __delay_cycles(10000);

    UC_PORT = (0x03 << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;
    __delay_cycles(1000);

    UC_PORT = (0x03 << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;
    __delay_cycles(1000);

    UC_PORT = (0x02 << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;
    __delay_cycles(1000);

}

void lcd_cmd (char cmd)
{
    // Send upper nibble
    UC_PORT = (((cmd >> 4) & 0x0F) << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;

    // Send lower nibble
    UC_PORT = ((cmd & 0x0F) << LCD_D0_OFFSET) | LCD_EN;
    UC_PORT &= ~LCD_EN;

    __delay_cycles(4000);
}

void lcd_data (unsigned char dat)
{
    // Send upper nibble
    UC_PORT = ((((dat >> 4) & 0x0F) << LCD_D0_OFFSET) | LCD_EN | LCD_RS);
    UC_PORT &= ~LCD_EN;

    // Send lower nibble
    UC_PORT = (((dat & 0x0F) << LCD_D0_OFFSET) | LCD_EN | LCD_RS);
    UC_PORT &= ~LCD_EN;

    __delay_cycles(4000); // a small delay may result in missing char display
}

void lcd_init ()
{
    lcd_reset();         // Call LCD reset

    lcd_cmd(0x28);       // 4-bit mode - 2 line - 5x7 font.
    lcd_cmd(0x0C);       // Display no cursor - no blink.
    lcd_cmd(0x06);       // Automatic Increment - No Display shift.
    lcd_cmd(0x80);       // Address DDRAM with 0 offset 80h.
    lcd_cmd(0x01);       // Clear screen

}

void display_line(char *line)
{
    while (*line)
        lcd_data(*line++);
}

void display_distance(char *line, int len)
{
    while (len--)
        if (*line)
            lcd_data(*line++);
        else
            lcd_data(' ');
}

/* A utility function to reverse a string  */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        char *temp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
        start++;
        end--;
    }
}
// Implementation of itoa()
char* itoa(unsigned int num, char* str, int base)
{
    unsigned int i = 0;
    int isNegative = 1;
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = -1;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        unsigned int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative < 0)
        str[i++] = '-';


    str[i] = '\0'; // Append string terminator

    reverse(str, i);
    return str;
}

int main()
{
    WDTCTL = WDTPW + WDTHOLD;       // Stop Watch Dog Timer
    UC_PORT_DIR = LCD_MASK;         // Output direction for LCD connections
    UC_PORT_DIR |= US_TRIG;         // Output direction for trigger to sensor

    UC_PORT &= ~US_TRIG;            // keep trigger at low
    P1SEL = US_ECHO;                // set US_ECHO as trigger for Timer from Port-1

    __delay_cycles(1600000);
    // Initialize LCD
    lcd_init();

    initialize_port2();

    lcd_cmd(0x80); // select 1st line (0x80 + addr) - here addr = 0x00
    display_line("**AUDIOVISIBLE**");
    //lcd_cmd(0xce); // select 2nd line (0x80 + addr) - here addr = 0x4e
    //display_line("");

    lcd_cmd(0xc0);
    display_line("Initializing... ");
    __delay_cycles(1600000);

    while (1)
    {
        lcd_cmd(0xc0);
        if(P2IN&4){
            // Clear
            display_line("Initializing... ");
            __delay_cycles(1600000);
        }
        else if(P2IN&1){
            display_line("     Ready      ");
            __delay_cycles(1600000);
        }
        else if(P2IN&2){
            display_line("    Playing     ");
            __delay_cycles(1600000);
        }
    }
}
