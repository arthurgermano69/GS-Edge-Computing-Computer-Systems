from flask import Flask, request, jsonify
import requests
from datetime import datetime

app = Flask(__name__)

NASA_API_KEY = "SUA_CHAVE_AQUI"

AREA = "-50,-25,-40,-18"

LIMITE_TEMPERATURA = 60 
LIMITE_FUMACA = 400 

# Consulta a API FIRMS da NASA e retorna focos de calor na área.

def consultar_nasa():
    url = (
        f"https://firms.modaps.eosdis.nasa.gov/api/area/csv/"f"{NASA_API_KEY}/VIIRS_SNPP_NRT/{AREA}/1"
    )
    try:
        resposta = requests.get(url, timeout=10, headers={"ngrok-skip-browser-warning": "true"})
        linhas = resposta.text.strip().split("\n")

        focos = len(linhas) - 1 if len(linhas) > 1 else 0

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
        print(f"  [ERRO] Falha ao consultar NASA: {e}")
        return -1, None

# Cruza dados do sensor com dados da NASA e retorna o nível de alerta.

def avaliar_situacao(temperatura, fumaca, focos_nasa):
    sensor_disparou = temperatura > LIMITE_TEMPERATURA or fumaca > LIMITE_FUMACA

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
    dados = request.get_json()

    if not dados:
        return jsonify({"erro": "Nenhum dado recebido"}), 400

    temperatura = dados.get("temperatura", 0)
    fumaca = dados.get("fumaca", 0)
    umidade = dados.get("umidade", 0)
    horario = datetime.now().strftime("%H:%M:%S")

    print("\n" + "=" * 50)
    print(f"DADOS RECEBIDOS DO ESP32 — {horario}")
    print("=" * 50)
    print(f"Temperatura: {temperatura}°C")
    print(f"Fumaça: {fumaca} (limite: {LIMITE_FUMACA})")
    print(f"Umidade: {umidade}%")
    print()
    print("Consultando NASA FIRMS...")

    focos, primeiro_foco = consultar_nasa()

    if focos >= 0:
        print(f"Focos detectados na região: {focos}")
        if primeiro_foco:
            print(f"Primeiro foco: lat {primeiro_foco['latitude']}, lon {primeiro_foco['longitude']}")
    else:
        print("Não foi possível consultar a NASA agora.")

    nivel, mensagem = avaliar_situacao(temperatura, fumaca, focos)

    print()
    print(f"  {mensagem}")
    print("=" * 50)

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
    print("=" * 50)
    print("OrbitMax Sentinel — Servidor iniciado")
    print("Aguardando dados do ESP32...")
    print("Rodando em http://localhost:5000")
    print("=" * 50)
    app.run(host="0.0.0.0", port=5000, debug=False)