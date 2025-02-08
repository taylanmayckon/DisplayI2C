#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "libs/led_matrix.h"

// Definindo os pinos do I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
// Definindo o endereço do display 
#define DISPLAY_ADRESS 0x3C
// Definição dos botões
#define BUTTON_A 5
#define BUTTON_B 6
// Definição dos LEDs RGB
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
// Definição da máscara para ativar os LEDs RGB
#define OUTPUT_MASK ((1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE))
// Máscara para GPIO dos botões
#define INPUT_MASK ((1 << BUTTON_A) | (1 << BUTTON_B))

// Variável que armazena o tempo do último evento (em us)
static volatile uint32_t last_time = 0;

// Função de interrupção da GPIO
void gpio_irq_handler(uint gpio, uint32_t events){
    // Obtendo tempo atual (em us)
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Aplicação do debounce
    if(current_time - last_time > 200000){
        last_time = current_time;

        if(gpio == BUTTON_A){
            gpio_put(LED_GREEN, !gpio_get(LED_GREEN));
            // Registro da ação
            if(gpio_get(LED_GREEN)){
                printf("LED GREEN: ON\n");
            }
            else{
                printf("LED_GREEN: OFF\n");
            }
        }

        if(gpio == BUTTON_B){
            gpio_put(LED_BLUE, !gpio_get(LED_BLUE));
            // Registro da ação
            if(gpio_get(LED_BLUE)){
                printf("LED BLUE: ON\n");
            }
            else{
                printf("LED BLUE: OFF\n");
            }
        }
    }
}


int main(){
    stdio_init_all();

    // Inicializando o I2C com 400KHz
    i2c_init(I2C_PORT, 400*1000);
    // Setando as funções dos pinos do I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // Garantindo o Pull up do I2C
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializando todos os pinos da máscara de OUTPUT
    gpio_init_mask(OUTPUT_MASK);
    // Definindo como saída
    gpio_set_dir_out_masked(OUTPUT_MASK);

    // Inicializando todos os pinos da máscara de INPUT
    gpio_init_mask(INPUT_MASK);
    // Definindo os botões como pull up
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
    // Configurando as interupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        sleep_ms(10); // Delay para reduzir o consumo de CPU
    }
}
