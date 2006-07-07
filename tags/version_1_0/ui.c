#include <pthread.h>
#include <stdlib.h>
#include <ncurses.h>
#include "ui.h"

WINDOW *win; /** manejaremos una única ventana de pantalla completa **/

static void IniVideo(){
	win=initscr(); /* Crea la ventana */
        clear();  /* Borra la pantalla entera bajo ncurses */
        refresh(); /* Actualiza la ventana con los cambios */
        noecho();
        cbreak();
        keypad(win, TRUE);
}

void ui_init(void){
	IniVideo();
}

void ui_finish(void) {
	refresh();
	endwin();
}

void ui_draws(int x, int y, char *cadena) {
	move(y,x);
	printw("%s",cadena);
	ui_refresh();
}

void ui_drawi(int x, int y, int valor) {
	move(y,x);
	printw("%d",valor);
	ui_refresh();
}

void ui_refresh(void) {
	clrtoeol();
	refresh();
}
