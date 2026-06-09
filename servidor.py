from flask import Flask, request, jsonify
import requests
from datetime import datetime

app = Flask(__name__)

# Chave de autenticação da API FIRMS da NASA — necessária para consultar focos de calor.

NASA_API_KEY = "SUA_CHAVE_AQUI"

# Coordenadas da área monitorada (Mata Atlântica — SP, PR, MG).
# Formato: longitude_min, latitude_min, longitude_max, latitude_max.

AREA = "-50,-25,-40,-18"

# Limiares de alerta — valores acima desses disparam o sistema.

LIMITE_TEMPERATURA = 60
LIMITE_FUMACA = 400

# Consulta a API FIRMS da NASA e retorna focos de calor na área.

def consultar_nasa():

    # Monta a URL da requisição com a chave, satélite VIIRS e área monitorada.
    # O número "1" no final indica que queremos dados das últimas 24 horas.
    
    url = (f"https://firms.modaps.eosdis.nasa.gov/api/area/csv/"f"{NASA_API_KEY}/VIIRS_SNPP_NRT/{AREA}/1")
    
    try:
        # Faz a requisição à NASA com timeout de 10 segundos.
        
        resposta = requests.get(url, timeout=10, headers={"ngrok-skip-browser-warning": "true"})

        # Separa a resposta em linhas — a primeira é o cabeçalho, as demais são focos.
        
        linhas = resposta.text.strip().split("\n")

        # Conta quantos focos foram detectados (desconta a linha do cabeçalho).
        
        focos = len(linhas) - 1 if len(linhas) > 1 else 0

        # Extrai as coordenadas do primeiro foco detectado, se houver.
        
        primeiro_foco = None
        if focos > 0:
            colunas = linhas[1].split(",")
            if len(colunas) >= 2:
                primeiro_foco = {
                    "latitude": colunas[0],
                    "longitude": colunas[1]
                }

        return focos, primeiro_foco

    except Exception as e:
        
        # Retorna -1 em caso de falha na comunicação com a NASA.
        
        print(f"[ERRO] Falha ao consultar NASA: {e}")
        return -1, None

# Cruza dados do sensor com dados da NASA e retorna o nível de alerta.

def avaliar_situacao(temperatura, fumaca, focos_nasa):

    # Verifica se o sensor local ultrapassou algum dos limiares definidos.
    sensor_disparou = temperatura > LIMITE_TEMPERATURA or fumaca > LIMITE_FUMACA

    # Quatro cenários possíveis com base na combinação sensor + satélite.
    
    if sensor_disparou and focos_nasa > 0:
        return "CRITICO", "Sensor disparou E satélite confirmou foco de calor na região!"
    elif sensor_disparou and focos_nasa == 0:
        return "ATENCAO", "Sensor disparou mas satélite não confirmou foco. Pode ser falso positivo."
    elif not sensor_disparou and focos_nasa > 0:
        return "MONITORAMENTO", "Sensor normal mas satélite detectou foco na região. Fique atento."
    else:
        return "NORMAL", "Tudo normal. Nenhum sinal de queimada detectado."


@app.route("/dados", methods=["POST"])

# Rota que recebe os dados do ESP32 via HTTP POST.

def receber_dados():

    # Lê o JSON enviado pelo ESP32.
    
    dados = request.get_json()

    if not dados:
        return jsonify({"erro": "Nenhum dado recebido"}), 400

    # Extrai os valores do JSON recebido.
    temperatura = dados.get("temperatura", 0)
    fumaca = dados.get("fumaca", 0)
    umidade = dados.get("umidade", 0)
    horario = datetime.now().strftime("%H:%M:%S")

    # Exibe os dados recebidos no terminal.
    
    print(f"DADOS RECEBIDOS DO ESP32 — {horario}")
    print(f"Temperatura: {temperatura}°C")
    print(f"Fumaça: {fumaca} (limite: {LIMITE_FUMACA})")
    print(f"Umidade: {umidade}%")
    print()
    print("Consultando NASA FIRMS...")

    # Consulta a NASA e exibe o resultado.
    
    focos, primeiro_foco = consultar_nasa()

    if focos >= 0:
        print(f"Focos detectados na região: {focos}")
        if primeiro_foco:
            print(f"Primeiro foco: lat {primeiro_foco['latitude']}, lon {primeiro_foco['longitude']}")
    else:
        print("Não foi possível consultar a NASA agora.")

    # Cruza os dados do sensor com os dados da NASA e gera o nível de alerta.
    
    nivel, mensagem = avaliar_situacao(temperatura, fumaca, focos)

    print()
    print(f"{mensagem}")

    # Retorna a resposta ao ESP32 com o resultado da análise.
    
    return jsonify({
        "status": "ok",
        "nivel_alerta": nivel,
        "mensagem": mensagem,
        "focos_nasa": focos,
        "dados_recebidos":
        {
            "temperatura": temperatura,
            "fumaca": fumaca,
            "umidade": umidade
        }
    })

@app.route("/", methods=["GET"])

# Rota de verificação — só pra confirmar que o servidor está no ar.

def home():
    return jsonify({
        "sistema": "OrbitMax Sentinel",
        "status": "online",
        "area_monitorada": "Mata Atlântica (SP, PR, MG)"
    })


if __name__ == "__main__":
    print("OrbitMax Sentinel — Servidor iniciado")
    print("Aguardando dados do ESP32...")
    print("Rodando em http://localhost:5000")
    
    # Inicia o servidor Flask em todas as interfaces de rede na porta 5000.
    
    app.run(host="0.0.0.0", port=5000, debug=False)