#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "libs/led_matrix.h"
#include "libs/ssd1306.h"
#include "libs/font.h"

// Definindo os pinos do I2C e endereço do display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADRESS 0x3C 
// Definição dos botões
#define BUTTON_A 5
#define BUTTON_B 6
// Definição dos LEDs RGB
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
#define OUTPUT_MASK ((1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE)) // Definição da máscara para ativar os LEDs RGB
#define INPUT_MASK ((1 << BUTTON_A) | (1 << BUTTON_B)) // Máscara para GPIO dos botões
#define LED_MATRIX_PIN 7
#define IS_RGBW false

ssd1306_t ssd; // Inicializa a estrutura do display no escopo global

// Variável que armazena o tempo do último evento (em us)
static volatile uint32_t last_time = 0;
static volatile bool green_state = false;
static volatile bool blue_state = false;

// Inicializando as strings do display
char string_line1[14] = "              ";
char string_line2[14] = "              ";
// Contadores para percorrer as strings
int count_line1 = 0;
int count_line2 = 0;
bool switch_string = false; // Variável que define qual string vai receber o caracter

// Variáveis da PIO declaradas no escopo global
PIO pio;
uint sm;

// Função que atualiza as strings do display
void update_strings(char entry){
    // Limpa as strings quando a entrada é "~", zerando os contadores
    if(entry == '~'){
        count_line1 = 0;
        count_line2 = 0;
        for(int i = 0; i<=13; i++){
            string_line1[i] = ' ';
            string_line2[i] = ' ';
        }
        switch_string = false;
        return;
    }

    if(count_line1 > 13){
        switch_string = true;
    }
    if(count_line2 > 13){
        return;
    }

    if(switch_string){ 
        string_line2[count_line2] = entry;
        count_line2++;
    }
    else{
        string_line1[count_line1] = entry;
        count_line1++;
    }

}

// Função que imprime o estado dos leds no display
void led_state_on_display(char *led_selected, int x_pos, int y_pos, bool bool_state){
    char *char_state;
    char *erase_string = "    "; // string para limpar o ON/OFF presente no display
    int x_offset;
    int y_offset = 10;

    if(led_selected == "GREEN"){
        x_offset = 8;
    }
    else{
        x_offset = 4;
    }

    if(bool_state){
        char_state = "ON";
        x_offset += 4;
    }
    else{
        char_state = "OFF";
    }

    ssd1306_draw_string(&ssd, erase_string, x_pos, y_pos + y_offset); // Limpando antes de imprimir no display

    ssd1306_draw_string(&ssd, led_selected, x_pos, y_pos); // Desenha o nome do led alterado
    ssd1306_draw_string(&ssd, char_state, x_pos + x_offset, y_pos + y_offset); // Desenha seu estado ON/OFF
}

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
                printf("LED GREEN: OFF\n");
            }
            green_state = gpio_get(LED_GREEN); // Atualiza o booleano da impressão no display
            led_state_on_display("GREEN", 12, 42, green_state); // Escreve ON/OFF no display
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
            blue_state = gpio_get(LED_BLUE); // Atualiza o booleano da impressão no display
            led_state_on_display("BLUE", 80, 42, blue_state);
        }

        ssd1306_send_data(&ssd); // Atualiza o display
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
    // Configuração do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADRESS, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    bool cor = true; // Booleano que indica a cor branca do pixel

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

    // Inicialização da PIO utilizada
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    // Inicializa o estado dos LEDs como off no display
    led_state_on_display("GREEN", 12, 42, green_state);
    led_state_on_display("BLUE", 80, 42, green_state);

    while (true) {
        // Atualização do display
        ssd1306_rect(&ssd, 0, 0, 127, 63, cor, !cor); // Desenha um frame nos contornos do display
        ssd1306_rect(&ssd, 36, 0, 127, 2, cor, cor); // Divisória entre as entradas e leds
        ssd1306_rect(&ssd, 38, 63, 2, 25, cor, cor); // Divisória entre os leds

        ssd1306_draw_string(&ssd, string_line1, 8, 8); // String da linha 1
        ssd1306_draw_string(&ssd, string_line2, 8, 20); // String da linha 2

        ssd1306_send_data(&ssd); // Atualiza o display

        // Recebimento dos caracteres via UART
        char entrada;
        scanf("%c", &entrada);

        // Atualiza MATRIZ DE LEDS, se for entrada numérica
        update_number(entrada);
        // Atualiza a string enviada para o display
        update_strings(entrada);
        
        sleep_ms(10); // Delay para reduzir o consumo de CPU
    }
}
