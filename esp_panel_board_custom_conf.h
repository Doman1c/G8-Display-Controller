/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file   esp_panel_board_custom_conf.h
 * @brief  Configuration file for custom ESP development boards
 * @author
 * @link
 *
 * This file contains all the configurations needed for a custom board using ESP Panel.
 * Users can modify these configurations according to their hardware design.
 */

#pragma once

// *INDENT-OFF*
#define ESP_OPEN_TOUCH 1 // 1 initiates the touch, 0 closes the touch.

/**
 * @brief Flag to enable custom board configuration (0/1)
 *
 * Set to `1` to enable custom board configuration, `0` to disable
 */
#define ESP_PANEL_BOARD_DEFAULT_USE_CUSTOM  (1)

#if ESP_PANEL_BOARD_DEFAULT_USE_CUSTOM
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// Please update the following macros to configure general parameters ///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Board name (format: "Manufacturer:Model")
 */
#define ESP_PANEL_BOARD_NAME                "Waveshare:ESP32-S3-Touch-LCD-4.3"

/**
 * @brief Panel resolution configuration in pixels
 */
#define ESP_PANEL_BOARD_WIDTH               (800)   // Panel width (horizontal, in pixels)
#define ESP_PANEL_BOARD_HEIGHT              (480)   // Panel height (vertical, in pixels)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Please update the following macros to configure the LCD panel /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief LCD panel configuration flag (0/1)
 *
 * Set to `1` to enable LCD panel support, `0` to disable
 */
#define ESP_PANEL_BOARD_USE_LCD             (1)

#if ESP_PANEL_BOARD_USE_LCD
/**
 * @brief LCD controller selection
 */
#define ESP_PANEL_BOARD_LCD_CONTROLLER      ST7262

/**
 * @brief LCD bus type selection
 */
#define ESP_PANEL_BOARD_LCD_BUS_TYPE        (ESP_PANEL_BUS_TYPE_RGB)

/**
 * @brief LCD bus parameters configuration
 *
 * Configure parameters based on the selected bus type. Parameters for other bus types will be ignored.
 */
#if ESP_PANEL_BOARD_LCD_BUS_TYPE == ESP_PANEL_BUS_TYPE_RGB
    #define ESP_PANEL_BOARD_LCD_RGB_USE_CONTROL_PANEL       (0)
    #define ESP_PANEL_BOARD_LCD_RGB_CLK_HZ          (16 * 1000 * 1000)
    #define ESP_PANEL_BOARD_LCD_RGB_HPW             (4)
    #define ESP_PANEL_BOARD_LCD_RGB_HBP             (8)
    #define ESP_PANEL_BOARD_LCD_RGB_HFP             (8)
    #define ESP_PANEL_BOARD_LCD_RGB_VPW             (4)
    #define ESP_PANEL_BOARD_LCD_RGB_VBP             (8)
    #define ESP_PANEL_BOARD_LCD_RGB_VFP             (8)
    #define ESP_PANEL_BOARD_LCD_RGB_PCLK_ACTIVE_NEG (1)
    #define ESP_PANEL_BOARD_LCD_RGB_DATA_WIDTH      (16)
    #define ESP_PANEL_BOARD_LCD_RGB_PIXEL_BITS      (ESP_PANEL_LCD_COLOR_BITS_RGB565)
    #define ESP_PANEL_BOARD_LCD_RGB_BOUNCE_BUF_SIZE (ESP_PANEL_BOARD_WIDTH * 10)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_HSYNC        (46)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_VSYNC        (3)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DE           (5)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_PCLK         (7)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DISP         (-1)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA0        (14)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA1        (38)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA2        (18)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA3        (17)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA4        (10)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA5        (39)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA6        (0)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA7        (45)
#if ESP_PANEL_BOARD_LCD_RGB_DATA_WIDTH > 8
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA8        (48)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA9        (47)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA10       (21)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA11       (1)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA12       (2)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA13       (42)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA14       (41)
    #define ESP_PANEL_BOARD_LCD_RGB_IO_DATA15       (40)
#endif
#endif

#define ESP_PANEL_BOARD_LCD_COLOR_BITS          (ESP_PANEL_LCD_COLOR_BITS_RGB565)
#define ESP_PANEL_BOARD_LCD_COLOR_BGR_ORDER     (0)
#define ESP_PANEL_BOARD_LCD_COLOR_INEVRT_BIT    (0)
#define ESP_PANEL_BOARD_LCD_SWAP_XY             (0)
#define ESP_PANEL_BOARD_LCD_MIRROR_X            (0)
#define ESP_PANEL_BOARD_LCD_MIRROR_Y            (0)
#define ESP_PANEL_BOARD_LCD_GAP_X               (0)
#define ESP_PANEL_BOARD_LCD_GAP_Y               (0)
#define ESP_PANEL_BOARD_LCD_RST_IO              (-1)
#define ESP_PANEL_BOARD_LCD_RST_LEVEL           (0)
#endif // ESP_PANEL_BOARD_USE_LCD

#define ESP_PANEL_BOARD_USE_TOUCH               (ESP_OPEN_TOUCH)
#if ESP_PANEL_BOARD_USE_TOUCH
#define ESP_PANEL_BOARD_TOUCH_CONTROLLER        GT911
#define ESP_PANEL_BOARD_TOUCH_BUS_TYPE          (ESP_PANEL_BUS_TYPE_I2C)
#if (ESP_PANEL_BOARD_TOUCH_BUS_TYPE == ESP_PANEL_BUS_TYPE_I2C) || \
    (ESP_PANEL_BOARD_TOUCH_BUS_TYPE == ESP_PANEL_BUS_TYPE_SPI)
#define ESP_PANEL_BOARD_TOUCH_BUS_SKIP_INIT_HOST        (0)
#endif
#if ESP_PANEL_BOARD_TOUCH_BUS_TYPE == ESP_PANEL_BUS_TYPE_I2C
    #define ESP_PANEL_BOARD_TOUCH_I2C_HOST_ID           (0)
#if !ESP_PANEL_BOARD_TOUCH_BUS_SKIP_INIT_HOST
    #define ESP_PANEL_BOARD_TOUCH_I2C_CLK_HZ            (400 * 1000)
    #define ESP_PANEL_BOARD_TOUCH_I2C_SCL_PULLUP        (1)
    #define ESP_PANEL_BOARD_TOUCH_I2C_SDA_PULLUP        (1)
    #define ESP_PANEL_BOARD_TOUCH_I2C_IO_SCL            (9)
    #define ESP_PANEL_BOARD_TOUCH_I2C_IO_SDA            (8)
#endif
    #define ESP_PANEL_BOARD_TOUCH_I2C_ADDRESS           (0)
#endif
#define ESP_PANEL_BOARD_TOUCH_SWAP_XY           (0)
#define ESP_PANEL_BOARD_TOUCH_MIRROR_X          (0)
#define ESP_PANEL_BOARD_TOUCH_MIRROR_Y          (0)
#define ESP_PANEL_BOARD_TOUCH_RST_IO            (-1)
#define ESP_PANEL_BOARD_TOUCH_RST_LEVEL         (0)
#define ESP_PANEL_BOARD_TOUCH_INT_IO            (4)
#define ESP_PANEL_BOARD_TOUCH_INT_LEVEL         (0)
#endif // ESP_PANEL_BOARD_USE_TOUCH

#define ESP_PANEL_BOARD_USE_BACKLIGHT           (1)
#if ESP_PANEL_BOARD_USE_BACKLIGHT
#define ESP_PANEL_BOARD_BACKLIGHT_TYPE          (ESP_PANEL_BACKLIGHT_TYPE_SWITCH_EXPANDER)
#if (ESP_PANEL_BOARD_BACKLIGHT_TYPE == ESP_PANEL_BACKLIGHT_TYPE_SWITCH_GPIO) || \
    (ESP_PANEL_BOARD_BACKLIGHT_TYPE == ESP_PANEL_BACKLIGHT_TYPE_SWITCH_EXPANDER) || \
    (ESP_PANEL_BOARD_BACKLIGHT_TYPE == ESP_PANEL_BACKLIGHT_TYPE_PWM_LEDC)
    #define ESP_PANEL_BOARD_BACKLIGHT_IO        (2)
    #define ESP_PANEL_BOARD_BACKLIGHT_ON_LEVEL  (1)
#endif
#define ESP_PANEL_BOARD_BACKLIGHT_IDLE_OFF      (0)
#endif // ESP_PANEL_BOARD_USE_BACKLIGHT

#define ESP_PANEL_BOARD_USE_EXPANDER            (1)
#if ESP_PANEL_BOARD_USE_EXPANDER
#define ESP_PANEL_BOARD_EXPANDER_CHIP           CH422G
#define ESP_PANEL_BOARD_EXPANDER_SKIP_INIT_HOST     (0)
#define ESP_PANEL_BOARD_EXPANDER_I2C_HOST_ID        (0)
#if !ESP_PANEL_BOARD_EXPANDER_SKIP_INIT_HOST
#define ESP_PANEL_BOARD_EXPANDER_I2C_CLK_HZ         (400 * 1000)
#define ESP_PANEL_BOARD_EXPANDER_I2C_SCL_PULLUP     (1)
#define ESP_PANEL_BOARD_EXPANDER_I2C_SDA_PULLUP     (1)
#define ESP_PANEL_BOARD_EXPANDER_I2C_IO_SCL         (9)
#define ESP_PANEL_BOARD_EXPANDER_I2C_IO_SDA         (8)
#endif
#define ESP_PANEL_BOARD_EXPANDER_I2C_ADDRESS        (0x20)
#endif // ESP_PANEL_BOARD_USE_EXPANDER

#define ESP_PANEL_BOARD_EXPANDER_POST_BEGIN_FUNCTION(p) \
    {  \
        auto board = static_cast<Board *>(p);  \
        auto expander = static_cast<esp_expander::CH422G*>(board->getIO_Expander()->getBase()); \
        expander->enableAllIO_Output(); \
        return true;    \
    }

#define ESP_PANEL_BOARD_LCD_PRE_BEGIN_FUNCTION(p) \
    {  \
        constexpr int LCD_RST = 3; \
        auto board = static_cast<Board *>(p);  \
        auto expander = board->getIO_Expander()->getBase(); \
        expander->digitalWrite(LCD_RST, 0); \
        vTaskDelay(pdMS_TO_TICKS(10)); \
        expander->digitalWrite(LCD_RST, 1); \
        vTaskDelay(pdMS_TO_TICKS(100)); \
        return true;    \
    }

#if ESP_PANEL_BOARD_USE_TOUCH
#define ESP_PANEL_BOARD_TOUCH_PRE_BEGIN_FUNCTION(p) \
    {  \
        constexpr gpio_num_t TP_INT = static_cast<gpio_num_t>(ESP_PANEL_BOARD_TOUCH_INT_IO); \
        constexpr int TP_RST = 1; \
        auto board = static_cast<Board *>(p);  \
        auto expander = board->getIO_Expander()->getBase(); \
        gpio_set_direction(TP_INT, GPIO_MODE_OUTPUT); \
        gpio_set_level(TP_INT, 0); \
        vTaskDelay(pdMS_TO_TICKS(10)); \
        expander->digitalWrite(TP_RST, 0); \
        vTaskDelay(pdMS_TO_TICKS(100)); \
        expander->digitalWrite(TP_RST, 1); \
        vTaskDelay(pdMS_TO_TICKS(200)); \
        gpio_reset_pin(TP_INT); \
        return true;    \
    }
#endif

#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_MAJOR 1
#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_MINOR 0
#define ESP_PANEL_BOARD_CUSTOM_FILE_VERSION_PATCH 0

#endif // ESP_PANEL_BOARD_USE_CUSTOM

// *INDENT-ON*
