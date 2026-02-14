import asyncio
import websockets
import json
import random
 
async def handler(websocket):
    print("Cliente conectado!")
    try:
        while True:
            # 1. Simulação de Leitura de Sensores
            data = {
                "rpm1": random.randint(1150, 1250),
                "rpm2": random.randint(800, 900),
                "rpm3": random.randint(500, 600),
                "rpm4": random.randint(300, 400),
                "vib": round(random.uniform(0.1, 1.5), 2)
            }
            
            # Envia para o Front-end
            await websocket.send(json.dumps(data))
            
            # 2. Verifica se chegou algum comando do Operador (não bloqueante)
            try:
                message = await asyncio.wait_for(websocket.recv(), timeout=0.1)
                command = json.loads(message)
                if "action" in command:
                    print(f"COMANDO RECEBIDO: {command['action']}")
                if "freq" in command:
                    print(f"FREQUÊNCIA ALTERADA: {command['freq']} Hz")
            except asyncio.TimeoutError:
                pass
            
            await asyncio.sleep(0.2) # Intervalo de atualização
            
    except websockets.exceptions.ConnectionClosed:
        print("Cliente desconectado.")
 
async def main():
    # Roda na porta 8080 para não conflitar com o Live Server (geralmente 5500)
    async with websockets.serve(handler, "localhost", 8080):
        print("Emulador de ESP32 rodando em ws://localhost:8080")
        await asyncio.Future()  # roda para sempre
 
if __name__ == "__main__":
    asyncio.run(main())