import model
import model_task
import torch
import json
import os
import sys

repo = {}

def traverse_directories(path):
    mapping_dict = {}

    for root, directories, files in os.walk(path):
        for directory in directories:
            file_paths = []
            for file in os.listdir(os.path.join(root, directory)):
                if file.endswith("training.csv"):
                    file_paths.append(os.path.join(root, directory, file))
            mapping_dict[directory] = file_paths
    
    return mapping_dict

with open("../conf/hive.json") as file:
    conf = json.load(file)
models_path = conf["model"]["models_path"]
datasets_path = conf["model"]["datasets_path"]

if models_path != "":
    for file_name in os.listdir(models_path):
        file_path = os.path.join(models_path, file_name)
        print(f"Loading model file: {file_path}")
        if os.path.isfile(file_path):
            try:
                model_name = os.path.splitext(file_name)[0]
                model_ = model_task.model_load(file_path)
                repo[model_name] = model_
            except Exception as e:
                print(f"Error loading model file {file_path}: {str(e)}")

if datasets_path != "":
    datasets_mapping = traverse_directories(datasets_path)
    for app_name, dataset_files in datasets_mapping.items():
        print(f"Trainging model [{app_name}] with dataset {dataset_files}")
        if app_name in repo:
            model_ = repo[app_name]
        else:
            model_ = model_task.model_create()
        model_task.train(model_, dataset_files, 1000)
        repo[app_name] = model_

        # Model size
        model_size = sys.getsizeof(model_)
        print(f"Model size: {model_size} bytes")

        # Model parameters size
        parameters_size = sum(p.numel() * p.element_size() for p in model_.parameters())
        print(f"Model parameters size: {parameters_size} bytes")