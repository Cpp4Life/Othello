import socketserver, random, re

from init import Game
from bot import callBot

class GameServerHandler(socketserver.BaseRequestHandler):
    def handle(self):
        gameInstance = Game()

        isPlayFirst = random.choice([True, False])
        if isPlayFirst:
            gameInstance.setNextTurn(callBot(gameInstance.getInfo()))

        while not gameInstance.checkGameOver():
            self.request.sendall(bytes(gameInstance.getInfo(), "ASCII"))

            try:
                ret = str(self.request.recv(8), "ASCII")
            except:
                self.request.sendall(bytes("ERROR: INVALID ASCII STRING ~ " + repr(ret), "ASCII"))
                return
            if ret != "NULL" and re.fullmatch("\w\d", ret) is None:
                self.request.sendall(bytes("ERROR: INVALID INPUT ~ " + repr(ret), "ASCII"))
                return
            if not gameInstance.setNextTurn(ret):
                self.request.sendall(bytes("ERROR: INVALID MOVE ~ " + repr(ret), "ASCII"))
                return

            if not gameInstance.checkGameOver():
                gameInstance.setNextTurn(callBot(gameInstance.getInfo()))

        self.request.sendall(bytes(gameInstance.getFinalResult(), "ASCII"))

if __name__ == "__main__":
    HOST, PORT = "", 14003

    with socketserver.TCPServer((HOST, PORT), GameServerHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()
