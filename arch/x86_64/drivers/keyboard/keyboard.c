#include <arch/x86_64/drivers/keyboard.h>
#include <kernel/scheduler/scheduler.h>

extern thread_t *read_blocked_thread;

int shift;
int capslock;
int ext;
int backspace;

const char *lowercase[] = {
    "UNKNOWN","ESC","1","2","3","4","5","6","7","8",
    "9","0","-","=","\b","\t","q","w","e","r",
    "t","y","u","i","o","p","[","]","\n","CTRL",
    "a","s","d","f","g","h","j","k","l",";",
    "\"","`","LSHFT","\\","z","x","c","v","b","n","m",",",
    ".","/","RSHFT","*","ALT"," ","CAPS","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NUMLCK","SCRLCK","HOME","UP","PGUP","-","LEFT","UNKNOWN","RIGHT",
    "+","END","DOWN","PGDOWN","INS","DEL","UNKNOWN","UNKNOWN","UNKNOWN","F11","F12","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN"
};

const char *uppercase[] = {
    "UNKNOWN","ESC","!","@","#","$","%","^","&","*","(",")","_","+","\b","\t","Q","W","E","R",
    "T","Y","U","I","O","P","{","}","\n","CTRL","A","S","D","F","G","H","J","K","L",":","\"","~","LSHFT","|","Z","X","C",
    "V","B","N","M","<",">","?","RSHFT","*","ALT"," ","CAPS","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NUMLCK","SCRLCK","HOME","UP","PGUP","-",
    "LEFT","UNKNOWN","RIGHT","+","END","DOWN","PGDOWN","INS","DEL","UNKNOWN","UNKNOWN","UNKNOWN","F11","F12","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN",
    "UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN","UNKNOWN"
};

const char* extended[] = {
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "Keypad Enter", "RCtrl", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "Keypad /", "", "", "RAlt", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "Home", "Up Arrow", "Page Up", "", "Left Arrow", "", "Right Arrow", "", "End",
    "Down Arrow", "Page Down", "Insert", "Delete", "", "", "", "", "", "", "", "Left Super", "Right Super", "Apps/Menu", "Power", "Sleep",
    "", "", "", "Wake", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

void keyboard_handler(struct InterruptRegisters *regs){
    uint8_t rawCode = inb(0x60);
    uint8_t scanCode = rawCode & 0x7F;
    int press = rawCode & 0x80;

    if (rawCode == 0xE0) { ext = 1; return; }

    switch(scanCode){
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
            break;
        
        case 14:
            if (!press){
                backspace = 1;
                write_kring[write_kring_head & KRING_BUF_MASK].ready = 1;
                if (read_kring_tail == read_kring_head--) read_kring_tail--;
                if (write_kring_tail == write_kring_head--) write_kring_tail--;

                kputchar_direct('\b');
            }
            break;
        
        case 28:
            if (!press){
                kring_write("\n", 1, 0);
                kring_write("\n", 1, 1);
                if (read_blocked_thread){
                    read_blocked_thread->state = READY;
                    enqueue_thread(read_blocked_thread);
                    read_blocked_thread = NULL;
                }
            }
            break;
        
        case 42:
            if (press) shift = 0;
            else shift = 1;
            break;

        case 58:
            if (capslock && press) capslock = 0;
            else if (!capslock && press) capslock = 1;
            break;
        
        default:
            if (!press){
                if (ext){
                    ext = 0;
                    char *key = extended[scanCode];
                    int len = strlen(key);
                    kring_write(key, len, 0);
                    kring_write(key, len, 1);
                }
                else {
                    char *key = ((!shift && !capslock) || (shift && capslock)) ? lowercase[scanCode] : uppercase[scanCode];
                    int len = strlen(key);
                    kring_write(key, len, 0);
                    kring_write(key, len, 1);
                }
            }
            break;
    }

    return;
}

void init_keyboard(void){
    shift = 0;
    capslock = 0;
    ext = 0;
    backspace = 0;
    setIRQHandler(1, &keyboard_handler);
    ioapic_set_irq(1, 0x21, 0);
}