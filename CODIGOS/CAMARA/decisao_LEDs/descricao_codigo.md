# 🎯 Descrição do Código – `decisao_LEDs.ino`

Este código faz parte do prova **AI Challenger** e implementa a lógica de reação automática do Arduino com base nos dados recebidos da câmara **HuskyLens**.  

Através de comunicação UART (via `SoftwareSerial`), o Arduino lê o **ID da cor detetada** pela HuskyLens (modo “Color Recognition”) e acende **LEDs simuladores** de ações robóticas:

- Virar à esquerda
- Virar à direita
- Parar

---

## 🔁 Funcionamento geral

### 🟢 Inicialização

- Configura os pinos dos 4 LEDs como saídas:
  - `D2` → LED VERDE (sistema ativo)
  - `D4` → LED VERMELHO (parar)
  - `D5` → LED AMARELO (virar à esquerda)
  - `D6` → LED AZUL (virar à direita)
- Liga o LED verde como indicador de sistema ativo.
- Estabelece comunicação com a HuskyLens (baud: 9600).
- Imprime no Serial Monitor o estado de ligação.

---

### 🔄 Loop principal

- A cada 500 ms, o Arduino solicita um novo frame à HuskyLens.
- Se houver deteção de uma cor (com ID):
  - Apaga todos os LEDs anteriores.
  - Liga apenas o LED correspondente à ação:
    - ID 1 → Amarelo → virar à esquerda
    - ID 2 → Azul → virar à direita
    - ID 3 → Vermelho → parar
- Se nada for detetado:
  - Apaga todos os LEDs, exceto o LED verde (sistema pronto).

---

## 📋 Tabela de mapeamento (exemplo)

| Cor reconhecida | ID atribuído | Ação simulada      | LED |
|------------------|---------------|---------------------|-----|
| Azul             | 1             | Virar à esquerda    | Amarelo (D5) |
| Verde            | 2             | Virar à direita     | Azul (D6)    |
| Vermelho         | 3             | Parar               | Vermelho (D4) |

> ⚠️ Os IDs dependem do treino da HuskyLens. Confirma no ecrã da câmara qual é o ID de cada cor treinada.

---

## 🔌 Ligações físicas

- HuskyLens:
  - **TX** → D8 (RX do Arduino)
  - **RX** ← D9 (TX do Arduino)
  - **GND** → GND do Arduino
  - **VCC** → *não ligado* (alimentada por powerbank USB)

- LEDs:
  - D2 → LED verde (sistema ativo)
  - D4 → LED vermelho (parar)
  - D5 → LED amarelo (esquerda)
  - D6 → LED azul (direita)
  - Cada LED com resistência (~220 Ω) no anodo ou cátodo

---

## 📘 Resumo técnico

- Comunicação UART com a HuskyLens via `SoftwareSerial`
- Leitura de deteções com `huskylens.request()` e `huskylens.read()`
- Resposta imediata com `digitalWrite()` consoante o ID
- Delay de 500 ms entre leituras
- Utilização de biblioteca oficial `HUSKYLENS.h` da DFRobot

---

## 🎓 Aplicação pedagógica

Este código permite aos alunos:

- Praticar integração de sensores inteligentes com Arduino
- Compreender o uso de comunicação serial entre módulos
- Visualizar decisões reativas com base em visão computacional
- Simular comportamentos de um robô com LEDs antes de usar motores reais

---

> 🧪 Este sketch é ideal para simular comportamentos básicos do carro do Azorebot antes da montagem completa com motores e chassis.
