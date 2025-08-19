#ifdef __GNUC__
#warning "this file includes <sys/termios.h> which is deprecated, use <termios.h> instead"
#endif
#include <termios.h>

/* Additional termios constants for Unikraft compatibility */
#ifndef VEOF
#define VEOF        0   /* EOF character */
#endif
#ifndef VEOL
#define VEOL        1   /* EOL character */  
#endif
#ifndef VDISCARD
#define VDISCARD    15  /* DISCARD character */
#endif
#ifndef VMIN
#define VMIN        16  /* MIN value (minimum chars) */
#endif
#ifndef VTIME
#define VTIME       17  /* TIME value (timeout) */
#endif

/* Terminal control constants */
#ifndef TCSBRK
#define TCSBRK      0   /* send break */
#endif
#ifndef TCSBRKP  
#define TCSBRKP     1   /* send break with duration */
#endif
#ifndef TCFLSH
#define TCFLSH      2   /* flush */
#endif
#ifndef TCDRAIN
#define TCDRAIN     3   /* drain */
#endif

