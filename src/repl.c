#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include "minicom.h"
#include "intl.h"
#include "repl.h"


static WIN* init_repl()
{
	WIN *repl;

	repl = mc_wopen(0, LINES - 10, COLS - 10, LINES - 1, BDOUBLE,
			stdattr, mfcolor, mbcolor, 1, 0, 1);
	mc_wredraw(repl, 1);

	return repl;
}

void run_repl(void)
{
	int done = 0, n;
	WIN *repl;
	
	repl = init_repl();
	
	while (!done) {
		n = wxgetch();

		switch (n) {
		case 27:
			done = 1;
			mc_wclose(repl, 1);
			break;
		case '\r':
		case '\n':
			mc_wprintf(repl, "\n");
			break;
		default:
			mc_wprintf(repl, "%c", (char)n);
			break;
		}
		mc_wflush();
	}
}
