////////////////////////////////////////////////////////////////////////////////
// gtk_trading_app.h
////////////////////////////////////////////////////////////////////////////////
#ifndef GTK_TRADING_APP_H
#define GTK_TRADING_APP_H

#include <gtk/gtk.h>
#include "app_data.hpp"


gboolean update_ui(gpointer user_data);
gboolean update_debug_text(gpointer user_data);
void start_monitoring(GtkButton *button, gpointer user_data);
void stop_monitoring(GtkButton *button, gpointer user_data);
void toggle_data_mode(GtkButton *button, gpointer user_data);
void ticker_toggle_changed(GtkToggleButton* toggle, gpointer user_data);

#endif // GTK_TRADING_APP_H