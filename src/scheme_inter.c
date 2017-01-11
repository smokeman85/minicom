#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include "minicom.h"
#include "intl.h"
#include "scheme_inter.h"
#include "scheme.h"
#include "scheme-private.h"

extern char *__progname;
#define A_BUTTON_MSG "A - script:"
#define WINDOW_MSG "Run scheme script (Return to run, ESC to stop)"

static void debug(char *s, ...)
{
	char lala[80];
	va_list ap;

	va_start(ap, s);
	vsnprintf(lala, sizeof(lala), s, ap);
	va_end(ap);
	write(2, lala, strlen(lala));
}

static int get_path_scheme(char *app_path, char *init_file)
{
	ssize_t count;

	memset(app_path, 0, PATH_MAX);
	count = readlink("/proc/self/exe", app_path, PATH_MAX);
	if(count == -1)
		return -1;

	app_path[count - strlen(__progname)] = '\0';
	strcat(app_path, init_file);

	return 0;
}

static void scheme_script_init(scheme *sc, char *init_file)
{
	FILE *finit;

	scheme_init(sc);

	scheme_set_input_port_file(sc, stdin);
	scheme_set_output_port_file(sc, stdout);

	finit = fopen(init_file, "r");
	scheme_load_file(sc, finit);
	fclose(finit);
}

static void scheme_print_value(WIN *w, pointer value)
{
	mc_wlocate(w, 1, 6);
	if (is_string(value))
		mc_wprintf(w, "Result: %s", string_value(value));
	else if (is_integer(value))
		mc_wprintf(w, "Result: %ld", ivalue(value));
	else if (is_real(value))
		mc_wprintf(w, "Result: %f", rvalue(value));
}

static WIN * scheme_init_gui()
{
	WIN *w;

	w = mc_wopen(10, 15, 70, 20, BDOUBLE, stdattr, mfcolor, mbcolor, 0, 0, 1);
	mc_wtitle(w, TMID, WINDOW_MSG);
	mc_wputs(w, "\n");
	mc_wprintf(w, "%s %s\n", A_BUTTON_MSG, "");
	mc_wredraw(w, 1);

	return w;
}

void scheme_script_run()
{
	WIN *w;
	int done = 0;
	int n;

	scheme sc;
	char script[256];
	char app_path[PATH_MAX];

	if (get_path_scheme(app_path, "init.scm") == -1)
		return;

	scheme_script_init(&sc, app_path);

	w = scheme_init_gui();
	while (!done) {
	        mc_wlocate(w, mbslen(A_BUTTON_MSG) + 1, 1);
		n = wxgetch();
		if (islower(n))
			n = toupper(n);
		switch (n) {
		case 27: /* ESC */
			mc_wclose(w, 1);
			done = 1;
			break;
		case 'A':
			mc_wlocate(w, mbslen(A_BUTTON_MSG) + 1, 1);
			mc_wgets(w, script, 256, sizeof(script));
			break;
		case '\r':
		case '\n':
			if (script[0] == '\0') {
				mc_wbell();
				break;
			}
			scheme_load_string(&sc, script);
			scheme_print_value(w, sc.value);
			break;	
		}
		mc_wflush();
	}

	scheme_deinit(&sc);
}
