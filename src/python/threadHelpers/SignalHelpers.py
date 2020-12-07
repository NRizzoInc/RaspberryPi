# standard includes
import signal
from threading import Event

# our includes
from .killableThreads import threadWithException 

def __signal_handler_generator(myThread:threadWithException, stopEvent:Event):
    def sig_handler(sig, frame):
        # end threads via control+c
        # end all threads via raise_exception
        # cleanup with join just in case
        print('You pressed Ctrl+C')
        myThread.raise_exception()
        stopEvent.set()
        myThread.join()
    return sig_handler

def setup_sig_handler(thread:threadWithException, stopEvent:Event=None):
    """2-in-1 function that makes it easy to stop a thread with ctrl+c. Note, call AFTER .start()"""
    print("Press Ctrl+C to Stop")
    signal.signal(signal.SIGINT, __signal_handler_generator(thread, stopEvent))
    # signal.pause() # blocks main if uncommented
