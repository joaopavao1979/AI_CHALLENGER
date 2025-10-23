# 🧪 HuskyCar – Calibração de Proximidade com HuskyLens

Este programa serve para calibrar os limites visuais de deteção da HuskyLens com base na **distância real ao marcador**.

## 🎯 Objetivo
Permitir a recolha de valores reais de **largura (W)** e **altura (H)** da caixa detetada pela HuskyLens, em função da distância ao objeto.

## 🛠️ Como usar
1. Coloca o robô parado no chão.
2. Coloca um marcador (cor previamente treinada com ID1) à frente do robô:
   - 👉 A 15 cm → regista os valores de W e H (mínimo)
   - 👉 A 25 cm → regista os valores de W e H (máximo)
3. Lê os dados no **Serial Monitor**.
4. Usa esses valores no código principal como:
   - `W_MIN`, `W_MAX`
   - `H_MIN`, `H_MAX`

## 📋 Informação apresentada
- `ID`: identificador da cor reconhecida
- `W`: largura da caixa
- `H`: altura da caixa
- `X`: posição horizontal (centrado em 0)
- `Y`: posição vertical (centrado em 0)

## ℹ️ Observações
- Apenas uma cor de cada vez (modo Color Recognition)
- Muito útil para aumentar a precisão da aproximação automática

---

👨‍🏫 Ideal para testes laboratoriais ou em contexto de competição.
