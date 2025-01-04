///////////////////////////////////////////////////////////////////////////////
// main.cpp
///////////////////////////////////////////////////////////////////////////////
#include <gtk/gtk.h>
#include "include/app_data.hpp"
#include "include/gtk_trading_app.hpp"
#include "include/config.hpp"
#include "include/influx_db_client.hpp"
#include "include/data_processor.hpp"
#include "include/stock_monitor.hpp"
#include "include/dev_monitor.hpp"
#include "include/advanced_graph_view.hpp"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    AppData app;
    app.config       = loadConfig("config.txt");
    app.dbClient     = std::make_shared<InfluxDBClient>(app.config.influxURL, app.config.influxDB);
    app.processor    = std::make_shared<DataProcessor>(app.dbClient, &app);
    app.stopFlag.store(false);
    app.requestCount.store(0);
    app.labelStats   = nullptr;
    app.drawingArea  = nullptr;
    app.textViewDebug= nullptr;
    app.running      = false;
    app.lastTime     = g_get_monotonic_time() / 1e6;
    app.globalLogScale = false; // Changed from app.logScale
    app.xOffset      = 0.0;
    app.xScale       = 1.0;
    app.yScale       = 1.0;

    // Main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 700);
    gtk_window_set_title(GTK_WINDOW(window), "Advanced MBO Terminal [DEV]");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *mainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), mainBox);

    // Left side: Controls
    GtkWidget *controlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(mainBox), controlBox, FALSE, FALSE, 5);

    GtkWidget *startBtn = gtk_button_new_with_label("Start");
    g_signal_connect(startBtn, "clicked", G_CALLBACK(start_monitoring), &app);
    gtk_box_pack_start(GTK_BOX(controlBox), startBtn, FALSE, FALSE, 5);

    GtkWidget *stopBtn = gtk_button_new_with_label("Stop");
    g_signal_connect(stopBtn, "clicked", G_CALLBACK(stop_monitoring), &app);
    gtk_box_pack_start(GTK_BOX(controlBox), stopBtn, FALSE, FALSE, 5);

    GtkWidget *toggleBtn = gtk_button_new_with_label("Toggle Mode (DEV/REAL)");
    g_signal_connect(toggleBtn, "clicked", G_CALLBACK(toggle_data_mode), &app);
    gtk_box_pack_start(GTK_BOX(controlBox), toggleBtn, FALSE, FALSE, 5);

    app.labelStats = gtk_label_new("Requests: 0 | Errors: 0");
    gtk_box_pack_start(GTK_BOX(controlBox), app.labelStats, FALSE, FALSE, 5);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(controlBox), sep, FALSE, FALSE, 5);

    // Checkbuttons for each symbol
    for(const auto& sym : app.config.symbols) {
        GtkWidget *chk = gtk_check_button_new_with_label(sym.c_str());
        gtk_widget_set_name(chk, sym.c_str());
        g_signal_connect(chk, "toggled", G_CALLBACK(ticker_toggle_changed), &app);
        gtk_box_pack_start(GTK_BOX(controlBox), chk, FALSE, FALSE, 5);
    }

    // Right side: Notebook with Graph, Debug, Data Streams
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(mainBox), notebook, TRUE, TRUE, 5);

    // Graph Tab
    GtkWidget *graphTab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), graphTab, gtk_label_new("Graph"));

    GtkWidget *drawArea = gtk_drawing_area_new();
    g_signal_connect(drawArea, "draw", G_CALLBACK(advanced_graph_on_draw), &app);
    g_signal_connect(drawArea, "scroll-event", G_CALLBACK(advanced_graph_scroll_event), &app);
    g_signal_connect(drawArea, "button-press-event", G_CALLBACK(advanced_graph_button_press_event), &app);
    g_signal_connect(drawArea, "motion-notify-event", G_CALLBACK(advanced_graph_motion_notify_event), &app);
    gtk_widget_add_events(drawArea, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK);

    // Checkbuttons for each symbol with log-scale option
    for(const auto& sym : app.config.symbols) {
        // Create a horizontal box to hold the ticker checkbox and log-scale checkbox
        GtkWidget *tickerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        
        // Ticker visibility checkbox
        GtkWidget *chk = gtk_check_button_new_with_label(sym.c_str());
        gtk_widget_set_name(chk, sym.c_str()); // Set widget name for identification
        g_signal_connect(chk, "toggled", G_CALLBACK(ticker_toggle_changed), &app);
        gtk_box_pack_start(GTK_BOX(tickerBox), chk, FALSE, FALSE, 5);
        
        // Log-scale checkbox (initially inactive)
        GtkWidget *logChk = gtk_check_button_new_with_label("Log Scale");
        gtk_widget_set_name(logChk, (std::string(sym) + "_log").c_str()); // Unique name
        g_signal_connect(logChk, "toggled", G_CALLBACK(ticker_toggle_changed), &app); // Correct callback
        gtk_box_pack_start(GTK_BOX(tickerBox), logChk, FALSE, FALSE, 5);
        
        gtk_box_pack_start(GTK_BOX(controlBox), tickerBox, FALSE, FALSE, 5);
    }
    gtk_box_pack_start(GTK_BOX(graphTab), drawArea, TRUE, TRUE, 5);
    app.drawingArea = drawArea;

    // Debug Tab
    GtkWidget *debugTab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), debugTab, gtk_label_new("Debug"));

    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(debugTab), scrolledWindow, TRUE, TRUE, 5);

    GtkWidget *textView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    app.textViewDebug = textView;

    // Data Streams Tab
    setup_data_streams_tab(&app, notebook);

    gtk_widget_show_all(window);

    // Timers for UI, debug, and data streams updates
    g_timeout_add(1000/30, update_ui, &app);
    g_timeout_add(1000, update_debug_text, &app);
    g_timeout_add(1000, update_data_streams, &app);

    gtk_main();

    // Cleanup
    app.stopFlag.store(true);
    for(auto &t : app.threads) {
        if(t.joinable()) {
            t.join();
        }
    }
    app.threads.clear();

    return 0;
}
