import torch
import torch.nn as nn
import torch.optim as optim
import pandas as pd
import model
from typing import List

def model_create():
    return model.Model(5, 64, 1)

def model_train(model_: model.Model, features_tensor: torch.Tensor, label_tensor: torch.Tensor, num_epoches_: int) -> None:
    optimizer = optim.Adam(model_.parameters(), lr=0.001)
    criterion = nn.MSELoss()
    
    for epoch in range(num_epoches_):
        # Forward
        outputs = model_(features_tensor)
        loss = criterion(outputs, label_tensor.unsqueeze(1))
        
        # Backward and optimize
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        
        # Print loss
        if (epoch+1) % 100 == 0:
            print(f'Epoch [{epoch+1}/{num_epoches_}], Loss: {loss.item():.4f}')

def model_inference(model_: model.Model, features_tensor: torch.Tensor) -> torch.Tensor:
    with torch.no_grad():
        label_tensor = model_(features_tensor)
        # label_tensor = label_tensor.round()
        return label_tensor

def model_save(model_: model.Model, save_path: str) -> None:
    state = {}
    state["model"] = model_.state_dict()
    torch.save(state, save_path)

def model_load(load_path: str) -> model.Model:
    model_ = model_create()
    model_.load_state_dict(torch.load(load_path)["model"])
    return model_

'''
def train(model_: model.Model, dataset_file_: str, num_epoches_: int) -> None:
    dataset = pd.read_csv(dataset_file_)
    # Five columns as feature
    features_tensor = torch.tensor(dataset.iloc[:, :5].values, dtype=torch.float32)
    # Last column as label
    label_tensor = torch.tensor(dataset.iloc[:, -1].values, dtype=torch.float32)

    model_train(model_, features_tensor, label_tensor, num_epoches_)
'''

def train(model_: model.Model, dataset_files_: List[str], num_epoches_: int) -> None:
    for dataset_file in dataset_files_:
        dataset_slice = pd.read_csv(dataset_file)
        if dataset_file == dataset_files_[0]:
            dataset = dataset_slice
        else:
            dataset = pd.concat([dataset, dataset_slice], ignore_index=True)
    # Five columns as feature
    features_tensor = torch.tensor(dataset.iloc[:, :5].values, dtype=torch.float32)
    # Last column as label
    label_tensor = torch.tensor(dataset.iloc[:, -1].values, dtype=torch.float32)

    model_train(model_, features_tensor, label_tensor, num_epoches_)

def inference(model_: model.Model, features_vec: List[List[int]]) -> List[int]:
    features_tensor = torch.tensor(features_vec, dtype=torch.float32)
    label_tensor = model_inference(model_, features_tensor)
    label_vec = list(map(int, label_tensor.squeeze().tolist()))
    return label_vec