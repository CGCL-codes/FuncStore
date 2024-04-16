from model_repo import repo
from model_service import inference_service
from model_service_handler import inference_service_handler
from model_task import model_save
import sys
import os
import json

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.server import TServer

with open("../conf/hive.json") as file:
    conf = json.load(file)
model_host = conf["model"]["host"]
model_port = conf["model"]["port"]
models_path = conf["model"]["models_path"]

def save_models(models_path):
    for model_name, model_ in repo.items():
        model_save(model_, os.path.join(models_path, model_name + ".pth"))

def suppress_keyboard_interrupt_message(models_path):
    old_excepthook = sys.excepthook
 
    def new_hook(exctype, value, traceback):
        if exctype != KeyboardInterrupt:
            old_excepthook(exctype, value, traceback)
        else:
            print('\nKeyboardInterrupt ...')
            if models_path != "":
                print(f'==> Saving models to {models_path} ...')
                save_models(models_path)
                print(f'==> Saving models done!')

    sys.excepthook = new_hook
 
if __name__ == "__main__":
    suppress_keyboard_interrupt_message(models_path)
    handler = inference_service_handler(repo)
    processor = inference_service.Processor(handler)
    transport = TSocket.TServerSocket(model_host, model_port)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()

    server = TServer.TThreadedServer(processor, transport, tfactory, pfactory)
    print("starting server...")
    server.serve()