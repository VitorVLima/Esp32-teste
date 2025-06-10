import requests
import random
import time

# Token do projeto Blynk
BLYNK_TOKEN = 'asrOcmhW7k3g1I3gUITYsMwKOmrcg6sa'

# Função para enviar valores para pinos virtuais
def enviar_blynk(pin, valor):
    url = f'https://blynk.cloud/external/api/update?token={BLYNK_TOKEN}&{pin}={valor}'
    response = requests.get(url)
    print(f'{pin} = {valor} -> {response.status_code}')

# Função para enviar um alerta (logEvent)
def enviar_alerta(mensagem):
    url = f'https://blynk.cloud/external/api/logEvent?token={BLYNK_TOKEN}&event=alerta&description={mensagem}'
    response = requests.get(url)
    print(f'alerta: {mensagem} -> {response.status_code}')

ultimo_alerta = 0
intervalo_alerta = 60  # segundos

# Loop de simulação
while True:
    batimentos = random.randint(40, 160)
    oximetria = random.randint(85, 100)
    aceleracao = round(random.uniform(0.1, 10.0), 2)
    rotacao = round(random.uniform(0.1, 1.5), 2)

    # Enviar dados simulados para Blynk
    enviar_blynk('V1', batimentos)
    enviar_blynk('V2', oximetria)
    enviar_blynk('V3', aceleracao)
    enviar_blynk('V4', rotacao)

    # Verificar condições de alerta
    agora = time.time()
    if agora - ultimo_alerta > intervalo_alerta:
        if batimentos < 50:
            enviar_alerta("Batimentos abaixo do normal (< 50 bpm)")
            ultimo_alerta = agora
        elif batimentos > 150:
            enviar_alerta("Batimentos acima do normal (> 150 bpm)")
            ultimo_alerta = agora
        elif oximetria < 90:
            enviar_alerta("Oximetria baixa detectada (< 90%)")
            ultimo_alerta = agora
        elif aceleracao < 0.3:
            enviar_alerta("Possível queda detectada")
            ultimo_alerta = agora

    time.sleep(10)
