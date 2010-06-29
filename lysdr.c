/* lysdr.c */

#include <gtk/gtk.h>

extern void gui_display();

int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    gtk_init(&argc, &argv);
    gui_display();
    gtk_main();
}
