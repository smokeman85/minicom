
#include "minicom.h"
#include "intl.h"
#include "scheme_inter.h"
#include "scheme.h"
#include "scheme-private.h"

void scheme_script_run()
{
	WIN *w;
	int done = 0;
	int n;

	FILE *finit;
	scheme sc;
	char script[256];

	scheme_init(&sc);
	
	scheme_set_input_port_file(&sc, stdin);
	scheme_set_output_port_file(&sc, stdout);

        finit = fopen("init.scm", "r");
	scheme_load_file(&sc, finit);
	fclose(finit);
	
	w = mc_wopen(10, 5, 70, 10, BDOUBLE, stdattr, mfcolor, mbcolor, 0, 0, 1);
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
