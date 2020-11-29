from threading import Thread

def startAndJoinThread(threadObj:Thread):
    """Helper function that starts and joins a thread"""
    threadObj.start()
    threadObj.join()