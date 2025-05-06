## Semáforo Inteligente utilizando multitarefas no FreeRTOS

Este projeto implementa um sistema de semáforo inteligente usando o microcontrolador RP2040 com FreeRTOS, integrando os seguintes componentes:

### Componentes Utilizados

* LEDs RGB 
* Matriz de LEDs WS2812 (5x5)
* Display OLED SSD1306 via I2C
* Buzzer PWM
* Dois botões (A e B)

### Funcionalidades

* Dois modos de operação, alternados pelo botão A
* Botão B reinicia o dispositivo no modo BOOTSEL (para regravação do firmware)
* Tarefas FreeRTOS controlam:

  * LEDs RGB com efeitos temporizados
  * Matriz de LEDs com ícones coloridos
  * Animações e textos no display OLED
  * Sinais sonoros variados no buzzer

### Estrutura

Cada periférico possui sua própria tarefa:

* `vLeds1Task`
* `vMatriz2Task`
* `vDisplay3Task`
* `vBuzzer4Task`

### Execução

Compile e grave o firmware no RP2040. Pressione o botão A para alternar os modos e veja os efeitos nos LEDs, display e buzzer. Use o botão B para entrar no modo de regravação.

---

### Vídeo de Demonstração

\[Inserir link do vídeo aqui]

---

### Autor

\[Victor Gabriel Guimarães Lopes]
