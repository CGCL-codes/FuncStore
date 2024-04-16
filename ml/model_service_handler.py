import model
import model_task
from model_service.ttypes import *
from typing import Dict

class inference_service_handler:
    def __init__(self, model_repo: Dict[str, model.Model]) -> None:
        self.model_repo_ = model_repo
    
    def inference(self, app: str, rpc_features: rpc_features_vec):
        # Get correspond model for model repo
        rpc_labels = rpc_label_vec()
        if app not in self.model_repo_:
            self.model_repo_[app] = model_task.model_create()
        model_ = self.model_repo_[app]
        rpc_labels.vec = model_task.inference(model_, rpc_features.vec)
        return rpc_labels