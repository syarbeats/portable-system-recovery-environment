#include <psre/m029/layout.h>
#include <psre/serial.h>

static bool shifted(uint8_t m) { return (m & (PSRE_M029_MOD_LSHIFT|PSRE_M029_MOD_RSHIFT)) != 0u; }
static bool controlled(uint8_t m) { return (m & (PSRE_M029_MOD_LCTRL|PSRE_M029_MOD_RCTRL)) != 0u; }

char psre_m029_translate(uint8_t u, uint8_t m, uint8_t leds) {
    bool shift = shifted(m);
    bool caps = (leds & PSRE_M029_LED_CAPS_LOCK) != 0u;
    if (u >= 0x04u && u <= 0x1du) {
        char c = (char)('a' + (u - 0x04u));
        if (shift != caps) c = (char)(c - 'a' + 'A');
        if (controlled(m)) return (char)(c - (c >= 'a' ? 'a' : 'A') + 1);
        return c;
    }
    if (u >= 0x1eu && u <= 0x26u) {
        static const char normal[] = "123456789";
        static const char upper[]  = "!@#$%^&*(";
        return shift ? upper[u-0x1eu] : normal[u-0x1eu];
    }
    if (u == 0x27u) return shift ? ')' : '0';
    switch (u) {
        case 0x28u: return '\n';       /* Enter */
        case 0x29u: return 0x1bu;       /* Escape */
        case 0x2au: return '\b';       /* Backspace */
        case 0x2bu: return '\t';       /* Tab */
        case 0x2cu: return ' ';
        case 0x2du: return shift ? '_' : '-';
        case 0x2eu: return shift ? '+' : '=';
        case 0x2fu: return shift ? '{' : '[';
        case 0x30u: return shift ? '}' : ']';
        case 0x31u: return shift ? '|' : '\\';
        case 0x32u: return shift ? '|' : '\\'; /* Non-US hash/backslash fallback */
        case 0x33u: return shift ? ':' : ';';
        case 0x34u: return shift ? '"' : '\'';
        case 0x35u: return shift ? '~' : '`';
        case 0x36u: return shift ? '<' : ',';
        case 0x37u: return shift ? '>' : '.';
        case 0x38u: return shift ? '?' : '/';
        case 0x54u: return '/';
        case 0x55u: return '*';
        case 0x56u: return '-';
        case 0x57u: return '+';
        case 0x58u: return '\n';
        case 0x59u: return '1'; case 0x5au: return '2'; case 0x5bu: return '3';
        case 0x5cu: return '4'; case 0x5du: return '5'; case 0x5eu: return '6';
        case 0x5fu: return '7'; case 0x60u: return '8'; case 0x61u: return '9';
        case 0x62u: return '0'; case 0x63u: return '.';
        default: return 0;
    }
}

const char *psre_m029_layout_name(void) { return "US QWERTY"; }
void psre_m029_report(void) {
    serial_write("\n--- M029 KEYBOARD LAYOUT ---\nLayout=US QWERTY\nASCII mapping=enabled\nShift/Caps Lock=enabled\nControl-letter=ASCII control codes\nSpecial keys=Enter, Escape, Backspace, Tab\nKeypad=numeric basic mapping\n");
}
