#------------------------------STANDARD DEPENDENCIES-----------------------------#
import sys
import threading
from queue import Queue
import ctypes
import signal

class threadWithException(threading.Thread):
    def __init__(self,
        name:str,
        target,
        toPrintOnStop:str="",
        stopEvent:threading.Event=None,
        resQueue:Queue=None,
        *args, **kwargs):
        """
            \n@Brief: Helper class that makes it easy to stop a thread
            \n@Param: name - The name of the thread
            \n@Param: target - the function to perform that will be stopped in the middle
            \n@Param: toPrintOnStop - (optional) What's printed when the thread is stopped during target's execution
            \n@Param: stopEvent - Event to broadcast a stop message if this worker thread finishes
            \n@Param: resQueue - Queue to store potential returns
            \n@Note: https://www.geeksforgeeks.org/python-different-ways-to-kill-a-thread/ -- "raising exceptions"
            \n@Use: Create it -> start it -> sleep/do other action -> thisThread.raise_exception() -> thisThread.join()
        """
        threading.Thread.__init__(self) 
        self.name = name
        self.workerFn = target
        self.toPrintOnStop = toPrintOnStop
        self.stopEvent = stopEvent
        self.outputQueue = resQueue
        self.args = args
        self.kwargs = kwargs

    def run(self): 
        # target function of the thread class 
        try:
            res = self.workerFn(*self.args, **self.kwargs)
            if self.outputQueue != None:    self.outputQueue.put(res)
            if self.stopEvent != None:      self.stopEvent.set()
        finally:
            if self.toPrintOnStop != "": print(self.toPrintOnStop)

    def get_id(self): 
        # returns id of the respective thread 
        if hasattr(self, '_thread_id'):
            return self._thread_id
        for id, thread in threading._active.items():
            if thread is self:
                return id

    def raise_exception(self):
        thread_id = self.get_id()
        res = ctypes.pythonapi.PyThreadState_SetAsyncExc(thread_id, 
            ctypes.py_object(SystemExit))
        if res > 1: 
            ctypes.pythonapi.PyThreadState_SetAsyncExc(thread_id, 0)
            print('Exception raise failure')

class stopThreadOnSetCallback(threading.Thread):
    def __init__(self, name:str, onStopCallback, stopEvent:threading.Event=None, *args, **kwargs):
        """
            \n@Brief: Helper class that makes it easy to stop a thread when threading.Event.set() is doen
            \n@Param: name - The name of the thread
            \n@Param: onStopCallback - the function to perform when the thread is told to stop
            \n@Param: stopEvent - Event to broadcast a stop message if this worker thread finishes
            \n@Note: https://www.geeksforgeeks.org/python-different-ways-to-kill-a-thread/ -- "raising exceptions"
            \n@Use: Create it -> start it -> sleep/do other action -> stopEvent.set() -> onStopCallback() -> exit
        """
        threading.Thread.__init__(self) 
        self.name = name
        self.onStopCallback = onStopCallback
        self.stopEvent = stopEvent
        self.args = args
        self.kwargs = kwargs

    def run(self): 
        """Waits until stopEvent.set() is called and then triggers callback"""
        self.stopEvent.wait()
        self.onStopCallback(*self.args, **self.kwargs)
