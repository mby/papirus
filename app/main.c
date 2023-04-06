#include "main.h"

int main()
{
	ppr_App app = ppr_new("Papirus", 640, 480);

	while (ppr_isRunning(app)) {
		ppr_pollEvents(app);
	}

	ppr_free(app);
}
