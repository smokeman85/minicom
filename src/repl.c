#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <libguile.h>
#include <guile/gh.h>
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

static void print_scm(WIN *repl, SCM val)
{
	size_t len;

	if (scm_is_integer(val))
		mc_wprintf(repl, "\n> %d\n", scm_num2int(val, 0, NULL ));
	else if (scm_is_string(val))
		mc_wprintf(repl, "\n> %s\n", gh_scm2newstr(val, &len));
	else
		mc_wprintf(repl, "\n unknown\n");
}

static void eval_guile(WIN *repl, char *script)
{
	SCM ret_val;

	scm_init_guile();

	ret_val = scm_c_eval_string(script);

	print_scm(repl, ret_val);
}

void run_repl(void)
{
	int done = 0, n;
	WIN *repl;
	char script[256];
	
	memset(script, 0, sizeof(script));
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
			eval_guile(repl, script);
			memset(script, 0, sizeof(script));
			break;
		default:
		        script[strlen(script)] = (char)n;
			mc_wprintf(repl, "%c", (char)n);
			break;
		}
		mc_wflush();
	}
}
