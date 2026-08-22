#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_display_panel.hpp>

#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "secrets.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const uint32_t C_BG       = 0x080B12;
static const uint32_t C_PANEL    = 0x111521;
static const uint32_t C_CARD     = 0x171B26;
static const uint32_t C_BORDER   = 0x303541;
static const uint32_t C_TEXT     = 0xF4F5F8;
static const uint32_t C_MUTED    = 0x858A96;
static const uint32_t C_ACCENT   = 0xA79BC8;
static const uint32_t C_ACCENT_2 = 0xBEB2DA;
static const uint32_t C_GREEN    = 0x83D8AA;

struct SourceUI {
    const char *key;
    lv_obj_t *button;
    lv_obj_t *active_dot;
};

static SourceUI sources[4];
static lv_obj_t *online_dot = nullptr;
static lv_obj_t *online_label = nullptr;
static lv_obj_t *power_status_label = nullptr;
static lv_obj_t *power_dot = nullptr;
static lv_obj_t *message_label = nullptr;

// Cache the visible state so unchanged backend responses don't trigger
// another full-screen LVGL refresh.
static String last_ui_signature = "";

enum PendingAction {
    ACTION_NONE, ACTION_PC, ACTION_MAC, ACTION_PS5, ACTION_SWITCH, ACTION_POWER
};
volatile PendingAction pending_action = ACTION_NONE;

static const char *wifiStatusName(wl_status_t status)
{
    switch (status) {
        case WL_IDLE_STATUS:     return "IDLE";
        case WL_NO_SSID_AVAIL:   return "NO_SSID";
        case WL_SCAN_COMPLETED:  return "SCAN_DONE";
        case WL_CONNECTED:       return "CONNECTED";
        case WL_CONNECT_FAILED:  return "AUTH_FAILED";
        case WL_CONNECTION_LOST: return "LOST";
        case WL_DISCONNECTED:    return "DISCONNECTED";
        default:                 return "UNKNOWN";
    }
}

static void printWifiDebug()
{
    wl_status_t st = WiFi.status();

    Serial.printf("[WiFi] status=%d (%s)", (int)st, wifiStatusName(st));
    if (st == WL_CONNECTED) {
        Serial.printf("  SSID=%s  IP=%s  RSSI=%d dBm  CH=%d\n",
                      WiFi.SSID().c_str(),
                      WiFi.localIP().toString().c_str(),
                      WiFi.RSSI(),
                      WiFi.channel());
    } else {
        Serial.printf("  target=%s\n", WIFI_SSID);
    }
}

static void scanTargetSSID()
{
    Serial.printf("[WiFi] Scanning for target SSID: %s\n", WIFI_SSID);
    int n = WiFi.scanNetworks(false, true);
    bool found = false;

    if (n < 0) {
        Serial.printf("[WiFi] scan failed: %d\n", n);
        return;
    }

    for (int i = 0; i < n; ++i) {
        if (WiFi.SSID(i) == WIFI_SSID) {
            found = true;
            Serial.printf("[WiFi] target FOUND: RSSI=%d dBm, channel=%d, auth=%d\n",
                          WiFi.RSSI(i), WiFi.channel(i), (int)WiFi.encryptionType(i));
        }
    }

    if (!found) {
        Serial.println("[WiFi] target SSID NOT FOUND");
    }
    WiFi.scanDelete();
}

static uint32_t last_status_poll = 0;
static uint32_t last_wifi_attempt = 0;
static uint32_t last_wifi_debug = 0;
static const uint32_t STATUS_POLL_MS = 5000;
static const uint32_t WIFI_RETRY_MS = 15000;

static String jsonStringValue(const String &json, const char *key)
{
    String token = "\"" + String(key) + "\"";
    int p = json.indexOf(token);
    if (p < 0) return "";
    p = json.indexOf(':', p + token.length());
    if (p < 0) return "";
    int q1 = json.indexOf('"', p + 1);
    if (q1 < 0) return "";
    int q2 = json.indexOf('"', q1 + 1);
    if (q2 < 0) return "";
    return json.substring(q1 + 1, q2);
}

static bool sourceEnabledFromJson(const String &json, const char *source, bool fallback)
{
    String token = "\"" + String(source) + "\"";
    int p = json.indexOf(token);
    if (p < 0) return fallback;
    int end = json.indexOf('}', p);
    if (end < 0) end = min((int)json.length(), p + 180);
    int ep = json.indexOf("\"enabled\"", p);
    if (ep < 0 || ep > end) return fallback;
    int colon = json.indexOf(':', ep);
    if (colon < 0) return fallback;
    String s = json.substring(colon + 1, min((int)json.length(), colon + 12));
    s.trim();
    return s.startsWith("true");
}

static bool httpGet(const String &path, String &payload)
{
    if (WiFi.status() != WL_CONNECTED) return false;
    HTTPClient http;
    http.setConnectTimeout(1200);
    http.setTimeout(1800);
    if (!http.begin(String(G8_API_BASE) + path)) return false;
    int code = http.GET();
    if (code > 0) payload = http.getString();
    http.end();
    return code >= 200 && code < 300;
}

static bool httpPost(const String &path, String &payload)
{
    if (WiFi.status() != WL_CONNECTED) return false;
    HTTPClient http;
    http.setConnectTimeout(1200);
    http.setTimeout(2500);
    if (!http.begin(String(G8_API_BASE) + path)) return false;
    http.addHeader("Content-Type", "application/json");
    int code = http.POST("");
    if (code > 0) payload = http.getString();
    http.end();
    return code >= 200 && code < 300;
}

static void setCardStyle(lv_obj_t *obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_radius(obj, 18, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x262237), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, lv_color_hex(C_ACCENT), LV_STATE_PRESSED);
    lv_obj_set_style_opa(obj, LV_OPA_40, LV_STATE_DISABLED);
}

static void setOnline(bool online)
{
    lv_obj_set_style_bg_color(online_dot, lv_color_hex(online ? C_GREEN : 0x666B76), 0);
    lv_label_set_text(online_label, online ? "ONLINE" : "OFFLINE");
    lv_obj_set_style_text_color(online_label, lv_color_hex(online ? 0xA8ADBA : 0x727783), 0);
}

static void setMessage(const char *s)
{
    lv_label_set_text(message_label, s);
}

static void setSourceSelected(const String &name)
{
    for (int i = 0; i < 4; ++i) {
        bool active = name == sources[i].key;
        lv_obj_set_style_bg_color(sources[i].button, lv_color_hex(active ? 0x282438 : C_CARD), 0);
        lv_obj_set_style_border_color(sources[i].button, lv_color_hex(active ? C_ACCENT : C_BORDER), 0);
        if (active) lv_obj_clear_flag(sources[i].active_dot, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(sources[i].active_dot, LV_OBJ_FLAG_HIDDEN);
    }
}

static void setSourceEnabled(int i, bool enabled)
{
    if (enabled) lv_obj_clear_state(sources[i].button, LV_STATE_DISABLED);
    else lv_obj_add_state(sources[i].button, LV_STATE_DISABLED);
}

static void setPowerState(const String &state)
{
    if (state == "on") {
        lv_label_set_text(power_status_label, "ON");
        lv_obj_set_style_text_color(power_status_label, lv_color_hex(C_ACCENT_2), 0);
        lv_obj_set_style_bg_color(power_dot, lv_color_hex(C_ACCENT), 0);
    } else if (state == "off") {
        lv_label_set_text(power_status_label, "OFF");
        lv_obj_set_style_text_color(power_status_label, lv_color_hex(0x8A8F9A), 0);
        lv_obj_set_style_bg_color(power_dot, lv_color_hex(0x747985), 0);
    } else {
        lv_label_set_text(power_status_label, "UNKNOWN");
        lv_obj_set_style_text_color(power_status_label, lv_color_hex(0x8A8F9A), 0);
        lv_obj_set_style_bg_color(power_dot, lv_color_hex(0x747985), 0);
    }
}

static void sourceEvent(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    SourceUI *s = static_cast<SourceUI *>(lv_event_get_user_data(e));
    if (!s) return;
    if      (!strcmp(s->key, "pc"))     pending_action = ACTION_PC;
    else if (!strcmp(s->key, "mac"))    pending_action = ACTION_MAC;
    else if (!strcmp(s->key, "ps5"))    pending_action = ACTION_PS5;
    else if (!strcmp(s->key, "switch")) pending_action = ACTION_SWITCH;
    setMessage("Switching...");
}

static void powerEvent(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        pending_action = ACTION_POWER;
        setMessage("Power...");
    }
}

static void createSource(
    lv_obj_t *parent, int i, const char *key, const char *label,
    const char *mark, int x, int y)
{
    sources[i].key = key;
    sources[i].button = lv_btn_create(parent);

    setCardStyle(sources[i].button);
    lv_obj_set_pos(sources[i].button, x, y);
    lv_obj_set_size(sources[i].button, 346, 92);

    lv_obj_t *icon = lv_label_create(sources[i].button);
    lv_label_set_text(icon, mark);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(C_MUTED), 0);
    lv_obj_align(icon, LV_ALIGN_CENTER, -48, 0);

    lv_obj_t *txt = lv_label_create(sources[i].button);
    lv_label_set_text(txt, label);
    lv_obj_set_style_text_font(txt, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(txt, lv_color_hex(C_TEXT), 0);
    lv_obj_align(txt, LV_ALIGN_CENTER, 18, 0);

    sources[i].active_dot = lv_obj_create(sources[i].button);
    lv_obj_remove_style_all(sources[i].active_dot);
    lv_obj_set_size(sources[i].active_dot, 6, 6);
    lv_obj_set_style_radius(sources[i].active_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(sources[i].active_dot, lv_color_hex(C_ACCENT_2), 0);
    lv_obj_set_style_bg_opa(sources[i].active_dot, LV_OPA_COVER, 0);
    lv_obj_align(sources[i].active_dot, LV_ALIGN_TOP_RIGHT, -13, 13);
    lv_obj_add_flag(sources[i].active_dot, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(sources[i].button, sourceEvent, LV_EVENT_CLICKED, &sources[i]);
}

static void buildUI()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *panel = lv_obj_create(scr);
    lv_obj_remove_style_all(panel);
    lv_obj_set_pos(panel, 20, 16);
    lv_obj_set_size(panel, 760, 448);
    lv_obj_set_style_radius(panel, 22, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(C_PANEL), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x292E3A), 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "Samsung OLED G8");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(C_TEXT), 0);
    lv_obj_set_pos(title, 24, 18);

    lv_obj_t *subtitle = lv_label_create(panel);
    lv_label_set_text(subtitle, "Display Control Hub");
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(C_MUTED), 0);
    lv_obj_set_pos(subtitle, 25, 59);

    online_dot = lv_obj_create(panel);
    lv_obj_remove_style_all(online_dot);
    lv_obj_set_size(online_dot, 6, 6);
    lv_obj_set_style_radius(online_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(online_dot, LV_OPA_COVER, 0);
    lv_obj_set_pos(online_dot, 648, 31);

    online_label = lv_label_create(panel);
    lv_label_set_text(online_label, "OFFLINE");
    lv_obj_set_style_text_font(online_label, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(online_label, 662, 25);

    createSource(panel, 0, "pc",     "PC",     "WIN", 24, 106);
    createSource(panel, 1, "mac",    "Mac",    "MAC", 390, 106);
    createSource(panel, 2, "ps5",    "PS5",    "PS",  24, 208);
    createSource(panel, 3, "switch", "Switch", "NS",  390, 208);

    lv_obj_t *power = lv_btn_create(panel);
    setCardStyle(power);
    lv_obj_set_pos(power, 24, 310);
    lv_obj_set_size(power, 712, 68);
    lv_obj_add_event_cb(power, powerEvent, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *power_symbol = lv_label_create(power);
    lv_label_set_text(power_symbol, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(power_symbol, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(power_symbol, lv_color_hex(C_TEXT), 0);
    lv_obj_align(power_symbol, LV_ALIGN_LEFT_MID, 18, 0);

    lv_obj_t *power_text = lv_label_create(power);
    lv_label_set_text(power_text, "Display Power");
    lv_obj_set_style_text_font(power_text, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(power_text, lv_color_hex(C_TEXT), 0);
    lv_obj_align(power_text, LV_ALIGN_LEFT_MID, 58, 0);

    power_dot = lv_obj_create(power);
    lv_obj_remove_style_all(power_dot);
    lv_obj_set_size(power_dot, 7, 7);
    lv_obj_set_style_radius(power_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(power_dot, LV_OPA_COVER, 0);
    lv_obj_align(power_dot, LV_ALIGN_RIGHT_MID, -88, 0);

    power_status_label = lv_label_create(power);
    lv_label_set_text(power_status_label, "UNKNOWN");
    lv_obj_set_style_text_font(power_status_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(power_status_label, lv_color_hex(0x8A8F9A), 0);
    lv_obj_align(power_status_label, LV_ALIGN_RIGHT_MID, -18, 0);

    message_label = lv_label_create(panel);
    lv_label_set_text(message_label, "Connecting...");
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(message_label, lv_color_hex(C_MUTED), 0);
    lv_obj_set_width(message_label, 712);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(message_label, 24, 394);

    setOnline(false);
    setPowerState("unknown");
}

static void refreshStatus()
{
    String body;
    if (!httpGet("/api/status", body)) {
        wl_status_t st = WiFi.status();

        String signature = "offline|";
        signature += String((int)st);
        if (st == WL_CONNECTED) {
            signature += '|';
            signature += WiFi.localIP().toString();
        }

        // If the visible offline state hasn't changed, don't invalidate LVGL.
        if (signature == last_ui_signature) return;

        if (lvgl_port_lock(60)) {
            setOnline(false);

            if (st == WL_CONNECTED) {
                static char msg[96];
                snprintf(msg, sizeof(msg), "Wi-Fi OK  %s  |  API OFFLINE",
                         WiFi.localIP().toString().c_str());
                setMessage(msg);
            } else if (st == WL_NO_SSID_AVAIL) {
                setMessage("Wi-Fi: SSID NOT FOUND");
            } else if (st == WL_CONNECT_FAILED) {
                setMessage("Wi-Fi: AUTH FAILED");
            } else {
                static char msg[64];
                snprintf(msg, sizeof(msg), "Wi-Fi: %s", wifiStatusName(st));
                setMessage(msg);
            }

            last_ui_signature = signature;
            lvgl_port_unlock();
        }
        return;
    }

    String source = jsonStringValue(body, "source");
    String power = jsonStringValue(body, "power");

    bool epc = sourceEnabledFromJson(body, "pc", true);
    bool emac = sourceEnabledFromJson(body, "mac", true);
    bool eps5 = sourceEnabledFromJson(body, "ps5", true);
    bool esw = sourceEnabledFromJson(body, "switch", true);

    String signature = "online|";
    signature += source;
    signature += '|';
    signature += power;
    signature += '|';
    signature += (epc ? '1' : '0');
    signature += (emac ? '1' : '0');
    signature += (eps5 ? '1' : '0');
    signature += (esw ? '1' : '0');
    signature += '|';
    signature += WiFi.localIP().toString();

    // The backend can still be polled regularly, but LVGL only redraws when
    // something that is actually visible on screen has changed.
    if (signature == last_ui_signature) return;

    if (lvgl_port_lock(80)) {
        setOnline(true);
        if (source.length()) setSourceSelected(source);
        if (power.length()) setPowerState(power);
        setSourceEnabled(0, epc);
        setSourceEnabled(1, emac);
        setSourceEnabled(2, eps5);
        setSourceEnabled(3, esw);

        static char okmsg[96];
        snprintf(okmsg, sizeof(okmsg), "Wi-Fi %s  |  API OK",
                 WiFi.localIP().toString().c_str());
        setMessage(okmsg);

        last_ui_signature = signature;
        lvgl_port_unlock();
    }
}

static void performAction()
{
    PendingAction a = pending_action;
    if (a == ACTION_NONE) return;
    pending_action = ACTION_NONE;

    String path;
    switch (a) {
        case ACTION_PC: path="/api/source/pc"; break;
        case ACTION_MAC: path="/api/source/mac"; break;
        case ACTION_PS5: path="/api/source/ps5"; break;
        case ACTION_SWITCH: path="/api/source/switch"; break;
        case ACTION_POWER: path="/api/power/toggle"; break;
        default: return;
    }

    String body;
    bool ok = httpPost(path, body);
    if (lvgl_port_lock(60)) {
        if (!ok) {
            // Force the next successful poll to restore the online state even
            // if the backend data itself hasn't changed.
            last_ui_signature = "";
            setOnline(false);
            setMessage("Command failed");
        } else {
            setMessage("");
        }
        lvgl_port_unlock();
    }

    delay(a == ACTION_POWER ? 1100 : 250);
    refreshStatus();
}

static void ensureWiFi()
{
    if (WiFi.status() == WL_CONNECTED) return;

    uint32_t now = millis();
    if (now - last_wifi_attempt < WIFI_RETRY_MS) return;
    last_wifi_attempt = now;

    Serial.printf("[WiFi] retrying connection to %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    if (lvgl_port_lock(50)) {
        setOnline(false);

        wl_status_t st = WiFi.status();
        if (st == WL_NO_SSID_AVAIL) setMessage("Wi-Fi: SSID NOT FOUND");
        else if (st == WL_CONNECT_FAILED) setMessage("Wi-Fi: AUTH FAILED");
        else setMessage("Connecting Wi-Fi...");

        lvgl_port_unlock();
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    Board *board = new Board();
    board->init();

#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 20);
    }
#endif
#endif

    assert(board->begin());

    Serial.println("[BOOT] Starting LVGL port...");
    bool lvgl_ok = lvgl_port_init(board->getLCD(), board->getTouch());
    Serial.printf("[BOOT] LVGL port init: %s\n", lvgl_ok ? "OK" : "FAILED");
    if (!lvgl_ok) {
        while (true) {
            delay(1000);
        }
    }

    lvgl_port_lock(-1);
    buildUI();
    lvgl_port_unlock();

    Serial.println("[BOOT] UI built; continuing to Wi-Fi");

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);

    scanTargetSSID();

    Serial.printf("[WiFi] connecting to %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    last_wifi_attempt = millis();
}

void loop()
{
    ensureWiFi();
    performAction();

    uint32_t now = millis();

    if (now - last_wifi_debug >= 2000) {
        last_wifi_debug = now;
        printWifiDebug();
    }

    if (now - last_status_poll >= STATUS_POLL_MS) {
        last_status_poll = now;
        refreshStatus();
    }

    delay(20);
}