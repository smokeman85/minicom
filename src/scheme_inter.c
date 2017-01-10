#include <unistd.h>
#include <limits.h>
#include "minicom.h"
#include "intl.h"
#include "scheme_inter.h"
#include "scheme.h"
#include "scheme-private.h"

extern char *__progname;

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
	
	w = mc_wopen(10, 15, 70, 20, BDOUBLE, stdattr, mfcolor, mbcolor, 0, 0, 1);
	mc_wtitle(w, TMID, "Run scheme script");
	mc_wputs(w, "\n");
	mc_wprintf(w, "%s %s\n", "A - script:", "");
	mc_wlocate(w, 4, 5);
	mc_wputs(w, "Return to run, ESC to stop");
	mc_wredraw(w, 1);

	while (!done) {
		mc_wlocate(w, mbslen ("Return to run, ESC to stop") + 5, 5);
		n = wxgetch();
		if (islower(n))
			n = toupper(n);
		switch (n) {
		case 27: /* ESC */
			mc_wclose(w, 1);
			done = 1;
			break;
		case 'A':
			mc_wlocate(w, mbslen("A - script:") + 1, 1);
			mc_wclreol(w);
			scr_user[0] = 0;
			mc_wgets(w, script, 32, 256);
			break;
		case '\r':
		case '\n':
			if (script[0] == '\0') {
				mc_wbell();
				break;
			}
			scheme_load_string(&sc, script);
			mc_wprintf(w, "%d\n", ivalue(sc.value));
			mc_wredraw(w, 1);
			break;	
		}
	}

	scheme_deinit(&sc);
}
