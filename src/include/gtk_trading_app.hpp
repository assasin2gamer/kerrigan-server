////////////////////////////////////////////////////////////////////////////////
// include/gtk_trading_app.hpp
//////////////////////////////////////////////////////////////////////////////
#ifndef GTK_TRADING_APP_HPP
#define GTK_TRADING_APP_HPP

#include <gtk/gtk.h>
#include "app_data.hpp"

void start_monitoring(GtkButton* button, gpointer user_data);
void stop_monitoring(GtkButton* button, gpointer user_data);
void toggle_data_mode(GtkButton* button, gpointer user_data);
void ticker_toggle_changed(GtkToggleButton* toggle, gpointer user_data);
void ticker_log_toggle_changed(GtkToggleButton* toggle, gpointer user_data);
void setup_data_streams_tab(AppData* app, GtkWidget* notebook);
gboolean update_ui(gpointer user_data);
gboolean update_debug_text(gpointer user_data);
gboolean update_data_streams(gpointer user_data);

#endif // GTK_TRADING_APP_HPP
