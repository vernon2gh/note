#include <stdio.h>
#include <gtk/gtk.h>

void hello(GtkWidget *widget, gpointer data)
{
	g_print("enter %s\n", __func__);

}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	g_print("enter %s\n", __func__);

	return TRUE;
	/* return FALSE; //call destroy event function */
}

void destroy(GtkWidget *widget, gpointer data)
{
	g_print("enter %s\n", __func__);
	gtk_main_quit();
}

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *button;

	g_print("entry %s\n", __func__);

	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "hello world title");

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	
	gtk_container_set_border_width(GTK_CONTAINER(window), 150);
	button = gtk_button_new_with_label("hello world label");

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), window);

	gtk_container_add(GTK_CONTAINER(window), button);

	gtk_widget_show(window);
	gtk_widget_show(button);
	gtk_main();

	g_print("leave %s\n", __func__);
	return 0;
}

