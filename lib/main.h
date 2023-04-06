#ifndef PAPIRUS_LIB_MAIN_H
#define PAPIRUS_LIB_MAIN_H

typedef struct ppr_App *ppr_App;

ppr_App ppr_new(const char *title, int width, int height);
void ppr_free(ppr_App app);

int ppr_isRunning(ppr_App app);
void ppr_pollEvents(ppr_App app);

#endif // PAPIRUS_LIB_MAIN_H
