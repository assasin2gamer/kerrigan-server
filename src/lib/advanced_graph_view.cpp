////////////////////////////////////////////////////////////////////////////////
// src/lib/advanced_graph_view.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/advanced_graph_view.hpp"
#include <cmath>
#include <algorithm>

void advanced_graph_draw(GtkWidget* widget, cairo_t* cr, AppData* app) {
    gtk_widget_set_size_request(widget, 800, 400);

    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    std::lock_guard<std::mutex> lock(app->dataMutex);

    double width  = gtk_widget_get_allocated_width(widget);
    double height = gtk_widget_get_allocated_height(widget);

    // Draw each ticker
    int colorIndex = 0;
    for(const auto& kv : app->tickerMap) {
        const std::string& ticker = kv.first;
        const TickerData& td = kv.second;
        if(td.values.empty()) continue;

        // Unique color for each ticker
        double r = (colorIndex & 1) ? 0.8 : 0.2;
        double g = (colorIndex & 2) ? 0.8 : 0.2;
        double b = (colorIndex & 4) ? 0.8 : 0.2;
        cairo_set_source_rgb(cr, r, g, b);

        cairo_set_line_width(cr, 2.0);
        // Start at left
        double xStep = 1.0;
        if(td.values.size() > 1) {
            xStep = width / (double)(td.values.size());
        }
        double prevX = 0;
        double prevY = 0;
        bool first = true;

        for(size_t i = 0; i < td.values.size(); i++) {
            double val = td.values[i];
            if(app->logScale && val > 0.0) {
                val = std::log10(val);
            }
            double mappedY = height - (val * app->yScale);
            double mappedX = (i * xStep - app->xOffset) * app->xScale;
            if(first) {
                cairo_move_to(cr, mappedX, mappedY);
                first = false;
            } else {
                cairo_line_to(cr, mappedX, mappedY);
            }
            prevX = mappedX;
            prevY = mappedY;
        }
        cairo_stroke(cr);

        colorIndex++;
    }
}

gboolean advanced_graph_on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    advanced_graph_draw(widget, cr, app);
    return FALSE;
}

gboolean advanced_graph_scroll_event(GtkWidget* widget, GdkEventScroll* event, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    if(event->direction == GDK_SCROLL_UP) {
        app->xScale *= 1.1;
        app->yScale *= 1.1;
    } else if(event->direction == GDK_SCROLL_DOWN) {
        app->xScale /= 1.1;
        app->yScale /= 1.1;
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean advanced_graph_button_press_event(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    if(event->button == 3) {
        app->logScale = !app->logScale;
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

gboolean advanced_graph_motion_notify_event(GtkWidget* widget, GdkEventMotion* event, gpointer user_data) {
    // Could implement drag/pan
    return TRUE;
}
