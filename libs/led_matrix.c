#include "led_matrix.h"
#include <stdbool.h>

#define MATRIX_PIN 7

// Variável que define a intensidade do brilho dos LEDS (De 0 a 255)
static const uint8_t intensidade_max = 10;

// Declaração das cores dos frames que serão usados na matriz de leds
uint8_t led_r = intensidade_max;
uint8_t led_g = 0;
uint8_t led_b = intensidade_max;

// Vetor com as possíveis entradas
static const char entradas_numericas[10] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};

// Quantidade de pixels
#define NUM_PIXELS 25

////////////////////////////////////// FRAMES UTILIZADOS //////////////////////////////////////
// Note que a função que imprime dados na PIO os colocam uma linha espelhada e a seguinte correta na impressão bit a bit
// Além de que começa de baixo para a cima a impressão a partir do vetor
// Então a configuração dos frames tem que tomar essa forma espelhado-certo, colocando a matriz de ponta-cabeça no arranjo final (Função que imprime os bits)
// Assim o output na matriz estará correto

// Declaração dos frames que serão exibidos na matriz de leds (espelhado-certo)
bool frame_0[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 0, 1, 0, // Linha 2 (Certo)
    0, 1, 0, 1, 0, // Linha 3 (Espelhado)
    0, 1, 1, 1, 0, // Linha 4 (Certo)
};

bool frame_1[NUM_PIXELS] = {
    0, 0, 1, 0, 0, // Linha 0 (Certo)
    0, 0, 1, 0, 0, // Linha 1 (Espelhado)
    0, 0, 1, 0, 0, // Linha 3 (Certo)
    0, 0, 1, 0, 0, // Linha 4 (Espelhado)
    0, 0, 1, 0, 0, // Linha 5 (Certo)
};

bool frame_2[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 0, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 0, 0, 1, 0, // Linha 4 (Espelhado)
    0, 1, 1, 1, 0, // Linha 5 (Certo)
};

bool frame_3[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 0, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 0, 0, // Linha 4 (Espelhado)
    0, 1, 1, 1, 0, // Linha 5 (Certo)
};

bool frame_4[NUM_PIXELS] = {
    0, 1, 0, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 0, 0, // Linha 4 (Espelhado)
    0, 0, 0, 1, 0, // Linha 5 (Certo)
};

bool frame_5[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 0, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 0, 0, // Linha 4 (Espelhado)
    0, 1, 1, 1, 0, // Linha 5 (Certo)
};

bool frame_6[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 0, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 1, 0, // Linha 4 (Espelhado)
    0, 1, 1, 1, 0, // Linha 5 (Certo)
};

bool frame_7[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 0, 0, // Linha 1 (Espelhado)
    0, 0, 0, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 0, 0, // Linha 4 (Espelhado)
    0, 0, 0, 1, 0, // Linha 5 (Certo)
};

bool frame_8[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 1, 0, // Linha 4 (Espelhado)
    0, 1, 1, 1, 0, // Linha 5 (Certo)
};

bool frame_9[NUM_PIXELS] = {
    0, 1, 1, 1, 0, // Linha 0 (Certo)
    0, 1, 0, 1, 0, // Linha 1 (Espelhado)
    0, 1, 1, 1, 0, // Linha 3 (Certo)
    0, 1, 0, 0, 0, // Linha 4 (Espelhado)
    0, 0, 0, 1, 0, // Linha 5 (Certo)
};
////////////////////////////////////// FRAMES UTILIZADOS //////////////////////////////////////

// Vetor que armazena os ponteiros de cada frame criado anteriormente
bool *all_frames[10] = {
    frame_0,
    frame_1,
    frame_2,
    frame_3,
    frame_4,
    frame_5,
    frame_6,
    frame_7,
    frame_8,
    frame_9
};

// Buffer que armazena o frame atual
bool led_buffer[NUM_PIXELS] = {
    1, 1, 1, 1, 1, // Linha 0
    1, 0, 0, 0, 1, // Linha 1
    1, 1, 1, 1, 1, // Linha 3
    0, 0, 0, 0, 1, // Linha 4
    0, 0, 0, 0, 1, // Linha 5
};

static inline void put_pixel(uint32_t pixel_grb){
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função que vai transformar valores correspondentes ao padrão RGB em dados binários
uint32_t urgb_u32(double r, double g, double b){
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função que vai imprimir cada pixel na matriz
void set_one_led(uint8_t r, uint8_t g, uint8_t b){
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    // Faz o processo de virar de cabeça para baixo o arranjo
    for (int i = NUM_PIXELS-1; i >= 0; i--){
        if (led_buffer[i]){
            put_pixel(color); // Liga o LED com um no buffer
        }
        else{
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

// Função que faz as operações necessárias para atualizar o display
void atualiza_numero(char numero_char){
    // Realiza a verificação se foi entrada numérica
    for(int i=0; i<10; i++){
        if(numero_char == entradas_numericas[i]){ // Como é entrada char só vai ser chamado na única detecção que houver
            int numero_int = numero_char - '0'; // Converte char no formato ASCII em INT

            // Pegando o ponteiro para o frame selecionado
            bool* ponteiro_selecionado = all_frames[numero_int];

            // Extraindo cada elemento para a variável criada
            for (int i = 0; i < NUM_PIXELS; i++){
                led_buffer[i] = ponteiro_selecionado[i];
            }

            // Chamando a função que vai imprimir o número
            set_one_led(led_r, led_g, led_b);
        }
    }
}