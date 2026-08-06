# Projeto BLE - Comunicação entre ESP32 Server, Clientes e App

## Descrição

Este projeto implementa uma rede utilizando **Bluetooth Low Energy (BLE)** com três dispositivos ESP32 e um aplicativo desenvolvido no **MIT App Inventor**.

A arquitetura consiste em:

- **1 ESP32 Server BLE**: responsável por receber comandos do aplicativo e distribuir informações para os clientes.
- **2 ESP32 Clientes BLE**: recebem as informações enviadas pelo servidor e atualizam o estado do LED.
- **Aplicativo Mobile (MIT App Inventor)**: realiza a conexão BLE com o servidor e permite controlar a intensidade do LED através de uma barra de controle (*Slider*).

O objetivo do projeto é demonstrar uma comunicação BLE bidirecional, onde um dispositivo centraliza o controle e transmite informações para outros nós da rede.

---

## Funcionamento

### 1. Aplicativo Mobile

O aplicativo desenvolvido no **MIT App Inventor** realiza a conexão com o ESP32 Server utilizando BLE.

Funcionalidades:

- Descoberta do dispositivo BLE.
- Conexão com o ESP32 Server.
- Controle da intensidade do LED através de uma barra deslizante (*Slider*).
- Envio do valor de intensidade para o servidor.

---

### 2. ESP32 Server BLE

O ESP32 Server possui uma característica BLE responsável por receber os valores enviados pelo aplicativo.

Após receber a nova intensidade do LED:

1. Atualiza o próprio LED utilizando PWM.
2. Envia o novo valor de intensidade para os ESP32 Clientes através de notificações BLE.

Principais funções:

- Gerenciamento da conexão BLE.
- Recepção de comandos do aplicativo.
- Comunicação com múltiplos clientes.
- Controle do LED local.

---

### 3. ESP32 Clientes BLE

Os clientes realizam a conexão com o ESP32 Server.

Após receberem uma atualização:

- Interpretam o valor recebido.
- Aplicam a intensidade no LED utilizando PWM.

---

## Características BLE Utilizadas

### GATT Server

O ESP32 Server funciona como um **GATT Server**, disponibilizando:

- Serviço BLE.
- Característica para escrita dos valores recebidos do aplicativo.
- Característica para notificações aos clientes.

### GATT Client

Os ESP32 Clientes funcionam como **GATT Clients**, realizando:

- Descoberta do serviço BLE.
- Inscrição nas notificações.
- Recepção dos valores enviados pelo servidor.

---

## Link de Demonstração
https://drive.google.com/file/d/1il9exlfnf6xMrlCVtNwRmsO19vnYo_nK/view?usp=drive_link 
---
