namespace cpp hive.predictor

struct rpc_features_vec {
  1: list<list<i32>> vec
}

struct rpc_label_vec {
  1: list<i32> vec
}

service inference_service {
  // Inference a features vector
  rpc_label_vec inference(1: string app, 2: rpc_features_vec features),
}