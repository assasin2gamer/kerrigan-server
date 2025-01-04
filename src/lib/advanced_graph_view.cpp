///////////////////////////////////////////////////////////////////////////////
// src/lib/advanced_graph_view.cpp
///////////////////////////////////////////////////////////////////////////////
#include "../include/advanced_graph_view.hpp"
#include "../include/app_data.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cairo.h>
#include <gtk/gtk.h>

// Function to draw the stacked graphs with individual log-scale options
void advanced_graph_draw(GtkWidget* widget, cairo_t* cr, AppData* app) {
    // Define margins
    const double margin_left = 60.0;
    const double margin_right = 20.0;
    const double margin_top = 20.0;
    const double margin_bottom = 60.0;
    const double graph_spacing = 40.0; // Space between stacked graphs

    // Get widget dimensions
    double width = gtk_widget_get_allocated_width(widget);
    double height = gtk_widget_get_allocated_height(widget);

    // Clear the background with white color
    cairo_set_source_rgb(cr, 1, 1, 1); // White background
    cairo_paint(cr);

    // Lock the data for thread-safe access
    std::lock_guard<std::mutex> lock(app->dataMutex);

    // Collect selected tickers (those with non-empty data)
    std::vector<std::pair<std::string, TickerData>> selectedTickers;
    for(const auto& kv : app->tickerMap) {
        if(!kv.second.values.empty()) {
            selectedTickers.emplace_back(kv.first, kv.second);
        }
    }

    size_t numTickers = selectedTickers.size();
    if(numTickers == 0) {
        // No data to display
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, margin_left, height / 2);
        cairo_show_text(cr, "No data to display. Please select tickers.");
        return;
    }

    // Calculate available height per graph
    double availableHeight = height - margin_top - margin_bottom - (numTickers - 1) * graph_spacing;
    double graphHeight = availableHeight / numTickers;

    // Iterate through each selected ticker and draw its graph
    size_t colorIndex = 0;
    for(const auto& tickerPair : selectedTickers) {
        const std::string& ticker = tickerPair.first;
        const TickerData& td = tickerPair.second;

        // Define the drawing area for this graph
        double graph_y = margin_top + (graphHeight + graph_spacing) * colorIndex;

        // Determine min and max for Y-axis scaling
        double localMin = *std::min_element(td.values.begin(), td.values.end());
        double localMax = *std::max_element(td.values.begin(), td.values.end());

        // Handle cases where all values are the same
        if(localMax - localMin == 0) {
            localMax += 1.0;
            localMin -= 1.0;
        }

        // Apply log-scale if enabled for this ticker
        bool useLogScale = td.logScale;
        if(useLogScale) {
            // Filter out non-positive values for log scale
            std::vector<double> positiveValues;
            for(auto val : td.values) {
                if(val > 0.0) {
                    positiveValues.push_back(val);
                }
            }

            if(!positiveValues.empty()) {
                // Recalculate min and max based on positive values
                localMin = *std::min_element(positiveValues.begin(), positiveValues.end());
                localMax = *std::max_element(positiveValues.begin(), positiveValues.end());

                // Apply log10 transformation
                localMin = std::log10(localMin);
                localMax = std::log10(localMax);

                // Handle cases where all log-transformed values are the same
                if(localMax - localMin == 0) {
                    localMax += 0.1;
                    localMin -= 0.1;
                }
            } else {
                // If no positive values, disable log scale for this ticker
                useLogScale = false;
            }
        }

        // Add some padding to the Y-axis
        double yPadding = 0.05 * (localMax - localMin);
        localMin -= yPadding;
        localMax += yPadding;

        // Calculate scaling factors
        double yScale = (graphHeight) / (localMax - localMin);
        double xScale = (width - margin_left - margin_right) / static_cast<double>(td.values.size() - 1);

        // Draw Y-axis labels and grid lines
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10.0);
        cairo_set_source_rgb(cr, 0, 0, 0); // Black text

        int numYLabels = 5;
        for(int i = 0; i <= numYLabels; ++i) {
            double yValue = localMin + i * (localMax - localMin) / numYLabels;
            double yPos = graph_y + graphHeight - (yValue - localMin) * yScale;

            // Draw grid line
            cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); // Light gray grid
            cairo_set_line_width(cr, 0.5);
            cairo_move_to(cr, margin_left, yPos);
            cairo_line_to(cr, width - margin_right, yPos);
            cairo_stroke(cr);

            // Reset color for labels
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_set_line_width(cr, 1.0);

            // Draw label
            std::stringstream ss;
            if(useLogScale) {
                ss << std::fixed << std::setprecision(2) << std::pow(10, yValue);
            }
            else {
                ss << std::fixed << std::setprecision(2) << yValue;
            }
            std::string label = ss.str();
            cairo_move_to(cr, 5, yPos + 5); // Position labels to the left of Y-axis
            cairo_show_text(cr, label.c_str());
        }

        // Draw X-axis labels and grid lines
        int numXLabels = 5;
        size_t maxDataPoints = td.values.size();
        double xStep = (width - margin_left - margin_right) / static_cast<double>(numXLabels);
        for(int i = 0; i <= numXLabels; ++i) {
            double xPos = margin_left + i * xStep;

            // Draw grid line
            cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); // Light gray grid
            cairo_set_line_width(cr, 0.5);
            cairo_move_to(cr, xPos, graph_y);
            cairo_line_to(cr, xPos, graph_y + graphHeight);
            cairo_stroke(cr);

            // Reset color for labels
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_set_line_width(cr, 1.0);

            // Draw label (e.g., index or timestamp)
            std::stringstream ss;
            size_t dataIndex = static_cast<size_t>(i * (maxDataPoints - 1) / numXLabels);
            ss << dataIndex;
            std::string label = ss.str();
            cairo_move_to(cr, xPos - 10, graph_y + graphHeight + 20); // Position labels below X-axis
            cairo_show_text(cr, label.c_str());
        }

        // Assign a unique color for each ticker
        double r = (colorIndex * 0.2) < 1.0 ? (colorIndex * 0.2) : 1.0;
        double g = (colorIndex * 0.3) < 1.0 ? (colorIndex * 0.3) : 1.0;
        double b = (colorIndex * 0.5) < 1.0 ? (colorIndex * 0.5) : 1.0;
        cairo_set_source_rgb(cr, r, g, b);

        cairo_set_line_width(cr, 2.0);

        // Start drawing the line
        bool firstPoint = true;
        for(size_t i = 0; i < td.values.size(); ++i) {
            double val = td.values[i];
            double processedVal = val;

            if(useLogScale && val > 0.0) {
                processedVal = std::log10(val);
            }

            // Calculate position
            double x = margin_left + i * xScale;
            double y = graph_y + graphHeight - (processedVal - localMin) * yScale;

            if(firstPoint) {
                cairo_move_to(cr, x, y);
                firstPoint = false;
            }
            else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_stroke(cr);

        // Draw ticker label at the top-left of each graph
        cairo_set_source_rgb(cr, r, g, b); // Same color as the line
        cairo_move_to(cr, margin_left, graph_y - 5);
        cairo_show_text(cr, ticker.c_str());

        colorIndex++;
    } // End of advanced_graph_draw
}
// Callback function for the "draw" signal
gboolean advanced_graph_on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    advanced_graph_draw(widget, cr, app);
    return FALSE;
}

// Callback function for the "scroll-event" signal (Zoom In/Out)
gboolean advanced_graph_scroll_event(GtkWidget* widget, GdkEventScroll* event, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    
    // Determine zoom direction
    if(event->direction == GDK_SCROLL_UP) {
        app->yScale *= 1.1; // Zoom in
    }
    else if(event->direction == GDK_SCROLL_DOWN) {
        app->yScale /= 1.1; // Zoom out
    }
    
    // Clamp yScale to prevent excessive zooming
    app->yScale = std::clamp(app->yScale, 0.1, 100.0);
    
    gtk_widget_queue_draw(widget); // Redraw the graph with new scaling
    return TRUE;
}

// Callback function for the "button-press-event" signal (Toggle Log Scale)
gboolean advanced_graph_button_press_event(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    
    if(event->button == 3) { // Right-click to toggle global log scale
        app->globalLogScale = !app->globalLogScale;
        gtk_widget_queue_draw(widget); // Redraw the graph with updated scale
    }
    return TRUE;
}

// Callback function for the "motion-notify-event" signal (Panning)
gboolean advanced_graph_motion_notify_event(GtkWidget* widget, GdkEventMotion* event, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    static double lastX = 0.0;
    
    if(event->state & GDK_BUTTON1_MASK) { // Left button held for panning
        double deltaX = event->x - lastX;
        // Adjust xOffset based on mouse movement and current xScale
        app->xOffset += deltaX / app->xScale;
        
        // Clamp xOffset to prevent panning beyond data
        app->xOffset = std::clamp(app->xOffset, 0.0, app->tickerMap.empty() ? 0.0 :
                                 static_cast<double>(app->tickerMap.begin()->second.values.size()) * app->xScale);
        
        gtk_widget_queue_draw(widget); // Redraw the graph with updated panning
    }
    
    lastX = event->x;
    return TRUE;
}
