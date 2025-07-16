import socket

# Estrutura da rede
redes = {}

def processar_mensagem(mensagem, addr):
    partes = mensagem.strip().split()
    if not partes:
        return ""

    comando = partes[0]

    if comando == "NODES":
        net = partes[1]
        lista = redes.get(net, [])
        resposta = f"NODESLIST {net}\n"
        for ip, tcp in lista:
            resposta += f"{ip} {tcp}\n"
        return resposta

    elif comando == "REG":
        net, ip, tcp = partes[1], partes[2], partes[3]
        if net not in redes:
            redes[net] = []
        redes[net].append((ip, tcp))
        return "OKREG\n"

    elif comando == "UNREG":
        net, ip, tcp = partes[1], partes[2], partes[3]
        if net in redes:
            redes[net] = [n for n in redes[net] if n != (ip, tcp)]
        return "OKUNREG\n"

    else:
        return "UNKNOWN COMMAND\n"

def main():
    servidor_ip = "0.0.0.0"
    servidor_porta = 59000

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((servidor_ip, servidor_porta))

    print(f"Servidor de Nós Simulado a correr em {servidor_ip}:{servidor_porta}")

    while True:
        data, addr = sock.recvfrom(1024)
        mensagem = data.decode()
        print(f"Recebido de {addr}: {mensagem.strip()}")

        resposta = processar_mensagem(mensagem, addr)
        sock.sendto(resposta.encode(), addr)

if __name__ == "__main__":
    main()
