////////////////////////////////////////////////////////////////////////////////
// gtk_trading_app.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/gtk_trading_app.hpp"
#include <cmath>
#include <sstream>
#include "../include/advanced_graph_view.hpp"



static void updateWindowTitle(GtkWindow* window, const AppData& app) {
    std::string baseTitle = "Advanced MBO Terminal";
    std::string modeLabel = (app.config.dataMode == DataMode::DEV) ? "DEV" : "REAL";
    std::string title = baseTitle + " [" + modeLabel + "]";
    gtk_window_set_title(window, title.c_str());
}

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

gboolean update_debug_text(gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(!app->textViewDebug) return TRUE;

    auto logs = app->processor->getDebugLogs();
    std::stringstream ss;
    for(const auto &line : logs) {
        ss << line << "\n";
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textViewDebug));
    gtk_text_buffer_set_text(buffer, ss.str().c_str(), -1);

    return TRUE;
}

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

    app->running = true;
}

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

void toggle_data_mode(GtkButton *button, gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    if(app->config.dataMode == DataMode::DEV) {
        app->config.dataMode = DataMode::REAL;
    } else {
        app->config.dataMode = DataMode::DEV;
    }
    updateWindowTitle(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))), *app);
}

void ticker_toggle_changed(GtkToggleButton* toggle, gpointer user_data) {
    AppData *app = static_cast<AppData *>(user_data);
    std::string ticker = gtk_widget_get_name(GTK_WIDGET(toggle));
    bool active = gtk_toggle_button_get_active(toggle);

    std::lock_guard<std::mutex> lock(app->dataMutex);
    if(active) {
        if(!app->tickerMap.count(ticker)) {
            app->tickerMap[ticker] = TickerData();
        }
    } else {
        if(app->tickerMap.count(ticker)) {
            app->tickerMap.erase(ticker);
        }
    }
}