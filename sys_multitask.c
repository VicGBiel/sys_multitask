#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/ws2812.pio.h"
#include "pico/bootrom.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>

#define btn_a 5
#define btn_b 6

#define NUM_PIXELS 25 // Número de LEDs na matriz 
#define IS_RGBW false // Define se os LEDs são RGBW ou apenas RGB
#define WS2812_PIN 7 // Pino onde os LEDs WS2812 estão conectados
#define vermelho 0x001000 
#define amarelo 0x051000
#define verde 0x100000 

#define led_pin_red 13
#define led_pin_green 11

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define buzzer_pin 10

static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
volatile bool modo = false;
volatile int estado = 0;

void vLeds1Task(){
    // Ativando as GPIOs do LED RGB
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_init(led_pin_green);
    gpio_set_dir(led_pin_green, GPIO_OUT);


    while (true){
        if(!modo){
            gpio_put(led_pin_green, true); // Liga o led verde
            for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(200));

            gpio_put(led_pin_red, true); // Liga o vermelho junto (total amarelo)
            for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(200));

            gpio_put(led_pin_green, false);
            for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(200));

            gpio_put(led_pin_red, false);
        } else {
            gpio_put(led_pin_green, true); // Liga o led verde
            gpio_put(led_pin_red, true); // Liga o vermelho 
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(75));

            gpio_put(led_pin_green, false);
            gpio_put(led_pin_red, false);
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(125));
        }
    }
}

void vMatriz2Task(){
    // Configuração da matriz de leds
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    const uint32_t cores_rgb[] = {verde, amarelo, vermelho};

    bool led_buffer[] = { 
        0, 0, 1, 0, 0, 
        0, 1, 1, 1, 0, 
        1, 1, 1, 1, 1, 
        0, 1, 1, 1, 0, 
        0, 0, 1, 0, 0
    };

    while (true){
        if(modo == 0){
            for(int i = 0; i < 3; i++){
                for (int j = 0; j < NUM_PIXELS; j++) {
                    if (led_buffer[j]) {
                        pio_sm_put_blocking(pio0, 0, cores_rgb[i] << 8u);
                    } else {
                        pio_sm_put_blocking(pio0, 0, 0 << 8u); // Apaga os outros LEDs
                    }
                }
                for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(200));
            }
        } else {
            for (int i = 0; i < NUM_PIXELS; i++) {
                if (led_buffer[i]) {
                    pio_sm_put_blocking(pio0, 0, amarelo << 8u);
                } else {
                    pio_sm_put_blocking(pio0, 0, 0 << 8u); // Apaga os outros LEDs
                }
            }
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(75));

            for (int i = 0; i < NUM_PIXELS; i++) {
                    pio_sm_put_blocking(pio0, 0, 0 << 8u); // Apaga os outros LEDs
            }
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(125));
        }
    }
}

void vDisplay3Task(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    bool cor = true;

    while (true){
        if(modo == 0){

        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 1, 5, 35, 63, cor, !cor);      // Desenha um retângulo
        ssd1306_circle(&ssd, 22, 11, 8, cor);
        ssd1306_circle(&ssd, 22, 32, 8, cor);
        ssd1306_fill_circle(&ssd, 22, 53, 8, cor);
        ssd1306_draw_string(&ssd, "SIGA", 45, 53); // Desenha uma string
        ssd1306_send_data(&ssd);    // Atualiza o display
        for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(195));

        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 1, 5, 35, 63, cor, !cor);      // Desenha um retângulo
        ssd1306_circle(&ssd, 22, 11, 8, cor);
        ssd1306_fill_circle(&ssd, 22, 32, 8, cor);
        ssd1306_circle(&ssd, 22, 53, 8, cor);
        ssd1306_draw_string(&ssd, "ATENCAO", 45, 32); // Desenha uma string
        ssd1306_send_data(&ssd);    // Atualiza o display
        for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(195));

        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 1, 5, 35, 63, cor, !cor);      // Desenha um retângulo
        ssd1306_fill_circle(&ssd, 22, 11, 8, cor);
        ssd1306_circle(&ssd, 22, 32, 8, cor);
        ssd1306_circle(&ssd, 22, 53, 8, cor);
        ssd1306_draw_string(&ssd, "PARE", 45, 11); // Desenha uma string
        ssd1306_send_data(&ssd);    // Atualiza o display
        for (int i = 0; i < 10 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(195));

        } else {

        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_draw_string(&ssd, "ATENCAO", 45, 32); // Desenha uma string
        ssd1306_rect(&ssd, 1, 5, 35, 63, cor, !cor);      // Desenha um retângulo
        ssd1306_circle(&ssd, 22, 11, 8, cor);
        ssd1306_circle(&ssd, 22, 53, 8, cor);
        ssd1306_fill_circle(&ssd, 22, 32, 8, cor);
        ssd1306_send_data(&ssd);    // Atualiza o display
        for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(73));

        ssd1306_fill_circle(&ssd, 22, 32, 8, !cor);
        ssd1306_circle(&ssd, 22, 32, 8, cor);
        ssd1306_send_data(&ssd);    // Atualiza o display
        for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(122));
        }
    }
}

void vBuzzer4Task()
{
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    uint slice_buz = pwm_gpio_to_slice_num(buzzer_pin);
    pwm_set_clkdiv(slice_buz, 40);
    pwm_set_wrap(slice_buz, 12500);
    pwm_set_enabled(slice_buz, true);  
    
    while (true){
        if(modo == 0){
            for(int i = 0; i < 2 && !modo; i++){
                pwm_set_gpio_level(buzzer_pin, 60);  
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_set_gpio_level(buzzer_pin, 0);  
                vTaskDelay(pdMS_TO_TICKS(900));
            }

            for(int i = 0; i < 10 && !modo; i++){
                pwm_set_gpio_level(buzzer_pin, 60);  
                vTaskDelay(pdMS_TO_TICKS(100));
                pwm_set_gpio_level(buzzer_pin, 0);  
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            pwm_set_gpio_level(buzzer_pin, 60);  
            for (int i = 0; i < 20 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(25));
            pwm_set_gpio_level(buzzer_pin, 0);  
            for (int i = 0; i < 20 && !modo; i++) vTaskDelay(pdMS_TO_TICKS(75));

        } else {
            pwm_set_gpio_level(buzzer_pin, 60);  
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(75));
            pwm_set_gpio_level(buzzer_pin, 0);  
            for (int i = 0; i < 10 && modo; i++) vTaskDelay(pdMS_TO_TICKS(125));
        }      
    }
}

// Trecho para modo BOOTSEL com botão B
void gpio_irq_handler(uint gpio, uint32_t events){
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if(current_time - last_time > 200000){
        last_time = current_time;
        
        if(gpio == btn_a){
            modo = !modo;
        } else if (gpio == btn_b){
            reset_usb_boot(0,0);
        }
    }

}

int main()
{
    // Ativando o botão A
    gpio_init(btn_a);
    gpio_set_dir(btn_a, GPIO_IN);
    gpio_pull_up(btn_a);
    
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(btn_b);
    gpio_set_dir(btn_b, GPIO_IN);
    gpio_pull_up(btn_b);

    gpio_set_irq_enabled_with_callback(btn_a, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(btn_b, GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    xTaskCreate(vLeds1Task, "Blink Task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatriz2Task, "Matriz Task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay3Task, "Display Task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(vBuzzer4Task, "Buzzer Task", 
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
