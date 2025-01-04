////////////////////////////////////////////////////////////////////////////////
// gtk_trading_app.cpp
////////////////////////////////////////////////////////////////////////////////

#include "../include/gtk_trading_app.hpp"
#include <cmath>
#include <sstream>
#include <gtk/gtk.h>
#include <iostream>

#include "../include/advanced_graph_view.hpp"
#include "../include/data_processor.hpp"
#include "../include/dev_monitor.hpp"
#include "../include/stock_monitor.hpp"// Constants
static const int GRAPH_HISTORY_SIZE = 300;

// Function to update window title based on mode
static void updateWindowTitle(GtkWindow* window, const AppData& app) {
    std::string baseTitle = "Advanced MBO Terminal";
    std::string modeLabel = (app.config.dataMode == DataMode::DEV) ? "DEV" : "REAL";
    std::string title = baseTitle + " [" + modeLabel + "]";
    gtk_window_set_title(window, title.c_str());
}

// Function to initialize DataStreamStats entries
static void initialize_data_stream(AppData* app, const std::string& streamID) {
    std::lock_guard<std::mutex> lock(app->statsMutex);
    auto it = app->dataStreamStats.find(streamID);
    if(it == app->dataStreamStats.end()) {
        app->dataStreamStats[streamID] = std::make_shared<DataStreamStats>();
    }
    // Reset statistics
    app->dataStreamStats[streamID]->messagesReceived = 0;
    app->dataStreamStats[streamID]->errors = 0;
}

// Callback function to start monitoring
void start_monitoring(GtkButton *button, gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(app->running) return;

    app->stopFlag.store(false);
    app->requestCount.store(0);
    app->processor->errorCount.store(0);

    {
        std::lock_guard<std::mutex> lock(app->dataMutex);
        for(auto &kv : app->tickerMap) {
            kv.second.values.clear();
        }
    }

    app->lastTime = g_get_monotonic_time() / 1e6;
    int activeCores = app->config.totalCores - app->config.reserveCores;
    if(activeCores <= 0) return;

    for(int i = 0; i < activeCores; ++i) {
        if(app->config.dataMode == DataMode::DEV) {
            DevMonitor mon(app->processor);
            app->threads.emplace_back([mon, app]() mutable {
                mon.run(app->stopFlag, app->requestCount);
            });
        } else {
            StockMonitor mon(app->config, app->processor);
            app->threads.emplace_back([mon, app]() mutable {
                mon.run(app->stopFlag, app->requestCount);
            });
        }
    }

    // Initialize dataStreamStats for the current mode
    std::string currentMode = (app->config.dataMode == DataMode::DEV) ? "DEV" : "REAL";
    initialize_data_stream(app, currentMode);

    app->running = true;
}

// Callback function to stop monitoring
void stop_monitoring(GtkButton *button, gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(!app->running) return;

    app->stopFlag.store(true);
    for(auto &t : app->threads) {
        if(t.joinable()) {
            t.join();
        }
    }
    app->threads.clear();
    app->running = false;
}

// Callback function to toggle data mode
void toggle_data_mode(GtkButton *button, gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(app->config.dataMode == DataMode::DEV) {
        app->config.dataMode = DataMode::REAL;
    } else {
        app->config.dataMode = DataMode::DEV;
    }
    updateWindowTitle(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))), *app);
}

// Callback function for ticker toggle changes
void ticker_toggle_changed(GtkToggleButton* toggle, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    const char* ticker_cstr = gtk_widget_get_name(GTK_WIDGET(toggle));
    if(ticker_cstr == nullptr) return;
    std::string ticker = ticker_cstr;
    bool active = gtk_toggle_button_get_active(toggle);
    
    std::lock_guard<std::mutex> lock(app->dataMutex);
    if(active) {
        if(app->tickerMap.find(ticker) == app->tickerMap.end()) {
            app->tickerMap[ticker] = TickerData();
        }
    }
    else {
        if(app->tickerMap.find(ticker) != app->tickerMap.end()) {
            app->tickerMap.erase(ticker);
        }
    }
    
    // Trigger a redraw of the graph
    if(app->drawingArea) {
        gtk_widget_queue_draw(app->drawingArea);
    }
}

// Function to setup the Data Streams tab
void setup_data_streams_tab(AppData* app, GtkWidget* notebook) {
    GtkWidget *dataStreamsTab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(dataStreamsTab), scrolledWindow, TRUE, TRUE, 5);

    // Create GtkListStore with columns: Stream ID, Messages Received, Errors
    GtkListStore *listStore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
    app->dataStreamsListStore = listStore; // Store in AppData

    GtkWidget *treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
    gtk_container_add(GTK_CONTAINER(scrolledWindow), treeView);

    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Stream ID Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Stream ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    // Messages Received Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Messages Received", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    // Errors Column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Errors", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    gtk_widget_show_all(dataStreamsTab);
    GtkWidget *dataStreamsLabel = gtk_label_new("Data Streams");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dataStreamsTab, dataStreamsLabel);
}

// Function to update the Data Streams tab in the UI
gboolean update_data_streams(gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(!app->dataStreamsListStore) return TRUE;

    gtk_list_store_clear(app->dataStreamsListStore);

    {
        std::lock_guard<std::mutex> lock(app->statsMutex);
        for(const auto &kv : app->dataStreamStats) {
            GtkTreeIter iter;
            gtk_list_store_append(app->dataStreamsListStore, &iter);
            gtk_list_store_set(app->dataStreamsListStore, &iter,
                               0, kv.first.c_str(),
                               1, kv.second->messagesReceived.load(),
                               2, kv.second->errors.load(),
                               -1);
        }
    }

    return TRUE;
}

// Existing update_ui function
gboolean update_ui(gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);

    if(app->labelStats) {
        std::stringstream ss;
        ss << "Requests: " << app->requestCount.load()
           << " | Errors: " << app->processor->errorCount.load();
        gtk_label_set_text(GTK_LABEL(app->labelStats), ss.str().c_str());
    }

    {
        std::lock_guard<std::mutex> lock(app->dataMutex);
        for(auto &kv : app->tickerMap) {
            if(kv.second.values.size() > GRAPH_HISTORY_SIZE * 2) {
                kv.second.values.erase(kv.second.values.begin(),
                                       kv.second.values.end() - GRAPH_HISTORY_SIZE);
            }
        }
    }

    if(app->drawingArea) {
        gtk_widget_queue_draw(app->drawingArea);
    }

    return TRUE;
}

// Existing update_debug_text function
gboolean update_debug_text(gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    if(app->textViewDebug) {
        auto logs = app->processor->getDebugLogs();
        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textViewDebug));
        std::string allLogs;
        for(const auto& log : logs) {
            allLogs += log + "\n";
        }
        gtk_text_buffer_set_text(buffer, allLogs.c_str(), -1);
    }
    return TRUE; // Continue calling this function
}

// Callback function for ticker log-scale checkbutton toggling
void ticker_log_toggle_changed(GtkToggleButton* toggle, gpointer user_data) {
    AppData* app = static_cast<AppData*>(user_data);
    
    const char* toggle_name = gtk_widget_get_name(GTK_WIDGET(toggle));
    if(toggle_name == nullptr) return;
    
    std::string fullName = toggle_name;
    
    // Extract ticker symbol by removing the "_log" suffix
    std::string ticker = fullName.substr(0, fullName.find("_log"));
    
    bool active = gtk_toggle_button_get_active(toggle);
    
    {
        std::lock_guard<std::mutex> lock(app->dataMutex);
        auto it = app->tickerMap.find(ticker);
        if(it != app->tickerMap.end()) {
            it->second.logScale = active;
        }
    }
    
    // Trigger a redraw of the graph
    if(app->drawingArea) {
        gtk_widget_queue_draw(app->drawingArea);
    }
}