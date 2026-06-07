# OrbitMax Sentinel

> Monitoramento de queimadas e desmatamento via dados satelitais e sensores de campo.

## Descrição do Projeto

O **OrbitMax Sentinel** é uma plataforma de monitoramento ambiental que combina sensores físicos de campo com dados orbitais reais da NASA para detectar e confirmar focos de queimadas em tempo real.

O sistema utiliza um ESP32 como estação de campo, capturando temperatura, umidade e índice de fumaça do ambiente. Esses dados são enviados via WiFi para um servidor Python que os cruza com informações dos satélites VIIRS e MODIS da NASA (via API FIRMS), gerando alertas consolidados para órgãos ambientais e cidadãos.

O projeto conecta diretamente ao **ODS 13 — Ação Climática**, propondo uma solução tecnológica real para um problema ambiental crítico no Brasil.

## Objetivo da Solução

- Detectar focos de calor e fumaça em campo usando sensores IoT
- Confirmar alertas cruzando dados do sensor com dados satelitais reais da NASA
- Reduzir falsos positivos ao exigir confirmação dupla: sensor + satélite
- Gerar alertas em tempo real para autoridades ambientais e cidadãos
- Contribuir para o combate ao desmatamento na Mata Atlântica

## Componentes Utilizados

| Componente | Função | Pino ESP32 |

| ESP32 | Microcontrolador principal com WiFi | — |
| DHT22 | Sensor de temperatura e umidade | GPIO 15 |
| Potenciômetro | Simula sensor de fumaça | GPIO 34 |
| LED Vermelho | Alerta visual de perigo | GPIO 2 |
| LED Verde | Status normal do sistema | GPIO 4 |
| Buzzer | Alarme sonoro de emergência | GPIO 5 |
| Resistor 220Ω | Proteção dos LEDs | — |
| Protoboard | Montagem dos componentes | — |

## Explicação do Funcionamento

O sistema opera em três camadas integradas:

**1. Camada de Campo — ESP32**

O ESP32 lê os sensores a cada 5 segundos. Se a temperatura ultrapassar 60°C ou o índice de fumaça ultrapassar 400, o LED vermelho acende e o buzzer dispara como alerta local imediato. Os dados são empacotados em JSON e enviados via WiFi para o servidor Python.

**2. Camada de Processamento — Python (Edge Node)**

O servidor Python recebe os dados do ESP32, consulta a API FIRMS da NASA perguntando se existem focos de calor detectados por satélite na região da Mata Atlântica, e cruza as duas informações para gerar uma conclusão:

| Sensor | Satélite NASA | Resultado |

| Disparou | Confirmou foco | ALERTA CRÍTICO — queimada confirmada |
| Disparou | Sem foco | ATENÇÃO — possível falso positivo |
| Normal | Detectou foco | MONITORAMENTO — foco distante do sensor |
| Normal | Sem foco | NORMAL — nenhuma ameaça detectada |

**3. Camada Orbital — NASA FIRMS**

A API FIRMS (Fire Information for Resource Management System) disponibiliza dados reais dos satélites VIIRS e MODIS que sobrevoam o Brasil várias vezes ao dia, detectando focos de calor com alta precisão geográfica.

## Estrutura do Circuito

```

ESP32
├── GPIO 15  →  DHT22 (DATA)
├── GPIO 34  →  Potenciômetro (SIG) — simula fumaça
├── GPIO 2   →  Resistor 220Ω → LED Vermelho → GND
├── GPIO 4   →  Resistor 220Ω → LED Verde → GND
├── GPIO 5   →  Buzzer → GND
├── 3.3V     →  DHT22 (VCC) + Potenciômetro (VCC)
└── GND      →  DHT22 (GND) + Potenciômetro (GND) + LEDs + Buzzer

```

**Simulação disponível no Wokwi:** (https://wokwi.com/projects/466048441914844161)

## Instruções de Execução

### Pré-requisitos

- Python 3.x instalado
- Conta gratuita na [NASA FIRMS](https://firms.modaps.eosdis.nasa.gov) com API key
- Bibliotecas Python: `flask` e `requests`

### Instalação das dependências

```bash
pip install flask requests
```

### Configuração do servidor Python

1. Abra o arquivo `servidor.py`
2. Substitua `SUA_CHAVE_AQUI` pela sua API key da NASA FIRMS:
```python
NASA_API_KEY = "sua_chave_aqui"
```
3. Execute o servidor:
```bash
python servidor.py
```

### Execução com hardware físico

1. Monte o circuito conforme a estrutura descrita acima
2. Substitua o endereço do servidor pelo IP local da sua máquina no código do ESP32:
```cpp
const char* SERVIDOR = "http://SEU_IP_LOCAL:5000/dados";
```
3. Faça o upload do código pelo Arduino IDE
4. Execute o `servidor.py` no computador
5. O terminal exibirá os dados recebidos do ESP32 e o resultado da consulta à NASA em tempo real

## Estrutura do Repositório

```
orbitmax-sentinel/
├── esp32_orbitmax.ino   # Código C++ do ESP32
├── servidor.py          # Servidor Python com integração NASA
└── README.md            # Documentação do projeto
```

## Conexão com ODS

Este projeto conecta diretamente ao **ODS 13 — Ação Climática**, contribuindo para o monitoramento e prevenção de queimadas e desmatamento na Mata Atlântica, um dos biomas mais ameaçados do Brasil.

## Integrantes do Grupo

| Nome Completo | RM |

| Arthur Germano Pinheiro | rm574042 |
| Artur Novazzi Maia | rm572624 |
| Bruno Araújo Castro | rm572723 |
| João Pedro De Souza | rm571437 |

## 🏫 Informações Acadêmicas

**Disciplina:** Edge Computing & Computer Systems  
**Professores:** Prof. Paulo Marcotti e Prof. Lucas Demetrius  
**Curso:** Engenharia de Software — 1º Ano  
**Instituição:** FIAP
