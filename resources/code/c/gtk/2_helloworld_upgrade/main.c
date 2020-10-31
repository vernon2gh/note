#include <stdio.h>
#include <gtk/gtk.h>

void callback(GtkWidget *widget, gpointer data)
{
	g_print("enter %s: %s\n", __func__, (gchar *)data);
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
	GtkWidget *box;

	g_print("entry %s\n", __func__);

	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "hello world title");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 150);

	box = gtk_box_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), box);

	button = gtk_button_new_with_label("欢迎");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(callback), "欢迎您");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_button_new_with_label("说明");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(callback), "说明内容");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_button_new_with_label("退出");
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), window);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	gtk_widget_show(box);
	gtk_widget_show(window);
	gtk_main();

	g_print("leave %s\n", __func__);
	return 0;
}

