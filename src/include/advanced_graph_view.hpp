////////////////////////////////////////////////////////////////////////////////
// include/advanced_graph_view.hpp
//////////////////////////////////////////////////////////////////////////////
#ifndef ADVANCED_GRAPH_VIEW_HPP
#define ADVANCED_GRAPH_VIEW_HPP

#include <gtk/gtk.h>
#include <map>
#include <string>
#include "app_data.hpp"

// Drawing function
void advanced_graph_draw(GtkWidget* widget, cairo_t* cr, AppData* app);

// Callback functions for GTK signals
gboolean advanced_graph_on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
gboolean advanced_graph_scroll_event(GtkWidget* widget, GdkEventScroll* event, gpointer user_data);
gboolean advanced_graph_button_press_event(GtkWidget* widget, GdkEventButton* event, gpointer user_data);
gboolean advanced_graph_motion_notify_event(GtkWidget* widget, GdkEventMotion* event, gpointer user_data);

#endif // ADVANCED_GRAPH_VIEW_HPP
