# standard includes
import signal

# our includes
from .killableThreads import threadWithException 

def signal_handler_generator(myThread:threadWithException):
    def sig_handler(sig, frame):
        print('You pressed Ctrl+C')
        # end thread
        myThread.raise_exception()
    return sig_handler

def setup_sig_handler(thread:threadWithException):
    print("Press Ctrl+C to Stop")
    signal.signal(signal.SIGINT, signal_handler_generator(thread))
    signal.pause()
