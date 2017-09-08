#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <libguile.h>
#include <guile/gh.h>
#include "minicom.h"
#include "intl.h"
#include "repl.h"

#define FUNC_SCHEME_SCRIPT "/etc/minicom/func.scm"

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
		mc_wprintf(repl, "\n> unknown\n");
}

static SCM eval_string_catch_handler (const char *string, SCM key, SCM args)
{
	scm_write(key, scm_current_error_port());
	scm_write(args, scm_current_error_port());
	printf("scm_c_eval_string of \"%s\" failed", string); /* TODO: print to repl */

	return SCM_BOOL_F;
}

static void load_scheme_func(WIN *repl)
{
	if (!access(FUNC_SCHEME_SCRIPT, F_OK) == -1) {
		mc_wprintf(repl, "%s not found\n", FUNC_SCHEME_SCRIPT);
		return;
	}

	scm_c_primitive_load(FUNC_SCHEME_SCRIPT);
}

static void eval_guile(WIN *repl, char *script)
{
	SCM ret_val;

	ret_val = scm_c_catch (SCM_BOOL_T,
			       (scm_t_catch_body) scm_c_eval_string,
			       (void *) script,
			       (scm_t_catch_handler) eval_string_catch_handler,
			       (void *) script,
			       (scm_t_catch_handler) NULL,
			       (void *) NULL);

	print_scm(repl, ret_val);
}

static int check_brackets_balance(char *str)
{
	int i, count = 0;
	int str_len = strlen(str);

	for (i = 0; i <= str_len; i++) {
		if (str[i] == '(')
			count++;
		else if (str[i] == ')')
			count--;
	}

	return count;
}

void run_repl(void)
{
	int done = 0, n;
	WIN *repl;
	char script[256];
	
	memset(script, 0, sizeof(script));
	repl = init_repl();

	scm_init_guile();
	load_scheme_func(repl);
	
	while (!done) {
		n = wxgetch();

		switch (n) {
		case 27:
			done = 1;
			mc_wclose(repl, 1);
			break;
		case '\r':
		case '\n':
			if (check_brackets_balance(script)) {
				mc_wprintf(repl, "\n");
				break;
			}
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
