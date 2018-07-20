import socket
import selectors
import queue
import cv2
import threading


class KinectServer(threading.Thread):

    def __init__(self, name, input_queue):
        super().__init__()
        self.name = name
        self.input_queue = input_queue
        self._sel = selectors.DefaultSelector()

    def _log(self, text):
        print("[ {name:^10} ] {txt}".format(name=self.name, txt=text))

    def _accept(self, key):
        try:
            conn, addr = key.fileobj.accept()
        except socket.error:
            print("Error while accepting connection")
            return

        self._log("Accepted destination {host[0]}:{host[1]}".format(host=addr))
        self._sel.register(conn, selectors.EVENT_WRITE, self._send)

    def _send(self, key):
        conn = key.fileobj
        try:
            data = self.input_queue.get_nowait()
            try:
                conn.sendall(data)
            except (socket.error, EOFError):
                self._sel.unregister(conn)
                conn.close()
                self._log("Client disconnected")
        except queue.Empty:
            pass

    def run(self):
        serv_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        serv_sock.bind(('', 8000))

        serv_sock.listen(5)

        self._sel.register(serv_sock, selectors.EVENT_READ, data=self._accept)

        self._log("Waiting for the destination to connect\n")

        while True:
            try:
                events = self._sel.select()
                for key, mask in events:
                    handler = key.data
                    handler(key)
            except KeyboardInterrupt:
                print("Stopping")
                break

        self._log("Stopped")

        for key in self._sel.get_map().values():
            key.fileobj.close()


if __name__ == '__main__':
    q = queue.Queue()
    k = KinectServer('Kinect', q)
    k.start()

    cap = cv2.VideoCapture(0)
    if (cap.isOpened() == False):
        print("Unable to read camera feed")

    ret = cap.isOpened()
    fourcc = cv2.VideoWriter_fourcc(*'DIVX')
    out = cv2.VideoWriter('output.avi', fourcc, 30.0, (640, 480))

    while ret:
        # Capture frame-by-frame
        ret, frame = cap.read()
        # Our operations on the frame come here
        if ret:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            #print(gray.shape)
            cv2.imshow('frame', gray)
            out.write(frame)

            #ss = bytes(gray.shape[0])+bytes(gray.shape[1])+gray.tobytes()
            q.put(gray.tobytes())
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    # When everything done, release the capture
    cap.release()
